#include <string.h>

#include "unparse.h"

#include "scan.h"
#include "parse.h"

/* convert a token type to a name */
static const char *tokenName(int n)
{
    switch (n) {
#define TOKEN(x) case TOK_ ## x: return #x;
#include "tokens.h"
#undef TOKEN
        default: return "???";
    }
}

/* convert a node type (numeric) to a name */
static const char *nodeName(int n)
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


static void unparseJSONPrime(struct Buffer_char *buf, Node *node)
{
    size_t i;
    const char *tmp;

    WRITE_ONE_BUFFER(*buf, '{');

    if (node->tok) {
        Token *tok = node->tok;
        WRITE_BUFFER(*buf, "\"tok\":{\"type\":\"", 15);

        tmp = tokenName(tok->type);
        WRITE_BUFFER(*buf, tmp, strlen(tmp));

        WRITE_ONE_BUFFER(*buf, '"');

        if (tok->pre) {
            WRITE_BUFFER(*buf, ",\"pre\":\"", 8);
            WRITE_BUFFER(*buf, tok->pre, strlen(tok->pre));
            WRITE_ONE_BUFFER(*buf, '"');
        }

        if (tok->tok) {
            WRITE_BUFFER(*buf, ",\"tok\":\"", 8);
            WRITE_BUFFER(*buf, tok->tok, strlen(tok->tok));
            WRITE_ONE_BUFFER(*buf, '"');
        }

        WRITE_ONE_BUFFER(*buf, ',');
    }

    WRITE_BUFFER(*buf, "\"type\":\"", 8);
    tmp = nodeName(node->type);
    WRITE_BUFFER(*buf, tmp, strlen(tmp));

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
