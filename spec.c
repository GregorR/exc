#define _XOPEN_SOURCE 700 /* for strdup */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

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
        INIT_BUFFER(cmd->repNames);

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
                            char *rName, *rName2;
                            i++;
                            SF(rName, strdup, NULL, (node->children[i+1]->tok->tok));
                            WRITE_ONE_BUFFER(cmd->repPositions, (int) cmd->cmd.bufused);
                            WRITE_ONE_BUFFER(cmd->repNames, rName);
                            SF(rName2, strdup, NULL, (rName));
                            WRITE_ONE_BUFFER(cmd->cmd, rName2);
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

        WRITE_ONE_BUFFER(cmd->cmd, NULL);

        /* and stop searching */
        *then = THEN_STOP;
        return node;
    }

    return node;
}

/* read a spec command */
SpecCmd *readSpecCmd(Node *node, const char *cmd)
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
            file = EXC_DEFAULT_SPEC_FILE;

        /* load from the spec dir */
        SF(path, malloc, NULL, (strlen(bindir) + strlen(EXC_DEFAULT_SPEC_DIR) + strlen(file) + 1));
        sprintf(path, "%s%s%s", bindir, EXC_DEFAULT_SPEC_DIR, file);
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

    return ret;
}

/* run a spec command with the given replacements */
struct Buffer_char execSpec(
    SpecCmd *cmd,
    char *const replNames[],
    char *const replVals[],
    struct Buffer_char input,
    int *status)
{
    /* FIXME: replacements not ACTUALLY implemented yet :) */
    return execBuffered(cmd->cmd.buf, input, status);
}
