#ifndef PARSE_H
#define PARSE_H

#include "buffer.h"
#include "node.h"
#include "scan.h"

BUFFER(Tokenp, Token *);

enum Nodes {
    NODE_FIRST = 0,

    NODE_TOK,

    NODE_STR_LITERAL,

    NODE_PAREN,

    NODE_GENERIC_SELECTION,
    NODE_GENERIC_ASSOC_LIST,
    NODE_GENERIC_ASSOCIATION,
    NODE_GENERIC_ASSOCIATION_DEFAULT,

    NODE_INDEX_EXPRESSION,
    NODE_CALL_EXPRESSION,
    NODE_MEMBER_DOT_EXPRESSION,
    NODE_MEMBER_ARROW_EXPRESSION,

    NODE_LAST
};

typedef struct ParseState_ {
    ScanState scanState;
    struct Buffer_Tokenp buf;
} ParseState;

#endif
