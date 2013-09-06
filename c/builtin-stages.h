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



#line 76 "builtin-stages.exc"
 Node *transformImportStage(TransformState *state, Node *node, int isprimary);

/* @extension stage */

#line 89 "builtin-stages.exc"
 Node *transformExtensionStage(TransformState *state, Node *node, int isprimary);


#line 245 "builtin-stages.exc"
 Node *transformHeaderStage(TransformState *state, Node *node, int isprimary);


#line 479 "builtin-stages.exc"
 Node *transformRawStage(TransformState *state, Node *node, int isprimary);
#endif
