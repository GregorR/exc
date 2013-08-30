#define _XOPEN_SOURCE 700 /* for strndup */

#include <stdlib.h>
#include <string.h>

#include "scan.h"

static int ciswhite(char c)
{
    return (c == ' ' ||
            c == '\t' ||
            c == '\n' ||
            c == '\v' ||
            c == '\r');
}

static int cisidnond(char c)
{
    return ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '_');
}

static int cisdigit(char c)
{
    return (c >= '0' && c <= '9');
}

static int cishexdigit(char c)
{
    return (cisdigit(c) ||
            (c >= 'A' && c <= 'F') ||
            (c >= 'a' && c <= 'f'));
}

static int cisid(char c)
{
    return cisidnond(c) || cisdigit(c);
}

static void updateIdx(ScanState *state, size_t ni)
{
    size_t i;
    for (i = state->idx; i < ni; i++) {
        switch (state->buf.buf[i]) {
            case '\n':
                state->l++;
                state->c = 1;
                break;

            default:
                state->c++;
        }
    }
}

static char *getWhite(ScanState *state)
{
    size_t fi = state->idx;
    size_t i = fi;
    struct Buffer_char *buf = &state->buf;
    char c = buf->buf[i];

    if (!c) return strdup("");

    while (ciswhite(c) ||
           (c == '/' && (buf->buf[i+1] == '/' || buf->buf[i+1] == '*'))) {
        if (c == '/') {
            if (buf->buf[i+1] == '/') {
                for (i+=2; buf->buf[i] && buf->buf[i] != '\n'; i++) {
                    if (buf->buf[i] == '\\' && buf->buf[i+1]) i++;
                }
            } else {
                for (i++; buf->buf[i+1] && (buf->buf[i] != '*' || buf->buf[i+1] != '/'); i++);
            }
        } else i++;
    }

    updateIdx(state, i);
    return strndup(buf->buf + fi, i - fi);
}

Token *cscan(ScanState *state)
{
    Token *ret = NULL;
    int ttype = 0;
    char *pre = NULL, *tok = NULL;
    size_t fi, i;
    struct Buffer_char *buf = &state->buf;
    char c;

    /* get any preceding whitespace */
    pre = getWhite(state);

    /* then get a token */
    fi = i = state->idx;
    c = buf->buf[i];
    if (cisidnond(c)) {
        /* it's an identifier or keyword */
        for (i++; cisid(buf->buf[i]); i++);
        tok = strndup(buf->buf + fi, i - fi);
        if (!tok) goto fail; 

        /* now check if it's a keyword */
#define KEYWORD(k) if (!strcmp(tok, #k)) { ttype = TOK_ ## k; } else
#include "keywords.h"
#undef KEYWORD
        { ttype = TOK_ID; }

    } else if (cisdigit(c) || (c == '.' && cisdigit(buf->buf[i+1]))) {
        /* a number of some kind; and that can be a LOT of different kinds! */
        int ishex = 0, hasdot = 0, hasexponent = 0, hasfsuffix = 0;

        /* first off, what flavor of number is it? */
        if (c == '0') {
            if (buf->buf[i+1] == 'x') {
                i += 2;
                ishex = 1;
            }
        }

        /* hex has more digits, which makes it a bit trickier to parse */
        if (ishex) {
            /* first part */
            for (c = buf->buf[i]; c && cishexdigit(c); i++, c = buf->buf[i]);

            /* maybe a dot? */
            if (c == '.') {
                /* optional fractional part */
                hasdot = 1;
                for (i++, c = buf->buf[i];
                     c && cishexdigit(c);
                     i++, c = buf->buf[i]);
            }

            /* maybe a p or P (exponent part) */
            if (c == 'p' || c == 'P') {
                hasexponent = 1;
                i++; c = buf->buf[i];
                if (c == '-' || c == '+') {
                    i++; c = buf->buf[i];
                }

                for (; c && cisdigit(c); i++, c = buf->buf[i]);
            }

        } else {
            /* first part */
            for (c = buf->buf[i]; c && cisdigit(c); i++, c = buf->buf[i]);

            /* maybe a dot? */
            if (c == '.') {
                /* optional fractional part */
                hasdot = 1;
                for (i++, c = buf->buf[i];
                     c && cisdigit(c);
                     i++, c = buf->buf[i]);
            }

            /* maybe an e or E (exponent part) */
            if (c == 'e' || c == 'E') {
                hasexponent = 1;
                i++; c = buf->buf[i];
                if (c == '-' || c == '+') {
                    i++; c = buf->buf[i];
                }

                for (; c && cisdigit(c); c = buf->buf[i]);
            }

        }

        /* and finally, an optional suffix */
        for (;
             c &&
             (c == 'u' || c == 'U' ||
              c == 'l' || c == 'L' ||
              c == 'f' || c == 'F');
             i++, c = buf->buf[i]) {
            if (c == 'f' || c == 'F') hasfsuffix = 1;
        }

        if (hasdot || hasexponent || hasfsuffix) {
            ttype = TOK_FLOAT_LITERAL;
        } else {
            ttype = TOK_INT_LITERAL;
        }

    } else if (c == '\'' ||
               ((c == 'L' || c == 'u' || c == 'U') && buf->buf[i+1] == '\'')) {
        /* a character literal, find the match */
        if (c != '\'') i++;
        for (i++, c = buf->buf[i];
             c && c != '\'';
             i++, c = buf->buf[i]) {
            if (c == '\\' && buf->buf[i+1]) i++;
        }
        if (c == '\'') i++;
        ttype = TOK_CHAR_LITERAL;

    } else if (c == '"' ||
               ((c == 'u' || c == 'U' || c == 'L') && buf->buf[i+1] == '"') ||
               (c == 'u' && buf->buf[i+1] == '8' && buf->buf[i+2] == '"')) {
        /* a string literal */
        while (c != '"') {
            i++, c = buf->buf[i];
        }

        for (i++, c = buf->buf[i];
             c && c != '"';
             i++, c = buf->buf[i]) {
            if (c == '\\' && buf->buf[i+1]) i++;
        }
        if (c == '"') i++;
        ttype = TOK_STR_LITERAL;

    } else if (c) {
        /* must be a punctuator of some kind */
        char nc = buf->buf[++i];
        switch (c) {
            case '[':
                ttype = TOK_LBRACKET;
                break;

            case ']':
                ttype = TOK_RBRACKET;
                break;

            case '(':
                ttype = TOK_LPAREN;
                break;

            case ')':
                ttype = TOK_RPAREN;
                break;

            case '{':
                ttype = TOK_LBRACE;
                break;

            case '}':
                ttype = TOK_RBRACE;
                break;

            case '.':
                if (nc == '.' && buf->buf[i+1] == '.') {
                    i += 2;
                    ttype = TOK_DOTDOTDOT;
                } else {
                    ttype = TOK_DOT;
                }
                break;

            case '-':
                if (nc == '>') {
                    i++;
                    ttype = TOK_ARROW;
                } else if (nc == '-') {
                    i++;
                    ttype = TOK_MINUSMINUS;
                } else if (nc == '=') {
                    i++;
                    ttype = TOK_SUBASG;
                } else {
                    ttype = TOK_MINUS;
                }
                break;

            case '+':
                if (nc == '+') {
                    i++;
                    ttype = TOK_PLUSPLUS;
                } else if (nc == '=') {
                    i++;
                    ttype = TOK_ADDASG;
                } else {
                    ttype = TOK_PLUS;
                }
                break;

            case '&':
                if (nc == '&') {
                    i++;
                    ttype = TOK_ANDAND;
                } else if (nc == '=') {
                    i++;
                    ttype = TOK_BANDASG;
                } else {
                    ttype = TOK_AND;
                }
                break;

            case '*':
                if (nc == '=') {
                    i++;
                    ttype = TOK_MULASG;
                } else {
                    ttype = TOK_STAR;
                }
                break;

            case '~':
                ttype = TOK_BNOT;
                break;

            case '!':
                if (nc == '=') {
                    i++;
                    ttype = TOK_NEQ;
                } else {
                    ttype = TOK_NOT;
                }
                break;

            case '/':
                if (nc == '=') {
                    i++;
                    ttype = TOK_DIVASG;
                } else {
                    ttype = TOK_DIV;
                }
                break;

            case '%':
                if (nc == '=') {
                    i++;
                    ttype = TOK_MODASG;
                } else {
                    ttype = TOK_MOD;
                }
                break;

            case '<':
                if (nc == '<') {
                    i++;
                    if (buf->buf[i] == '=') {
                        i++;
                        ttype = TOK_SHLASG;
                    } else {
                        ttype = TOK_SHL;
                    }
                } else if (nc == '=') {
                    i++;
                    ttype = TOK_LTE;
                } else {
                    ttype = TOK_LT;
                }
                break;

            case '>':
                if (nc == '>') {
                    i++;
                    if (buf->buf[i] == '=') {
                        i++;
                        ttype = TOK_SHRASG;
                    } else {
                        ttype = TOK_SHR;
                    }
                } else if (nc == '=') {
                    i++;
                    ttype = TOK_GTE;
                } else {
                    ttype = TOK_GT;
                }
                break;

            case '=':
                if (nc == '=') {
                    i++;
                    ttype = TOK_EQ;
                } else {
                    ttype = TOK_ASG;
                }
                break;

            case '^':
                if (nc == '=') {
                    i++;
                    ttype = TOK_BXORASG;
                } else {
                    ttype = TOK_BXOR;
                }
                break;

            case '|':
                if (nc == '|') {
                    i++;
                    ttype = TOK_OROR;
                } else if (nc == '=') {
                    i++;
                    ttype = TOK_BORASG;
                } else {
                    ttype = TOK_BOR;
                }
                break;

            case '?':
                ttype = TOK_HOOK;
                break;

            case ':':
                ttype = TOK_COLON;
                break;

            case ';':
                ttype = TOK_SEMICOLON;
                break;

            case ',':
                ttype = TOK_COMMA;
                break;

            case '#':
                if (nc == '#') {
                    i++;
                    ttype = TOK_HASHHASH;
                } else {
                    ttype = TOK_HASH;
                }
                break;

            default:
                ttype = TOK_PUNC_UNKNOWN;
        }

    }

    if (fi == i) {
        /* didn't progress, just add the terminator */
        ttype = TOK_TERM;
    }

    updateIdx(state, i);
    tok = strndup(buf->buf + fi, i - fi);
    if (!tok) goto fail;

    ret = calloc(sizeof(Token), 1);
    if (!ret) goto fail;

    ret->type = ttype;
    ret->f = state->f;
    ret->l = state->l;
    ret->c = state->c;
    ret->pre = pre;
    ret->tok = tok;
    return ret;

fail:
    free(pre);
    free(tok);
    free(ret);
    return NULL;
}

void freeToken(Token *tok)
{
    free(tok->pre);
    free(tok->tok);
    free(tok);
}
