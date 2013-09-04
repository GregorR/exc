#ifndef EXC_builtin_stages
#define EXC_builtin_stages 1

#include "transform.h"

 Node *transformImportStage(TransformState *state, Node *node, int isprimary);
/* @extension stage */
 Node *transformExtensionStage(TransformState *state, Node *node, int isprimary);
 Node *transformHeaderStage(TransformState *state, Node *node, int isprimary);
 Node *transformRawStage(TransformState *state, Node *node, int isprimary);
#endif
