#define _XOPEN_SOURCE 700 /* for strdup */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "helpers.h"
#include "node.h"

/* create a node */
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
void freeNode(Node *node)
{
    size_t i;
    for (i = 0; node->children[i]; i++)
        freeNode(node->children[i]);
    if (node->tok)
        freeToken(node->tok);
    free(node);
}

/* Create a token. All memory becomes owned by the token. */
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
void freeToken(Token *tok)
{
    free(tok->pre);
    free(tok->tok);
    free(tok);
}
