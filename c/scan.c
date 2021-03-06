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

#line 13 "src/scan.exc"
#define _XOPEN_SOURCE 700
#include "scan.h"


#include "stdio.h"

#include "stdlib.h"

#include "string.h"



#line 40 "src/scan.exc"
 ScanState newScanState(struct Buffer_charp *filenames)
{
    ScanState ret;
    ret.filenames = filenames;
    ret.buf.buf = NULL;
    ret.buf.bufused = ret.buf.bufsz = 0;
    ret.idx = 0;
    ret.f = 0;
    ret.l = ret.c = 1;
    return ret;
}

static size_t getFileNo(struct Buffer_charp *buf, char *name)
{
    size_t i;
    char *mname;

    if (buf == NULL) return 0;

    for (i = 0; i < buf->bufused; i++)
        if (!strcmp(buf->buf[i], name)) return i;

    /* add it */
    SF(mname, strdup, NULL, (name));
    WRITE_ONE_BUFFER(*buf, mname);
    return buf->bufused - 1;
}

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
    state->idx = ni;
}

static char *getWhite(ScanState *state)
{
    size_t fi = state->idx;
    size_t i = fi;
    struct Buffer_char *buf = &state->buf;
    char c = buf->buf[i];
    char *ret;

    if (!c) {
        SF(ret, strdup, NULL, (""));
        return ret;
    }

    while (ciswhite(c) ||
           (c == '/' && (buf->buf[i+1] == '/' || buf->buf[i+1] == '*'))) {
        if (c == '/') {
            if (buf->buf[i+1] == '/') {
                for (i+=2; buf->buf[i] && buf->buf[i] != '\n'; i++) {
                    if (buf->buf[i] == '\\' && buf->buf[i+1]) i++;
                }
            } else {
                for (i++; buf->buf[i+1] && (buf->buf[i] != '*' || buf->buf[i+1] != '/'); i++);
                if (buf->buf[i] == '*' && buf->buf[i+1] == '/') i += 2;
            }

        } else i++;

        c = buf->buf[i];
    }

    updateIdx(state, i);
    SF(ret, strndup, NULL, (buf->buf + fi, i - fi));
    return ret;
}


#line 153 "src/scan.exc"
 Token *cscan(ScanState *state)
{
    Token *ret = NULL;
    int ttype = 0;
    char *pre = NULL, *tok = NULL;
    size_t fi, i;
    struct Buffer_char *buf = &state->buf;
    char c;
    int firstChar;

    /* get any preceding whitespace */
    pre = getWhite(state);

    /* preprocessing cares if this is the first character in the file */
    firstChar = (state->idx == 0);

    if (0) {
        char *morePre, *cPre;
retryWhite:
        /* need more whitespace after a preprocessor directive */
        morePre = getWhite(state);
        SF(cPre, malloc, NULL, (strlen(pre) + strlen(morePre) + 1));
        sprintf(cPre, "%s%s", pre, morePre);
        free(pre);
        free(morePre);
        pre = cPre;
    }

    /* then get a token */
    fi = i = state->idx;
    c = buf->buf[i];

    if (c == '#') {
        /* a preprocessor directive; hopefully #line */
        if (!firstChar &&
            (!pre[0] || pre[strlen(pre)-1] != '\n')) {
            /* nope, just take this as a token */
            ttype = TOK_HASH;
            i++;

        } else {
            Token *line, *file;

            /* a real preprocessing directive */
            if (!strncmp(buf->buf + i, "#line ", 6)) {
                /* it's a #line directive */
                i += 6;

            } else if (!strncmp(buf->buf + i, "# ", 2) &&
                       (buf->buf[i+2] >= '0' && buf->buf[i+2] <= '9')) {
                /* it's a GNU-style #line thingy */
                i += 2;

            } else {
                /* it's... confusing */
                while (i < buf->bufused && buf->buf[i] != '\n') i++;
                updateIdx(state, i);
                goto retryWhite;

            }

            /* now just scan to get the vital elements */
            updateIdx(state, i);
            line = cscan(state);
            file = cscan(state);

            /* skip to the end of the line */
            i = state->idx;
            while (i < buf->bufused && buf->buf[i] != '\n') i++;
            if (buf->buf[i] == '\n') i++;
            updateIdx(state, i);

            /* and try to set our line number */
            if (line->type == TOK_INT_LITERAL && file->type == TOK_STR_LITERAL) {
                /* perfect! */
                long ln;
                char *fname;

                ln = strtol(line->tok, NULL, 10);
                if (file->tok[0] && file->tok[1]) {
                    fname = file->tok + 1;
                    file->tok[strlen(file->tok)-1] = '\0'; /* remove the ending " */
                } else {
                    /* weird filename? */
                    fname = "???";
                }

                state->f = getFileNo(state->filenames, fname);
                state->l = ln;
                state->c = 1;
            }
            freeToken(line);
            freeToken(file);

            /* that was basically whitespace, so retreat */
            goto retryWhite;
        }

    } else if (cisidnond(c)) {
        /* it's an identifier or keyword */
        for (i++; cisid(buf->buf[i]); i++);
        SF(tok, strndup, NULL, (buf->buf + fi, i - fi));

        /* now check if it's a keyword */

#line 1 "src/keywords.h"
if (!strcmp(tok, "auto")) { ttype = TOK_auto; } else
if (!strcmp(tok, "break")) { ttype = TOK_break; } else
if (!strcmp(tok, "case")) { ttype = TOK_case; } else
if (!strcmp(tok, "char")) { ttype = TOK_char; } else
if (!strcmp(tok, "const")) { ttype = TOK_const; } else
if (!strcmp(tok, "continue")) { ttype = TOK_continue; } else
if (!strcmp(tok, "default")) { ttype = TOK_default; } else
if (!strcmp(tok, "do")) { ttype = TOK_do; } else
if (!strcmp(tok, "double")) { ttype = TOK_double; } else
if (!strcmp(tok, "else")) { ttype = TOK_else; } else
if (!strcmp(tok, "enum")) { ttype = TOK_enum; } else
if (!strcmp(tok, "extern")) { ttype = TOK_extern; } else
if (!strcmp(tok, "float")) { ttype = TOK_float; } else
if (!strcmp(tok, "for")) { ttype = TOK_for; } else
if (!strcmp(tok, "goto")) { ttype = TOK_goto; } else
if (!strcmp(tok, "if")) { ttype = TOK_if; } else
if (!strcmp(tok, "inline")) { ttype = TOK_inline; } else
if (!strcmp(tok, "int")) { ttype = TOK_int; } else
if (!strcmp(tok, "long")) { ttype = TOK_long; } else
if (!strcmp(tok, "register")) { ttype = TOK_register; } else
if (!strcmp(tok, "restrict")) { ttype = TOK_restrict; } else
if (!strcmp(tok, "return")) { ttype = TOK_return; } else
if (!strcmp(tok, "short")) { ttype = TOK_short; } else
if (!strcmp(tok, "signed")) { ttype = TOK_signed; } else
if (!strcmp(tok, "sizeof")) { ttype = TOK_sizeof; } else
if (!strcmp(tok, "static")) { ttype = TOK_static; } else
if (!strcmp(tok, "struct")) { ttype = TOK_struct; } else
if (!strcmp(tok, "switch")) { ttype = TOK_switch; } else
if (!strcmp(tok, "typedef")) { ttype = TOK_typedef; } else
if (!strcmp(tok, "union")) { ttype = TOK_union; } else
if (!strcmp(tok, "unsigned")) { ttype = TOK_unsigned; } else
if (!strcmp(tok, "void")) { ttype = TOK_void; } else
if (!strcmp(tok, "volatile")) { ttype = TOK_volatile; } else
if (!strcmp(tok, "while")) { ttype = TOK_while; } else
if (!strcmp(tok, "_Alignas")) { ttype = TOK__Alignas; } else
if (!strcmp(tok, "_Alignof")) { ttype = TOK__Alignof; } else
if (!strcmp(tok, "_Atomic")) { ttype = TOK__Atomic; } else
if (!strcmp(tok, "_Bool")) { ttype = TOK__Bool; } else
if (!strcmp(tok, "_Complex")) { ttype = TOK__Complex; } else
if (!strcmp(tok, "_Generic")) { ttype = TOK__Generic; } else
if (!strcmp(tok, "_Imaginary")) { ttype = TOK__Imaginary; } else
if (!strcmp(tok, "_Noreturn")) { ttype = TOK__Noreturn; } else
if (!strcmp(tok, "_Static_assert")) { ttype = TOK__Static_assert; } else
if (!strcmp(tok, "_Thread_local")) { ttype = TOK__Thread_local; } else

#line 260 "src/scan.exc"
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
                } else if (nc == '@') {
                    i++;
                    ttype = TOK_OPEN_DECORATION;
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

            case '@':
                if (nc == '/') {
                    i++;
                    ttype = TOK_CLOSE_DECORATION;
                } else {
                    ttype = TOK_DECORATION;
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

    if (!tok)
        SF(tok, strndup, NULL, (buf->buf + fi, i - fi));

    SF(ret, calloc, NULL, (sizeof(Token), 1));

    ret->type = ttype;
    ret->idx = state->idx;
    ret->f = state->f;
    ret->l = state->l;
    ret->c = state->c;
    ret->pre = pre;
    ret->tok = tok;

    updateIdx(state, i);
    return ret;
}
#line 1 "<stdin>"
