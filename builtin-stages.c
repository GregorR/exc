#define _XOPEN_SOURCE 700 /* for strdup */

#include <string.h>

#include "builtin-stages.h"
#include "parse.h"
#include "unparse.h"

/* @import stage */
static Node *transformImportStageF(TransformState *state, Node *node, int *then, void *arg)
{
    struct Buffer_char fname;
    size_t i;

    /* found an @import, make sure it's a declaration */
    if (!node->parent ||
        !node->parent->parent ||
        node->parent->parent->type != NODE_DECORATION_DECLARATION) return node;

    /* get the filename */
    if (node->children[1]->type != NODE_NIL) {
        fname = cunparse(node->children[1]->children[0]);
        if (!fname.buf) return node;
    } else {
        fname.buf = strdup("");
        if (!fname.buf) return node;
    }

    /* swap the @import for a @include */
    free(node->children[0]->tok->tok);
    node->children[0]->tok->tok = strdup("include");
    if (!node->children[0]->tok->tok) {
        FREE_BUFFER(fname);
        return node;
    }

    if (node->children[1]->type != NODE_NIL) {
        trAppend(node->children[1]->children[0],
            newNode(NULL, NODE_TOK,
                newToken(TOK_DOT, 1, NULL, "."),
                0),
            newNode(NULL, NODE_TOK,
                newToken(TOK_ID, 1, NULL, "h"),
                0),
            NULL
        );
    }

    /* and add the file */
    for (i = 0; i < state->filenames.bufused && strcmp(fname.buf, state->filenames.buf[i]); i++);
    if (i < state->filenames.bufused) {
        /* already found! */
        FREE_BUFFER(fname);

    } else {
        WRITE_ONE_BUFFER(state->filenames, fname.buf);

    }

    return node;
}

Node *transformImportStage(TransformState *state, Node *node, int isprimary)
{
    TrFind find;

    /* search for @import */
    memset(&find, 0, sizeof(find));
    find.matchDecoration[0] = "import";
    transform(state, node, &find, transformImportStageF, NULL);

    return node;
}

/* @extension stage */
Node *transformExtensionStage(TransformState *state, Node *node, int isprimary)
{
    return node;
}

/* @raw stage */
static Node *transformRawStageF(TransformState *state, Node *node, int *then, void *arg)
{
    Node *nnode;

    /* switch based on which it is */
    char *type = node->children[0]->tok->tok;

    int ptype = 0;
    if (node->parent) {
        if (node->parent->type == NODE_DECLARATION_DECORATOR_LIST) {
            if (node->parent->parent)
                ptype = node->parent->parent->type;
        } else {
            ptype = node->parent->type;
        }
    }

    if (!strcmp(type, "rem")) {
        /* decorator declaration or decorator expression as expression statement */
        if (ptype == NODE_DECORATION_DECLARATION) {
            node = node->parent->parent;
            nnode = newNode(NULL, NODE_NIL, NULL, 0);
            trReplace(node, nnode, 1);
            freeNode(node);
            return nnode;

        }

        if (ptype == NODE_EXPRESSION_STATEMENT) {
            node = node->parent->parent->parent;
            nnode = newNode(NULL, NODE_NIL, NULL, 0);
            trReplace(node, nnode, 1);
            freeNode(node);
            return nnode;
        }

        return node;

    } else if (!strcmp(type, "raw")) {
        /* expression which is not an expression statement */
        if ((node->parent && node->parent->type != NODE_DECLARATION_DECORATOR_LIST) &&
            ptype != NODE_DECORATION_OP && ptype != NODE_EXPRESSION_STATEMENT) {
            Node *repl;

            if (node->children[1] && node->children[1]->children[0]) {
                /* we have an open, so use it */
                repl = trParenthesize(node->children[1]->children[0]);
                node->children[1]->children[0] = newNode(NULL, NODE_NIL, NULL, 0);

            } else {
                /* just an empty () */
                repl = trParenthesize(newNode(NULL, NODE_NIL, NULL, 0));

            }

            trReplace(node, repl, 1);
            freeNode(node);
            return repl;

        }

        /* operator declaration */
        else if (ptype == NODE_DECORATION_OP) {
            Node *repl;

            /* replace the node itself with its open part */
            if (node->children[1] && node->children[1]->children[0]) {
                repl = node->children[1]->children[0];
                trReplace(node, repl, 1);
                node->children[1]->children[0] = newNode(NULL, NODE_NIL, NULL, 0);
                freeNode(node);
                node = repl;
            } else {
                repl = newNode(NULL, NODE_NIL, NULL, 0);
                trReplace(node, repl, 1);
                freeNode(node);
                node = repl;
            }
            node = node->parent;

            /* surround both sides in parens */
            node->children[0] = trParenthesize(node->children[0]);
            node->children[0]->parent = node;
            node->children[2] = trParenthesize(node->children[2]);
            node->children[2]->parent = node;

            /* and parenthesize the entire node */
            repl = trParenthesize(node);
            trReplace(node, repl, 1);
            node->parent = repl;

            return node;

        }

        /* decorator declaration */
        else if (ptype == NODE_DECORATION_DECLARATION) {
            Node *pnode = node->parent->parent;

            if (node->children[1] && node->children[1]->children[0]) {
                Node *repl = node->children[1]->children[0];
                trReplace(pnode, repl, 1);
                node->children[1]->children[0] = newNode(NULL, NODE_NIL, NULL, 0);
                freeNode(pnode);
                return repl;

            } else {
                Node *repl = newNode(NULL, NODE_NIL, NULL, 0);
                trReplace(pnode, repl, 1);
                freeNode(pnode);
                return repl;

            }

        }

        /* otherwise, just replace it */
        else {
            Node *repl;
            if (node->children[1] && node->children[1]->children[0]) {
                struct Buffer_char test;
                test = cunparse(node->children[1]->children[0]);
                FREE_BUFFER(test);

                repl = node->children[1]->children[0];
                trReplace(node, repl, 1);
                node->children[1]->children[0] = newNode(NULL, NODE_NIL, NULL, 0);
                freeNode(node);

            } else {
                repl = newNode(NULL, NODE_NIL, NULL, 0);
                trReplace(node, repl, 1);
                freeNode(node);

            }

            return repl;

        }

    } else if (!strcmp(type, "include")) {
        /* must be a decoration declaration */
        if (ptype == NODE_DECORATION_DECLARATION) {
            struct Buffer_char fname, fnamestr;
            Node *pnode;

            /* manufacture a proper replacement */
            Node *repl = newNode(NULL, NODE_DECORATION_OPEN_CONT, NULL, 4);

            repl->children[0] = newNode(repl, NODE_TOK, newToken(TOK_PUNC_UNKNOWN, 1, "", "#"), 0);
            repl->children[1] = newNode(repl, NODE_TOK, newToken(TOK_ID, 1, "", "include"), 0);
            repl->children[3] = newNode(repl, NODE_TOK, newToken(TOK_PUNC_UNKNOWN, 1, "\n", ""), 0);

            /* unparse the raw part to get the filename */
            INIT_BUFFER(fnamestr);
            WRITE_ONE_BUFFER(fnamestr, '"');
            if (node->children[1] && node->children[1]->children[0]) {
                fname = cunparse(node->children[1]->children[0]);
                WRITE_BUFFER(fnamestr, fname.buf, fname.bufused - 1);
                FREE_BUFFER(fname);
            }
            WRITE_BUFFER(fnamestr, "\"", 2);

            /* then make that a node */
            repl->children[2] = newNode(repl, NODE_TOK, newToken(TOK_STR_LITERAL, 1, " ", fnamestr.buf), 0);
            FREE_BUFFER(fnamestr);

            /* and replace it */
            pnode = node->parent->parent;
            trReplace(pnode, repl, 1);
            freeNode(pnode);
            return repl;

        }

    }

    return node;
}

Node *transformRawStage(TransformState *state, Node *node, int isprimary)
{
    TrFind find;

    if (!isprimary) return node;

    /* search for @rem, @raw and @include */
    memset(&find, 0, sizeof(find));
    find.matchDecoration[0] = "rem";
    find.matchDecoration[1] = "raw";
    find.matchDecoration[2] = "include";
    transform(state, node, &find, transformRawStageF, NULL);

    return node;
}

