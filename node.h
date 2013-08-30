#ifndef NODE_H
#define NODE_H

#include <sys/types.h>

typedef struct Token_ {
    int type;
    size_t f, l, c;
    char *pre, *tok;
} Token;

typedef struct Node_ {
    struct Node_ *parent;
    int type;
    Token *tok;
    struct Node_ *children[1];
} Node;

Token *newToken(int type, size_t toklen, char *tok);
Node *newNode(Node *parent, int type, Token *tok, size_t len);

#endif
