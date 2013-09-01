#ifndef BUILTIN_STAGES_H
#define BUILTIN_STAGES_H

#include "transform.h"

/* @import stage */
void transformImportStage(TransformState *state, Node *node);

/* @extension stage */
void transformExtensionStage(TransformState *state, Node *node);

/* @raw stage */
void transformRawStage(TransformState *state, Node *node);

#endif
