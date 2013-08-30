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

    NODE_TOK,

    NODE_STR_LITERAL,

    NODE_PAREN,

    NODE_GENERIC_SELECTION,
    NODE_GENERIC_ASSOC_LIST,
    NODE_GENERIC_ASSOCIATION,
    NODE_GENERIC_ASSOCIATION_DEFAULT,

    NODE_INDEX,
    NODE_CALL,
    NODE_MEMBER_DOT,
    NODE_MEMBER_ARROW,
    NODE_POSTINC,
    NODE_POSTDEC,
    NODE_COMPOUND_LITERAL,

    NODE_ARGUMENT_EXPRESSION_LIST,

    NODE_PREINC,
    NODE_PREDEC,
    NODE_ADDROF,
    NODE_DEREF,
    NODE_POSITIVE,
    NODE_NEGATIVE,
    NODE_BNOT,
    NODE_NOT,
    NODE_SIZEOF_EXP,
    NODE_SIZEOF_TYPE,
    NODE_ALIGNOF,

    NODE_CAST,

    NODE_MUL,
    NODE_DIV,
    NODE_MOD,

    NODE_ADD,
    NODE_SUB,

    NODE_SHL,
    NODE_SHR,

    NODE_LT,
    NODE_GT,
    NODE_LTE,
    NODE_GTE,

    NODE_EQ,
    NODE_NEQ,

    NODE_BAND,

    NODE_BXOR,

    NODE_BOR,

    NODE_AND,

    NODE_OR,

    NODE_LAST
};

typedef struct ParseState_ {
    ScanState scanState;
    struct Buffer_Tokenp buf;
} ParseState;

#endif
