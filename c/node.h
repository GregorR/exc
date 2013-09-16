#ifndef EXC_src_node
#define EXC_src_node 1
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

#include "sys/types.h"



#line 23 "src/node.exc"
 typedef struct Token_ {
    int type;
    size_t idx, f, l, c;
    char *pre, *tok;
} Token;


#line 29 "src/node.exc"
 typedef struct Node_ {
    struct Node_ *parent;
    int type;
    Token *tok;
    struct Node_ *children[1];
} Node;

/* create a node */

#line 37 "src/node.exc"
 Node *newNode(Node *parent, int type, Token *tok, size_t children);

/* free a node and all the tokens and nodes under it */

#line 48 "src/node.exc"
 void freeNode(Node *node);

/* Create a token. All memory becomes owned by the token. */

#line 59 "src/node.exc"
 Token *newToken(int type, int dup, char *pre, char *tok);

/* free a token */

#line 87 "src/node.exc"
 void freeToken(Token *tok);
#endif
