#define _XOPEN_SOURCE 700
#include "stdio.h"

#include "stdlib.h"

#include "string.h"

#include "sys/types.h"

#include "exec.h"

#include "node.h"

#include "parse.h"

#include "scan.h"

#include "spec.h"

#include "transform.h"

#include "unparse.h"

static Node *readSpecCmdPrime(TransformState *state, Node *node, int *then, void *arg)
{
    SpecCmd **ret = (SpecCmd **) arg;
    SpecCmd *cmd;
    size_t i;
    /* make sure we have an open part */
    if (node->children[1] && node->children[1]->children[0]) {
        node = node->children[1]->children[0];
        /* allocate space */
        SF(cmd, malloc, NULL, (sizeof(SpecCmd)));
        *ret = cmd;
        INIT_BUFFER(cmd->cmd);
        INIT_BUFFER(cmd->repPositions);
        cmd->i = cmd->o = -1;
        /* read it in */
        for (i = 0; node->children[i]; i++) {
            Node *cnode = node->children[i];
            if (cnode->tok) {
                switch (cnode->tok->type) {
                    case TOK_DECORATION:
                        /* a replacement */
                        if (node->children[i+1] &&
                            node->children[i+1]->tok &&
                            node->children[i+1]->tok->tok) {
                            char *rName;
                            i++;
                            SF(rName, strdup, NULL, (node->children[i]->tok->tok));
                            WRITE_ONE_BUFFER(cmd->cmd, rName);
                            /* special case for @i and @o */
                            if (!strcmp(rName, "i")) {
                                cmd->i = (int) cmd->cmd.bufused;
                            } else if (!strcmp(rName, "o")) {
                                cmd->o = (int) cmd->cmd.bufused;
                            } else {
                                WRITE_ONE_BUFFER(cmd->repPositions, (int) cmd->cmd.bufused - 1);
                            }
                        }
                        break;
                    case TOK_STR_LITERAL:
                    {
                        /* get the actual string */
                        struct Buffer_char val = cunparseStrLiteral(cnode->tok);
                        WRITE_ONE_BUFFER(cmd->cmd, val.buf);
                        break;
                    }
                    default:
                    {
                        /* just accept the token itself */
                        char *tok;
                        SF(tok, strdup, NULL, (cnode->tok->tok));
                        WRITE_ONE_BUFFER(cmd->cmd, tok);
                    }
                }
            }
        }
        /* and stop searching */
        *then = THEN_STOP;
        return node;
    }
    return node;
}
/* read a spec command */
static SpecCmd *readSpecCmd(Node *node, const char *cmd)
{
    TrFind find;
    SpecCmd *ret = NULL;
    memset(&find, 0, sizeof(TrFind));
    find.matchDecoration[0] = cmd;
    transform(NULL, node, &find, readSpecCmdPrime, (void *) &ret);
    return ret;
}
/* load the default or specified spec file (return allocated) */
 Spec *excLoadSpec(const char *bindir, const char *file)
{
    FILE *f = NULL;
    char *path, *parseError;
    struct Buffer_char buf;
    ScanState sstate;
    Node *node;
    Spec *ret;
    if (file) {
        /* try to load from a direct source */
        f = fopen(file, "r");
    }
    if (!f) {
        if (!file)
            file = "gcc.excspec";
        /* load from the spec dir */
        SF(path, malloc, NULL, (strlen(bindir) + strlen("/../share/exc/spec/") + strlen(file) + 1));
        sprintf(path, "%s%s%s", bindir, "/../share/exc/spec/", file);
        f = fopen(path, "r");
        free(path);
    }
    if (!f) {
        /* failing that, load it direct from bindir (uninstalled) */
        SF(path, malloc, NULL, (strlen(bindir) + strlen(file) + 2));
        sprintf(path, "%s/%s", bindir, file);
        f = fopen(path, "r");
        free(path);
    }
    if (!f) {
        /* failure! */
        perror(file);
        exit(1);
    }
    /* read it in */
    INIT_BUFFER(buf);
    READ_FILE_BUFFER(buf, f);
    WRITE_ONE_BUFFER(buf, '\0');
    fclose(f);
    /* parse it */
    sstate = newScanState(0);
    sstate.buf = buf;
    parseError = NULL;
    node = cparse(&sstate, &parseError);
    if (!node) {
        if (parseError)
            fprintf(stderr, "Error parsing spec file: %s\n", parseError);
        exit(1);
    }
    /* and read in each line */
    SF(ret, malloc, NULL, (sizeof(Spec)));
    ret->cpp = readSpecCmd(node, "cpp");
    ret->cc = readSpecCmd(node, "cc");
    ret->ld = readSpecCmd(node, "ld");
    return ret;
}
/* run a spec command with the given replacements */
 struct Buffer_char execSpec(
    SpecCmd *cmd,
    char *const addlFlags[],
    char *const repNames[],
    char *const repVals[],
    struct Buffer_char input,
    int *status)
{
    size_t i;
    struct Buffer_char ret;
    struct Buffer_charp repCmd;
    char fin[] = "/tmp/i.XXXXXX";
    char fon[] = "/tmp/o.XXXXXX";
    FILE *fi = NULL, *fo = NULL;
    int tmpi;
    /* copy the command */
    INIT_BUFFER(repCmd);
    for (i = 0; i < cmd->cmd.bufused; i++)
        WRITE_ONE_BUFFER(repCmd, cmd->cmd.buf[i]);
    /* and the additional flags */
    if (addlFlags) {
        for (i = 0; addlFlags[i]; i++)
            WRITE_ONE_BUFFER(repCmd, addlFlags[i]);
    }
    WRITE_ONE_BUFFER(repCmd, NULL);
    /* handle input and output */
    if (cmd->i >= 0) {
        SF(tmpi, mkstemp, -1, (fin));
        SF(fi, fdopen, NULL, (tmpi, "w"));
        if (fwrite(input.buf, 1, input.bufused, fi) < input.bufused) {
            fprintf(stderr, "Error writing to temporary file %s!\n", fin);
            exit(1);
        }
        repCmd.buf[cmd->i] = fin;
    }
    if (cmd->o >= 0) {
        SF(tmpi, mkstemp, -1, (fon));
        SF(fo, fdopen, NULL, (tmpi, "r"));
        repCmd.buf[cmd->o] = fon;
    }
    /* perform replacements */
    if (repNames && repVals) {
        for (i = 0; repNames[i]; i++) {
            size_t w;
            for (w = 0; w < cmd->repPositions.bufused; w++) {
                if (!strcmp(repNames[i],
                            cmd->cmd.buf[cmd->repPositions.buf[w]])) {
                    repCmd.buf[cmd->repPositions.buf[w]] = repVals[i];
                    break;
                }
            }
        }
    }
    /* execute it */
    ret = execBuffered(repCmd.buf, input, status);
    /* get out output */
    if (cmd->o >= 0) {
        ret.bufused = 0;
        READ_FILE_BUFFER(ret, fo);
    }
    /* get rid of our temp files */
    if (fi) fclose(fi);
    if (fo) fclose(fo);
    FREE_BUFFER(repCmd);
    /* and return the output */
    return ret;
}
