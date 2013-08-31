#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "buffer.h"
#include "node.h"
#include "scan.h"

BUFFER(Tokenp, Token *);

enum Nodes {
    NODE_FIRST = 0,

#define NODE(x) NODE_ ## x,
#include "nodes.h"
#undef NODE

    NODE_LAST
};

typedef struct ParseState_ {
    ScanState *scanState;
    struct Buffer_Tokenp buf;
} ParseState;

Node *cparse(ScanState *state);

#endif
