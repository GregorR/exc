TOKEN(KEY_FIRST)
#define KEYWORD(x) TOKEN(x)
#include "keywords.h"
#undef KEYWORD
TOKEN(KEY_LAST)

TOKEN(ID)

TOKEN(LITERAL_FIRST)
TOKEN(INT_LITERAL)
TOKEN(FLOAT_LITERAL)
TOKEN(CHAR_LITERAL)
TOKEN(STR_LITERAL)
TOKEN(LITERAL_LAST)

TOKEN(PUNC_FIRST)
TOKEN(LBRACKET)
TOKEN(RBRACKET)
TOKEN(LPAREN)
TOKEN(RPAREN)
TOKEN(LBRACE)
TOKEN(RBRACE)
TOKEN(DOT)
TOKEN(ARROW)
TOKEN(PLUSPLUS)
TOKEN(MINUSMINUS)
TOKEN(AND)
TOKEN(STAR)
TOKEN(PLUS)
TOKEN(MINUS)
TOKEN(BNOT)
TOKEN(NOT)
TOKEN(DIV)
TOKEN(MOD)
TOKEN(SHL)
TOKEN(SHR)
TOKEN(LT)
TOKEN(GT)
TOKEN(LTE)
TOKEN(GTE)
TOKEN(EQ)
TOKEN(NEQ)
TOKEN(BXOR)
TOKEN(BOR)
TOKEN(ANDAND)
TOKEN(OROR)
TOKEN(HOOK)
TOKEN(COLON)
TOKEN(SEMICOLON)
TOKEN(DOTDOTDOT)
TOKEN(ASG_START)
TOKEN(ASG)
TOKEN(MULASG)
TOKEN(DIVASG)
TOKEN(MODASG)
TOKEN(ADDASG)
TOKEN(SUBASG)
TOKEN(SHLASG)
TOKEN(SHRASG)
TOKEN(BANDASG)
TOKEN(BXORASG)
TOKEN(BORASG)
TOKEN(ASG_END)
TOKEN(COMMA)
TOKEN(HASH)
TOKEN(HASHHASH)
TOKEN(PUNC_UNKNOWN)
TOKEN(PUNC_LAST)

TOKEN(TERM)
