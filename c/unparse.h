#ifndef EXC_src_unparse
#define EXC_src_unparse 1
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


#include "stdio.h"

#include "stdlib.h"

#include "sys/types.h"


#include "buffer.h"


/* convert a token type to a name */

#line 27 "src/unparse.exc"
 const char *tokenName(int n);

/* convert a node type (numeric) to a name */

#line 38 "src/unparse.exc"
 const char *nodeName(int n);


#line 107 "src/unparse.exc"
 struct Buffer_char cunparse(struct Buffer_charp *filenames, Node *node);



#line 118 "src/unparse.exc"
 struct Buffer_char cunparseStrLiteral(Token *tok);


#line 268 "src/unparse.exc"
 struct Buffer_char cunparseJSON(Node *node);
#endif
