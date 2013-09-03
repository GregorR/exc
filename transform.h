#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "buffer.h"
#include "node.h"

BUFFER(charp, char *);
BUFFER(Nodep, Node *);

typedef struct TransformState_ TransformState;
typedef Node *(*transform_func_t)(TransformState *, Node *, int *then);
typedef int (*transform_condition_func_t)(TransformState *, Node *);
typedef Node *(*transform_stage_func_t)(TransformState *, Node *, int isprimary);

/* "then" options */
enum {
    THEN_INNER_INCLUSIVE, /* then check the node and all its children */
    THEN_INNER_EXCLUSIVE, /* then check the node's children (but not the node itself) */
    THEN_OUTER            /* then check the node's neighbor */
};

/* transforms themselves */
typedef struct Transform_ {
    const char *name;
    transform_stage_func_t func;
} Transform;
BUFFER(Transform, Transform);

struct TransformState_ {
    struct Buffer_Transform transforms;
    struct Buffer_charp filenames; /* memory owned by the transform state */
    struct Buffer_Nodep files;
};

/* utility functions for transforms */

/* resize a node */
Node *trResize(Node *node, size_t to);

/* append nodes as children of an existing node, which can replace the node */
Node *trAppend(Node *parent, ...);

/* a find request */
#define TR_FIND_MATCH_CT 4
typedef struct TrFind_ {
    int matchNode[TR_FIND_MATCH_CT];
    const char *matchDecoration[TR_FIND_MATCH_CT];
    transform_condition_func_t matchFunc;

    int notInNode[TR_FIND_MATCH_CT];
    const char *notInDecoration[TR_FIND_MATCH_CT];
    transform_condition_func_t notInFunc;
} TrFind;

/* perform the given transformation on matching nodes */
void transform(TransformState *state, Node *node, TrFind *find, transform_func_t func);

/* starting from the given file (malloc'd, now owned by TransformState), read,
 * preprocess, and transform */
TransformState transformFile(char *filename);

#endif
