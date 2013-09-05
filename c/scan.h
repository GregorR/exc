#ifndef EXC_scan
#define EXC_scan 1
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
#include "node.h"

#include "buffer.h"

 enum Tokens {
    TOK_FIRST = 0,
TOK_KEY_FIRST,
TOK_auto,
TOK_break,
TOK_case,
TOK_char,
TOK_const,
TOK_continue,
TOK_default,
TOK_do,
TOK_double,
TOK_else,
TOK_enum,
TOK_extern,
TOK_float,
TOK_for,
TOK_goto,
TOK_if,
TOK_inline,
TOK_int,
TOK_long,
TOK_register,
TOK_restrict,
TOK_return,
TOK_short,
TOK_signed,
TOK_sizeof,
TOK_static,
TOK_struct,
TOK_switch,
TOK_typedef,
TOK_union,
TOK_unsigned,
TOK_void,
TOK_volatile,
TOK_while,
TOK__Alignas,
TOK__Alignof,
TOK__Atomic,
TOK__Bool,
TOK__Complex,
TOK__Generic,
TOK__Imaginary,
TOK__Noreturn,
TOK__Static_assert,
TOK__Thread_local,
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
/* exc additions */
TOK_DECORATION,
TOK_OPEN_DECORATION,
TOK_CLOSE_DECORATION,
/* /exc additions */
TOK_PUNC_UNKNOWN,
TOK_PUNC_LAST,
TOK_TERM,
    TOK_LAST
};
 typedef struct ScanState_ {
    struct Buffer_char buf;
    size_t idx, f, l, c;
} ScanState;
 ScanState newScanState(size_t f);
 Token *cscan(ScanState *state);
#endif
