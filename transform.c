#define _XOPEN_SOURCE 700 /* for fdopen */

#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtin-stages.h"
#include "parse.h"
#include "transform.h"

enum {
    MATCH_NO = 0,
    MATCH_MATCH,
    MATCH_NOTIN
};

static int match(TransformState *state, Node *node, TrFind *find)
{
    int i;

    /* first try node type match */
    for (i = 0; i < TR_FIND_MATCH_CT; i++) {
        if (find->matchNode[i] == node->type) return MATCH_MATCH;
        if (find->notInNode[i] == node->type) return MATCH_NOTIN;
    }

    /* now if it's a decorator, try matching it */
    if (node->type == NODE_EXPRESSION_DECORATOR ||
        node->type == NODE_DECLARATION_DECORATOR) {
        /* the first child is a decoration name, check it */
        const char *dName = node->children[0]->tok->tok;

        for (i = 0; i < TR_FIND_MATCH_CT &&
                    (find->matchDecoration[i] || find->notInDecoration[i]); i++) {
            if (find->matchDecoration[i] &&
                !strcmp(find->matchDecoration[i], dName)) return MATCH_MATCH;
            if (find->notInDecoration[i] &&
                !strcmp(find->notInDecoration[i], dName)) return MATCH_NOTIN;
        }
    }

    /* and maybe it'll provide functions */
    if (find->matchFunc && find->matchFunc(state, node)) return MATCH_MATCH;
    if (find->notInFunc && find->notInFunc(state, node)) return MATCH_NOTIN;

    return MATCH_NO;
}

/* resize a node */
Node *trResize(Node *node, size_t to)
{
    size_t i;
    Node *nnode = newNode(node->parent, node->type, node->tok, to);

    for (i = 0; node->children[i] && i < to; i++) {
        nnode->children[i] = node->children[i];
        nnode->children[i]->parent = node;
        node->children[i] = NULL;
    }

    /* make sure we don't lose any children */
    if (node->children[i]) {
        for (; node->children[i]; i++) {
            freeNode(node->children[i]);
            node->children[i] = NULL;
        }
    }

    /* update it in the parent */
    if (node->parent) {
        Node *pnode = node->parent;
        for (i = 0; pnode->children[i] && pnode->children[i] != node; i++);
        if (pnode->children[i]) pnode->children[i] = nnode;
    }

    /* free the old node */
    freeNode(node);

    return nnode;
}

/* append nodes as children of an existing node */
Node *trAppend(Node *parent, ...)
{
    struct Buffer_Nodep buf;
    Node *child;
    va_list ap;
    size_t i, ni;

    INIT_BUFFER(buf);

    /* first collect all the nodes */
    va_start(ap, parent);
    while ((child = va_arg(ap, Node *)))
        WRITE_ONE_BUFFER(buf, child);
    va_end(ap);

    /* now resize the parent node */
    for (i = 0; parent->children[i]; i++);
    parent = trResize(parent, i + buf.bufused);

    /* and add the new nodes */
    for (ni = 0; ni < buf.bufused; ni++, i++) {
        parent->children[i] = buf.buf[ni];
        parent->children[i]->parent = parent;
    }

    FREE_BUFFER(buf);

    return parent;
}

/* perform the given transformation on matching nodes */
void transform(TransformState *state, Node *node, TrFind *find, transform_func_t func)
{
    int then;
    size_t i;

    /* find a matching node */
    while (node) {
        /* first try the node itself */
        if (match(state, node, find)) {
            Node *snode;

            then = THEN_INNER_EXCLUSIVE;
            snode = func(state, node, &then);

            /* and figure out what to do next */
            if (snode) {
                node = snode;
                if (then == THEN_INNER_INCLUSIVE) continue;
                else if (then == THEN_OUTER) goto outer;
            }
        }

        /* then children nodes */
        if (node->children[0]) {
            node = node->children[0];
            continue;
        }

        /* then siblings */
outer:
        while (node) {
            Node *pnode = node->parent;
            if (pnode) {
                for (i = 0; pnode->children[i] && pnode->children[i] != node; i++);
                if (pnode->children[i]) i++;
                if (pnode->children[i]) {
                    /* OK, have a neighbor, continue from there */
                    node = pnode->children[i];
                    break;
                }
            }
            node = pnode;
        }
    }
}

/* starting from the given file (malloc'd, now owned by TransformState), read,
 * preprocess, and transform */
TransformState transformFile(char *filename)
{
    TransformState state;
    size_t i;
    pid_t pid, tmpp;
    int pipei[2], pipeo[2];
    int tmpi;
    FILE *f;
    Node *node;

    INIT_BUFFER(state.transforms);
    INIT_BUFFER(state.filenames);
    INIT_BUFFER(state.files);
    WRITE_ONE_BUFFER(state.filenames, filename);

    /* first stage just gets all the files input */
    for (i = 0; i < state.filenames.bufused; i++) {
        filename = state.filenames.buf[i];

        /* have we read, preprocessed and parsed the file? */
        if (state.files.bufused <= i) {
            /* we need to read in the file first! */
            struct Buffer_char loader, source;
            ScanState scanState;
            char *error;

            INIT_BUFFER(loader);
            WRITE_BUFFER(loader, "#include \"", 10);
            WRITE_BUFFER(loader, filename, strlen(filename));
            WRITE_BUFFER(loader, ".exc\"\n", 6);

            /* prepare to pipe it into/out of the child */
            SF(tmpi, pipe, -1, (pipei));
            SF(tmpi, pipe, -1, (pipeo));

            /* FIXME: the command should be in a spec file! */
            SF(pid, fork, -1, ());
            if (pid == 0) {
                SF(tmpi, close, -1, (pipei[1]));
                SF(tmpi, close, -1, (pipeo[0]));

                SF(tmpi, dup2, -1, (pipei[0], 0));
                SF(tmpi, dup2, -1, (pipeo[1], 1));

                SF(tmpi, close, -1, (pipei[0]));
                SF(tmpi, close, -1, (pipeo[1]));

                SF(tmpi, execlp, -1, ("gcc", "gcc", "-E", "-undef", "-x", "c", "-", NULL));
                exit(1);

            }

            SF(tmpi, close, -1, (pipei[0]));
            SF(tmpi, close, -1, (pipeo[1]));

            /* give them the loader */
            SF(tmpi, write, -1, (pipei[1], loader.buf, loader.bufused));
            SF(tmpi, close, -1, (pipei[1]));
            FREE_BUFFER(loader);

            /* read in the preprocessed source */
            SF(f, fdopen, NULL, (pipeo[0], "r"));
            INIT_BUFFER(source);
            READ_FILE_BUFFER(source, f);
            SF(tmpi, fclose, EOF, (f));
            WRITE_ONE_BUFFER(source, '\0');

            /* and wait for them */
            SF(tmpp, waitpid, -1, (pid, &tmpi, 0));
            if (tmpi != 0) {
                fprintf(stderr, "cpp failed!\n");
                exit(1);
            }

            /* now parse it */
            scanState = newScanState(i);
            scanState.buf = source;

            error = NULL;
            node = cparse(&scanState, &error);

            if (!node) {
                fprintf(stderr, "%s: %s\n", filename, error);
                FREE_BUFFER(source);
                continue;
            }

            WRITE_ONE_BUFFER(state.files, node);

        }

        /* run the imports */
        state.files.buf[i] = transformImportStage(&state, state.files.buf[i]);
    }

    return state;

}
