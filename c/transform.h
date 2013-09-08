#ifndef EXC_transform
#define EXC_transform 1
/*
 * Written in 2013 by Gregor Richards
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty. 
 *
 * You should have received a copy of the CC0 Public Domain Dedication along
 * with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>. 
 */ 

#include "node.h"

#include "spec.h"


#include "sys/types.h"


#include "buffer.h"


#line 32 "transform.exc"
BUFFER(Nodep, Node *);


#line 34 "transform.exc"
 typedef struct TransformState_ TransformState;

#line 35 "transform.exc"
 typedef Node *(*transform_func_t)(TransformState *, Node *, int *then, void *arg);

#line 36 "transform.exc"
 typedef int (*transform_condition_func_t)(TransformState *, Node *);

#line 37 "transform.exc"
 typedef Node *(*transform_stage_func_t)(TransformState *, Node *, int isprimary);

#line 38 "transform.exc"
 typedef transform_stage_func_t *(*transform_loader_func_t)(TransformState *);

/* "then" options */

#line 41 "transform.exc"
 enum {
    THEN_INNER_INCLUSIVE, /* then check the node and all its children */
    THEN_INNER_EXCLUSIVE, /* then check the node's children (but not the node itself) */
    THEN_OUTER, /* then check the node's neighbor */
    THEN_STOP /* do not continue transformation */
};

/* transforms themselves */

#line 49 "transform.exc"
 typedef struct Transform_ {
    const char *name;
    transform_stage_func_t func;
} Transform;
BUFFER(Transform, Transform);


#line 55 "transform.exc"
 struct TransformState_ {
    const char *bindir;
    struct Buffer_Transform transforms;
    struct Buffer_charp ppfilenames; /* filenames, as seen by the preprocessor */
    struct Buffer_charp filenames; /* filenames handled by exc */
    struct Buffer_Nodep files;
    Node *header;
};

/* a find request */


#line 66 "transform.exc"
 typedef struct TrFind_ {
    int matchNode[4];
    const char *matchDecoration[4];
    transform_condition_func_t matchFunc;

    int notInNode[4];
    const char *notInDecoration[4];
    transform_condition_func_t notInFunc;
} TrFind;

/* parenthesize a node */

#line 83 "transform.exc"
 Node *trParenthesize(Node *node);

/* replace a node */

#line 93 "transform.exc"
 void trReplace(Node *from, Node *to, int preserveWhitespace);

/* resize a node */

#line 141 "transform.exc"
 Node *trResize(Node *node, size_t to);

/* append nodes as children of an existing node */

#line 171 "transform.exc"
 Node *trAppend(Node *parent, ...);

/* prepend a single node to an existing node, and perhaps give it the
 * successor's whitespace */

#line 203 "transform.exc"
 Node *trPrepend(Node *parent, Node *child);

/* duplicate a tree of nodes */

#line 223 "transform.exc"
 Node *trDupNode(Node *node);

/* perform the given transformation on matching nodes */

#line 287 "transform.exc"
 void transform(TransformState *state, Node *node, TrFind *find, transform_func_t func, void *arg);

/* starting from the given file (malloc'd, now owned by TransformState), read,
 * preprocess, and transform */

#line 336 "transform.exc"
 TransformState transformFile(const char *bindir, Spec *spec, char *const cflags[], char *filename);

/* free a TransformState */

#line 419 "transform.exc"
 void freeTransformState(TransformState *state);
#endif
