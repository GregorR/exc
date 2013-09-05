#ifndef EXC_builtin_stages
#define EXC_builtin_stages 1
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
#include "parse.h"

#include "transform.h"

 Node *transformImportStage(TransformState *state, Node *node, int isprimary);
/* @extension stage */
 Node *transformExtensionStage(TransformState *state, Node *node, int isprimary);
 Node *transformHeaderStage(TransformState *state, Node *node, int isprimary);
 Node *transformRawStage(TransformState *state, Node *node, int isprimary);
#endif
