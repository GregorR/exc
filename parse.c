#include "parse.h"

BUFFER(Nodep, Node *);

static Node *newNode(Node *parent, int type, Token *tok, size_t children)
{
    Node *ret = calloc(sizeof(Node) + children * sizeof(Node *), 1);
    ret->parent = parent;
    ret->type = type;
    ret->tok = tok;
    return ret;
}

static Token *scan(ParseState *state)
{
    if (state->buf.bufused) {
        state->buf.bufused--;
        return state->buf.buf[state->buf.bufused];
    }

    return cscan(&state->scanState);
}

static void pushToken(ParseState *state, Token *tok)
{
    WRITE_ONE_BUFFER(state->buf, tok);
}

static void pushNode(ParseState *state, Node *node)
{
    ssize_t i;
    for (i = 0; node->children[i]; i++);
    for (i--; i >= 0; i--)
        pushNode(state, node->children[i]);
    if (node->tok) {
        pushToken(node->tok);
        node->tok = NULL;
    }
}

static Token *expect(ParseState *state, int type)
{
    Token *ret = scan(state);

    if (ret->type == type) return ret;

    pushToken(state, tok);
    return NULL;
}

static Node *expectN(ParseState *state, Node *parent, int type)
{
    Token *tok;

    if ((tok = expect(state, type))) {
        Node *ret = newNode(parent, NODE_TOK, tok, 0);
        if (!ret) {
            pushToken(state, tok);
            return NULL;
        }
        ret->tok = tok;
        return ret;
    }

    return NULL;
}

#define MKRETN(nodeType, chn) do { \
    ret = newNode(parent, nodeType, NULL, chn); \
    if (!ret) { \
        pushNode(node); \
        freeNode(node); \
        return NULL; \
    } \
    ret->children[0] = node; \
    node->parent = ret; \
} while (0)

#define MKRETN2(nodeType, chn) do { \
    ret = newNode(parent, nodeType, NULL, chn); \
    if (!ret) { \
        pushNode(node); \
        pushNode(node2); \
        freeNode(node); \
        freeNode(node2); \
        return NULL; \
    } \
    ret->children[0] = node; \
    ret->children[1] = node2; \
    node->parent = ret; \
    node2->parent = ret; \
} while (0)

#define MKRETT(nodeType, chn) do { \
    ret = newNode(parent, nodeType, tok, chn); \
    if (!ret) { \
        pushToken(tok); \
        return NULL; \
    } \
} while (0)

#define REQUIREP(chn, parser) do { \
if (!(ret->children[chn] = parser(state, ret))) { \
    pushNode(ret); \
    freeNode(ret); \
    return NULL; \
} \
} while(0)

#define REQUIRET(chn, toktype) do { \
if (!(ret->children[chn] = expectN(state, ret, toktype))) { \
    pushNode(ret); \
    freeNode(ret); \
    return NULL; \
} \
} while (0)

static Node *parsePostfixExpression(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;
    Token *tok;

    if ((tok = expect(state, TOK_LPAREN))) {
        /* a compound literal */
        MKRETT(NODE_COMPOUND_LITERAL, 5);
        REQUIREP(0, parseTypeName);
        REQUIRET(1, TOK_LPAREN);
        REQUIRET(2, TOK_LBRACE);
        REQUIREP(3, parseInitializerListElision);
        REQUIRET(4, TOK_RBRACE);
        node = ret;

    } else if (!(node = parsePrimaryExpression(state, parent))) {
        return NULL;

    }

    while (1) {
        if ((node2 = expectN(state, TOK_LBRACKET))) {
            MKRETN2(NODE_INDEX_EXPRESSION, 4);
            REQUIREP(2, parseExpression);
            REQUIRET(3, TOK_RBRACKET);
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, TOK_LPAREN))) {
            MKRETN2(NODE_CALL_EXPRESSION, 4);
            REQUIREP(2, parseArgumentExpressionListOpt);
            REQUIRET(3, TOK_RPAREN);
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, TOK_DOT))) {
            MKRETN2(NODE_MEMBER_DOT_EXPRESSION, 3);
            REQUIREP(2, parseIdentifier);
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, TOK_ARROW))) {
            MKRETN2(NODE_MEMBER_ARROW_EXpRESSION, 3);
            REQUIREP(2, parseIdentifier);
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, TOK_PLUSPLUS))) {
            MKRETN2(NODE_POSTINC, 2);
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, TOK_MINUSMINUS))) {
            MKRETN2(NODE_POSTDEC, 2);
            node = ret;
            continue;
        }

        break;
    }

    return node;
}

static Node *parseGenericAssociation(ParseState *state, Node *parent)
{
    Node *ret, *node;
    Token *tok;

    if ((node = parseTypeName(state, parent))) {
        MKRETN(NODE_GENERIC_ASSOCIATION, 3);
        REQUIRET(1, TOK_COLON);
        REQUIREP(2, parseAssignmentExpression);
        return ret;
    }

    if ((tok = expect(state, TOK_DEFAULT))) {
        MKRETT(NODE_GENERIC_ASSOCIATION_DEFAULT, 2);
        REQUIRET(0, TOK_COLON);
        REQUIREP(1, parseAssignmentExpression);
        return ret;
    }

    return NULL;
}

static Node *parseGenericAssocList(ParseState *state, Node *parent)
{
    Node *ret, *node;
    struct Buffer_Nodep buf;
    size_t i;

    INIT_BUFFER(buf);

    if (!(node = parseGenericAssociation(state, parent))) {
        FREE_BUFFER(buf);
        return NULL;
    }
    WRITE_ONE_BUFFER(buf, node);

    while ((node = expectN(state, parent, TOK_COMMA))) {
        WRITE_ONE_BUFFER(buf, node);

        if (!(node = parseGenericAssociation(state, parent))) {
            for (i = 0; i < buf.bufused; i++) {
                pushNode(buf.buf[i]);
                freeNode(buf.buf[i]);
            }
            FREE_BUFFER(buf);
            return NULL;
        }

        WRITE_ONE_BUFFER(buf, node);
    }

    /* OK, got the whole list */
    ret = newNode(parent, NODE_GENERIC_ASSOC_LIST, NULL, buf.bufused);
    for (i = 0; i < buf.bufused; i++) {
        ret->children[i] = buf.buf[i];
        ret->children[i].parent = ret;
    }

    return ret;
}

static Node *parseGenericSelection(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK__Generic))) {
        MKRETT(NODE_GENERIC_SELECTION, 5);
        REQUIRET(0, TOK_LPAREN);
        REQUIREP(1, parseAssignmentExpression);
        REQUIRET(2, TOK_COMMA);
        REQUIREP(3, parseGenericAssocList);
        REQUIRET(4, TOK_RPAREN);
        return ret;
    }

    return NULL;
}

static Node *parsePrimaryExpression(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((ret = parseIdentifier(state, parent))) return ret;
    if ((ret = parseConstant(state, parent))) return ret;
    if ((ret = expectN(state, parent, TOK_STR_LITERAL))) {
        ret->type = NODE_STR_LITERAL;
        return ret;
    }

    if (tok = expect(state, TOK_LPAREN)) {
        /* parenthesized expression */
        MKRETT(NODE_PAREN, 2);
        REQUIREP(0, parseExpression);
        REQUIRET(1, TOK_RPAREN);
        return ret;
    }

    if ((ret = parseGenericSelection(state, parent))) return ret;
}
