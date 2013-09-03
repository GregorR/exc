#define _XOPEN_SOURCE 700 /* for strdup */

#include <string.h>

#include "builtin-stages.h"
#include "parse.h"
#include "unparse.h"

/* @import stage */
static Node *transformImportStageF(TransformState *state, Node *node, int *then)
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
    transform(state, node, &find, transformImportStageF);

    return node;
}

/* @extension stage */
Node *transformExtensionStage(TransformState *state, Node *node, int isprimary)
{
    return node;
}

/* @raw stage */
static Node *transformRawStageF(TransformState *state, Node *node, int *then)
{
    /* switch based on which it is */
    char *type = node->children[0]->tok->tok;
    if (!strcmp(type, "rem")) {

    } else if (!strcmp(type, "raw")) {

    } else if (!strcmp(type, "include")) {

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
    transform(state, node, &find, transformRawStageF);

    return node;
}

