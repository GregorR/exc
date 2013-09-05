#ifndef EXC_unparse
#define EXC_unparse 1
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
#include "stdio.h"

#include "stdlib.h"

#include "sys/types.h"

#include "buffer.h"

#include "node.h"

/* convert a token type to a name */
 const char *tokenName(int n);
/* convert a node type (numeric) to a name */
 const char *nodeName(int n);
 struct Buffer_char cunparse(Node *node);
 struct Buffer_char cunparseStrLiteral(Token *tok);
 struct Buffer_char cunparseJSON(Node *node);
#endif
