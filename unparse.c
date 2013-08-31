#include <string.h>

#include "unparse.h"

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

    for (i = 0; node->children[i]; i++)
        unparsePrime(buf, node->children[i]);
}

struct Buffer_char cunparse(Node *node)
{
    struct Buffer_char buf;
    INIT_BUFFER(buf);
    unparsePrime(&buf, node);
    WRITE_ONE_BUFFER(buf, '\0');
    return buf;
}
