#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "buffer.h"
#include "node.h"
#include "spec.h"

BUFFER(Nodep, Node *);

typedef struct TransformState_ TransformState;
typedef Node *(*transform_func_t)(TransformState *, Node *, int *then, void *arg);
typedef int (*transform_condition_func_t)(TransformState *, Node *);
typedef Node *(*transform_stage_func_t)(TransformState *, Node *, int isprimary);

/* "then" options */
enum {
    THEN_INNER_INCLUSIVE, /* then check the node and all its children */
    THEN_INNER_EXCLUSIVE, /* then check the node's children (but not the node itself) */
    THEN_OUTER,           /* then check the node's neighbor */
    THEN_STOP             /* do not continue transformation */
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
    Node *header;
};

/* utility functions for transforms */

/* parenthesize a node */
Node *trParenthesize(Node *node);

/* replace a node */
void trReplace(Node *from, Node *to, int preserveWhitespace);

/* resize a node */
Node *trResize(Node *node, size_t to);

/* append nodes as children of an existing node, which can replace the node */
Node *trAppend(Node *parent, ...);

/* prepend a single node to an existing node */
Node *trPrepend(Node *parent, Node *child);

/* duplicate a tree of nodes */
Node *trDupNode(Node *node);

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
void transform(TransformState *state, Node *node, TrFind *find, transform_func_t func, void *arg);

/* starting from the given file (malloc'd, now owned by TransformState), read,
 * preprocess, and transform */
TransformState transformFile(Spec *spec, char *const cflags[], char *filename);

/* free a TransformState */
void freeTransformState(TransformState *state);

#endif
