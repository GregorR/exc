#ifndef NODE_H
#define NODE_H

#include <sys/types.h>

typedef struct Token_ {
    int type;
    size_t idx, f, l, c;
    char *pre, *tok;
} Token;

typedef struct Node_ {
    struct Node_ *parent;
    int type;
    Token *tok;
    struct Node_ *children[1];
} Node;

Token *newToken(int type, int dup, char *pre, char *tok);
void freeToken(Token *);

Node *newNode(Node *parent, int type, Token *tok, size_t len);
void freeNode(Node *);

#endif
