#ifndef NODE_H
#define NODE_H

#include <sys/types.h>

typedef struct Token_ {
    size_t len;
    char *tok;
} Token;

typedef struct Node_ {
    struct Node_ *parent;
    int type;
    Token *tok;
    struct Node *children[1];
} Node;

#endif
