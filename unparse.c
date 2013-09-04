#include <string.h>

#include "unparse.h"

#include "scan.h"
#include "parse.h"

/* convert a token type to a name */
const char *tokenName(int n)
{
    switch (n) {
#define TOKEN(x) case TOK_ ## x: return #x;
#include "tokens.h"
#undef TOKEN
        default: return "???";
    }
}

/* convert a node type (numeric) to a name */
const char *nodeName(int n)
{
    switch (n) {
#define NODE(x) case NODE_ ## x: return #x;
#include "nodes.h"
#undef NODE
        default: return "???";
    }
}

static void unparsePrime(struct Buffer_char *buf, Node *node)
{
    size_t i;

    if (node->tok) {
        Token *tok = node->tok;
        if (tok->pre)
            WRITE_BUFFER(*buf, tok->pre, strlen(tok->pre));
        if (tok->tok)
            WRITE_BUFFER(*buf, tok->tok, strlen(tok->tok));
    }

    for (i = 0; node->children[i]; i++) {
        unparsePrime(buf, node->children[i]);
    }
}

struct Buffer_char cunparse(Node *node)
{
    struct Buffer_char buf;
    INIT_BUFFER(buf);
    unparsePrime(&buf, node);
    WRITE_ONE_BUFFER(buf, '\0');
    return buf;
}


struct Buffer_char cunparseStrLiteral(Token *tok)
{
    struct Buffer_char buf;
    size_t i;
    char xspace[3];
    char *sl = tok->tok;
    char delimiter = sl[0];
    INIT_BUFFER(buf);

    for (i = 1; sl[i]; i++) {
        char w = sl[i];

        if (sl[i] == delimiter) break;

        if (sl[i] == '\\') {
            /* an escape sequence */
            switch (sl[++i]) {
                case '0':
                    w = '\0';
                    break;

                case 'b':
                    w = '\b';
                    break;

                case 'f':
                    w = '\f';
                    break;

                case 'n':
                    w = '\n';
                    break;

                case 'r':
                    w = '\r';
                    break;

                case 't':
                    w = '\t';
                    break;

                case 'x':
                    if (sl[++i]) {
                        xspace[0] = sl[i]; 
                        if (sl[++i]) {
                            xspace[1] = sl[i];
                            xspace[2] = '\0';
                        } else xspace[1] = '\0';
                    } else xspace[0] = '\0';
                    w = (char) strtol(xspace, NULL, 16);
                    break;

                default:
                    w = sl[i];
                    break;
            }
        }

        WRITE_ONE_BUFFER(buf, w);
    }

    WRITE_ONE_BUFFER(buf, '\0');

    return buf;
}


static void unparseJSONStr(struct Buffer_char *buf, const char *str)
{
    size_t i;
    for (i = 0; str[i]; i++) {
        char c = str[i];
        switch (c) {
            case '"':
            case '\\':
                WRITE_ONE_BUFFER(*buf, '\\');
                WRITE_ONE_BUFFER(*buf, c);
                break;

            case '\b':
                WRITE_BUFFER(*buf, "\\b", 2);
                break;

            case '\f':
                WRITE_BUFFER(*buf, "\\f", 2);
                break;

            case '\n':
                WRITE_BUFFER(*buf, "\\n", 2);
                break;

            case '\r':
                WRITE_BUFFER(*buf, "\\r", 2);
                break;

            case '\t':
                WRITE_BUFFER(*buf, "\\t", 2);
                break;

            default:
                WRITE_ONE_BUFFER(*buf, c);
        }
    }
}

static void unparseJSONPrime(struct Buffer_char *buf, Node *node)
{
    size_t i;
    const char *tmp;

    WRITE_ONE_BUFFER(*buf, '{');

    if (node->tok) {
        Token *tok = node->tok;
        WRITE_BUFFER(*buf, "\"tok\":{\"type\":\"", 15);

        tmp = tokenName(tok->type);
        unparseJSONStr(buf, tmp);

        WRITE_ONE_BUFFER(*buf, '"');

        if (tok->pre) {
            WRITE_BUFFER(*buf, ",\"pre\":\"", 8);
            unparseJSONStr(buf, tok->pre);
            WRITE_ONE_BUFFER(*buf, '"');
        }

        if (tok->tok) {
            WRITE_BUFFER(*buf, ",\"tok\":\"", 8);
            unparseJSONStr(buf, tok->tok);
            WRITE_ONE_BUFFER(*buf, '"');
        }

        WRITE_BUFFER(*buf, "},", 2);
    }

    WRITE_BUFFER(*buf, "\"type\":\"", 8);
    tmp = nodeName(node->type);
    unparseJSONStr(buf, tmp);

    WRITE_BUFFER(*buf, "\",\"children\":[", 14);

    for (i = 0; node->children[i]; i++) {
        if (i != 0) WRITE_ONE_BUFFER(*buf, ',');
        unparseJSONPrime(buf, node->children[i]);
    }

    WRITE_BUFFER(*buf, "]}", 2);
}

struct Buffer_char cunparseJSON(Node *node)
{
    struct Buffer_char buf;
    INIT_BUFFER(buf);
    unparseJSONPrime(&buf, node);
    WRITE_ONE_BUFFER(buf, '\0');
    return buf;
}
