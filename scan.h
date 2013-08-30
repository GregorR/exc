#ifndef SCAN_H
#define SCAN_H

#include "buffer.h"
#include "node.h"

enum Tokens {
    TOK_FIRST = 0,

    TOK_KEY_FIRST,
#define KEYWORD(x) TOK_ ## x,
#include "keywords.h"
#undef KEYWORD
    TOK_KEY_LAST,

    TOK_ID,

    TOK_LITERAL_FIRST,
    TOK_INT_LITERAL,
    TOK_FLOAT_LITERAL,
    TOK_CHAR_LITERAL,
    TOK_STR_LITERAL,
    TOK_LITERAL_LAST,

    TOK_PUNC_FIRST,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_DOT,
    TOK_ARROW,
    TOK_PLUSPLUS,
    TOK_MINUSMINUS,
    TOK_AND,
    TOK_STAR,
    TOK_PLUS,
    TOK_MINUS,
    TOK_BNOT,
    TOK_NOT,
    TOK_DIV,
    TOK_MOD,
    TOK_SHL,
    TOK_SHR,
    TOK_LT,
    TOK_GT,
    TOK_LTE,
    TOK_GTE,
    TOK_EQ,
    TOK_NEQ,
    TOK_BXOR,
    TOK_BOR,
    TOK_ANDAND,
    TOK_OROR,
    TOK_HOOK,
    TOK_COLON,
    TOK_SEMICOLON,
    TOK_DOTDOTDOT,
    TOK_ASG_START,
    TOK_ASG,
    TOK_MULASG,
    TOK_DIVASG,
    TOK_MODASG,
    TOK_ADDASG,
    TOK_SUBASG,
    TOK_SHLASG,
    TOK_SHRASG,
    TOK_BANDASG,
    TOK_BXORASG,
    TOK_BORASG,
    TOK_ASG_END,
    TOK_COMMA,
    TOK_HASH,
    TOK_HASHHASH,
    TOK_PUNC_UNKNOWN,
    TOK_PUNC_LAST,

    TOK_TERM,

    TOK_LAST
};

typedef struct ScanState_ {
    struct Buffer_char buf;
    size_t idx, f, l, c;
} ScanState;

Token *cscan(ScanState *state);

void freeToken(Token *tok);

#endif
