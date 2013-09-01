#include <string.h>

#include "builtin-stages.h"
#include "parse.h"

/* @import stage */
static Node *transformImportStageF(TransformState *state, Node *node, int *then)
{
    /* found an @import, make sure it's a declaration */
    if (node->parent->type != NODE_DECORATION_DECLARATION) return node;

    fprintf(stderr, "Wooh.\n");
    return node;
}

void transformImportStage(TransformState *state, Node *node)
{
    TrFind find;

    /* search for @import */
    memset(&find, 0, sizeof(find));
    find.matchDecoration[0] = "import";
    transform(state, node, &find, transformImportStageF);
}

/* @extension stage */
void transformExtensionStage(TransformState *state, Node *node);

/* @raw stage */
void transformRawStage(TransformState *state, Node *node);

