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

#line 13 "src/node.exc"
#define _XOPEN_SOURCE 700
#include "node.h"

#include "stdlib.h"

#include "stdio.h"

#include "string.h"


#include "helpers.h"


/* create a node */

#line 45 "src/node.exc"
 Node *newNode(Node *parent, int type, Token *tok, size_t children)
{
    Node *ret;
    SF(ret, calloc, NULL, (sizeof(Node) + children * sizeof(Node *), 1));
    ret->parent = parent;
    ret->type = type;
    ret->tok = tok;
    return ret;
}

/* free a node and all the tokens and nodes under it */

#line 56 "src/node.exc"
 void freeNode(Node *node)
{
    size_t i;
    for (i = 0; node->children[i]; i++)
        freeNode(node->children[i]);
    if (node->tok)
        freeToken(node->tok);
    free(node);
}

/* add a special to this node */

#line 67 "src/node.exc"
 NodeSpecial *nodeAddSpecial(Node *node, const char *type)
{
    NodeSpecial *ret;

    SF(ret, malloc, NULL, (sizeof(NodeSpecial)));
    ret->next = node->specials;
    node->specials = ret;

    SF(ret->type, strdup, NULL, (type));

    ret->val = NULL;
    ret->free = NULL;

    return ret;
}

/* Create a token. All memory becomes owned by the token. */

#line 84 "src/node.exc"
 Token *newToken(int type, int dup, char *pre, char *tok)
{
    Token *ret;
    SF(ret, malloc, NULL, (sizeof(Token)));
    ret->type = type;
    ret->idx = ret->l = ret->c = 0;
    ret->f = -1;

    if (pre == NULL) {
        SF(ret->pre, strdup, NULL, (""));
    } else if (dup) {
        SF(ret->pre, strdup, NULL, (pre));
    } else {
        ret->pre = pre;
    }

    if (tok == NULL) {
        SF(ret->tok, strdup, NULL, (""));
    } else if (dup) {
        SF(ret->tok, strdup, NULL, (tok));
    } else {
        ret->tok = tok;
    }

    return ret;
}

/* free a token */

#line 112 "src/node.exc"
 void freeToken(Token *tok)
{
    free(tok->pre);
    free(tok->tok);
    free(tok);
}
#line 1 "<stdin>"
