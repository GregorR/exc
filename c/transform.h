#ifndef EXC_src_transform
#define EXC_src_transform 1
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


#line 32 "src/transform.exc"
BUFFER(Nodep, Node *);


#line 34 "src/transform.exc"
 typedef struct TransformState_ TransformState;

#line 35 "src/transform.exc"
 typedef Node *(*transform_func_t)(TransformState *, Node *, int *then, void *arg);

#line 36 "src/transform.exc"
 typedef int (*transform_condition_func_t)(TransformState *, Node *);

#line 37 "src/transform.exc"
 typedef Node *(*transform_stage_func_t)(TransformState *, Node *, int isprimary);

#line 38 "src/transform.exc"
 typedef void (*transform_loader_func_t)(TransformState *);

/* "then" options */

#line 41 "src/transform.exc"
 enum {
    THEN_INNER_INCLUSIVE, /* then check the node and all its children */
    THEN_INNER_EXCLUSIVE, /* then check the node's children (but not the node itself) */
    THEN_OUTER, /* then check the node's neighbor */
    THEN_STOP /* do not continue transformation */
};

/* transforms themselves */

#line 49 "src/transform.exc"
 typedef struct Transform_ {
    const char *name;
    transform_stage_func_t func;
} Transform;
BUFFER(Transform, Transform);


#line 55 "src/transform.exc"
 struct TransformState_ {
    const char *bindir;
    struct Buffer_charp extensions; /* loaded extensions */
    struct Buffer_Transform transforms; /* extension transforms */
    struct Buffer_charp ppfilenames; /* filenames, as seen by the preprocessor */
    struct Buffer_charp filenames; /* filenames handled by exc */
    struct Buffer_Nodep files;
    Node *header;
};

/* a find request */
#define TR_FIND_MATCH_CT 8

#line 67 "src/transform.exc"
 typedef struct TrFind_ {
    int matchNode[TR_FIND_MATCH_CT];
    const char *matchDecoration[TR_FIND_MATCH_CT];
    transform_condition_func_t matchFunc;

    int notInNode[TR_FIND_MATCH_CT];
    const char *notInDecoration[TR_FIND_MATCH_CT];
    transform_condition_func_t notInFunc;
} TrFind;

/* parenthesize a node */

#line 84 "src/transform.exc"
 Node *trParenthesize(Node *node);

/* replace a node */

#line 94 "src/transform.exc"
 void trReplace(Node *from, Node *to, int preserveWhitespace);

/* resize a node */

#line 142 "src/transform.exc"
 Node *trResize(Node *node, size_t to);

/* append nodes as children of an existing node */

#line 172 "src/transform.exc"
 Node *trAppend(Node *parent, ...);

/* insert a single node at a location within a given node */

#line 203 "src/transform.exc"
 Node *trInsert(Node *parent, size_t loc, Node *child);

/* insert before a given child node */

#line 223 "src/transform.exc"
 Node *trInsertBefore(Node *rel, Node *child);

/* insert after a given child node */

#line 236 "src/transform.exc"
 Node *trInsertAfter(Node *rel, Node *child);

/* prepend a single node to an existing node, and perhaps give it the
 * successor's whitespace */

#line 251 "src/transform.exc"
 Node *trPrepend(Node *parent, Node *child);

/* duplicate a tree of nodes */

#line 257 "src/transform.exc"
 Node *trDupNode(Node *node);

/* perform the given transformation on matching nodes */

#line 321 "src/transform.exc"
 void transform(TransformState *state, Node *node, TrFind *find, transform_func_t func, void *arg);

/* starting from the given file (malloc'd, now owned by TransformState), read,
 * preprocess, and transform */

#line 373 "src/transform.exc"
 TransformState transformFile(const char *bindir, Spec *spec, char *const cflags[], char *filename);

/* free a TransformState */

#line 465 "src/transform.exc"
 void freeTransformState(TransformState *state);

/* add an extension to the transform state */

#line 485 "src/transform.exc"
 void trAddStage(TransformState *state, const char *name, transform_stage_func_t func);
#endif
