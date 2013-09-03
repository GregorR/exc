#ifndef BUILTIN_STAGES_H
#define BUILTIN_STAGES_H

#include "transform.h"

/* @import stage */
Node *transformImportStage(TransformState *state, Node *node, int isprimary);

/* @extension stage */
Node *transformExtensionStage(TransformState *state, Node *node, int isprimary);

/* @raw stage */
Node *transformRawStage(TransformState *state, Node *node, int isprimary);

#endif
