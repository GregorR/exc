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

#include "parse.h"

#include "unparse.h"

#include "string.h"


#line 50 "src/parse.exc"
BUFFER(Nodep, Node *);

/* scan for a token, with pushback support */
static Token *scan(ParseState *state)
{
    if (state->buf.bufused) {
        state->buf.bufused--;
        return state->buf.buf[state->buf.bufused];
    }

    return cscan(state->scanState);
}

/* push a token back */
static void pushToken(ParseState *state, Token *tok)
{
    WRITE_ONE_BUFFER(state->buf, tok);
}

/* push a whole node worth of tokens back */
static void pushNode(ParseState *state, Node *node)
{
    ssize_t i;
    for (i = 0; node->children[i]; i++);
    for (i--; i >= 0; i--)
        pushNode(state, node->children[i]);
    if (node->tok) {
        pushToken(state, node->tok);
        node->tok = NULL;
    }
}

/* expect a given token (return the token or push back and return NULL) */
static Token *expect(ParseState *state, int type)
{
    Token *ret = scan(state);

    if (ret->type == type) return ret;

    /* add it to the error state */
    if (ret->idx > state->eidx) {
        state->eidx = ret->idx;
        state->ef = ret->f;
        state->el = ret->l;
        state->ec = ret->c;
        state->eexpected.bufused = 0;
        state->efound = ret->type;
    }

    if (ret->idx == state->eidx) {
        WRITE_ONE_BUFFER(state->eexpected, type);
    }

    pushToken(state, ret);
    return NULL;
}

/* expect a given token, as a node */
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


/* make an optional parser based on a parser `name` */
/***************************************************************
 * IDENTIFIERS/CONSTANTS                                       *
 ***************************************************************/

#line 309 "src/parse.exc"
 Node *parseDecorationName(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    tok = scan(state);
    switch (tok->type) {
        case TOK_LPAREN:
        case TOK_RPAREN:
        case TOK_LBRACKET:
        case TOK_RBRACKET:
        case TOK_LBRACE:
        case TOK_RBRACE:
        case TOK_DECORATION:
        case TOK_OPEN_DECORATION:
        case TOK_CLOSE_DECORATION:
        case TOK_TERM:
            /* these tokens are NOT acceptable as decoration names */
            pushToken(state, tok);
            break;

        default:
            do { ret = newNode(parent, NODE_DECORATION_NAME, tok, 0); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
            return ret;
    }

    return NULL;
}


#line 338 "src/parse.exc"
 Node *parseIdentifier(ParseState *state, Node *parent)
{
    Node *ret = expectN(state, parent, TOK_ID);
    if (!ret) return NULL;
    ret->type = NODE_ID;
    return ret;
}

#line 345 "src/parse.exc"
 Node *parseIdentifierOpt(ParseState *state, Node *parent) { Node *ret; if ((ret = parseIdentifier(state, parent))) return ret; return newNode(parent, NODE_NIL, NULL, 0); }


#line 347 "src/parse.exc"
 Node *parseStrLiteralPart(ParseState *state, Node *parent)
{
    return expectN(state, parent, TOK_STR_LITERAL);
}


#line 352 "src/parse.exc"
 Node *parseStrLiteralL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); while ((node = parseStrLiteralPart(state, parent))) { WRITE_ONE_BUFFER(buf, node); } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_STR_LITERAL, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; buf.buf[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 352 "src/parse.exc"
 Node *parseStrLiteral(ParseState *state, Node *parent) { return parseStrLiteralL(state, parent, 1); } 
#line 352 "src/parse.exc"
 Node *parseStrLiteralOpt(ParseState *state, Node *parent) { return parseStrLiteralL(state, parent, 0); }


#line 354 "src/parse.exc"
 Node *parseConstant(ParseState *state, Node *parent)
{
    Node *ret;

    if ((ret = expectN(state, parent, TOK_INT_LITERAL))) {
        ret->type = NODE_INT_LITERAL;
        return ret;
    }

    if ((ret = expectN(state, parent, TOK_FLOAT_LITERAL))) {
        ret->type = NODE_FLOAT_LITERAL;
        return ret;
    }

    if ((ret = expectN(state, parent, TOK_CHAR_LITERAL))) {
        ret->type = NODE_CHAR_LITERAL;
        return ret;
    }

    if ((ret = parseStrLiteral(state, parent))) return ret;

    return NULL;
}


/***************************************************************
 * EXPRESSIONS                                                 *
 ***************************************************************/

#line 382 "src/parse.exc"
 Node *parsePrimaryExpression(ParseState *state, Node *parent);

#line 383 "src/parse.exc"
 Node *parseCastExpression(ParseState *state, Node *parent);

#line 384 "src/parse.exc"
 Node *parseAssignmentExpression(ParseState *state, Node *parent);

#line 385 "src/parse.exc"
 Node *parseExpression(ParseState *state, Node *parent);

#line 386 "src/parse.exc"
 Node *parseTypeName(ParseState *state, Node *parent);

#line 387 "src/parse.exc"
 Node *parseInitializerList(ParseState *state, Node *parent);

#line 388 "src/parse.exc"
 Node *parseDecorationOpExpression(ParseState *state, Node *parent);

#line 389 "src/parse.exc"
 Node *parseExpressionDecorator(ParseState *state, Node *parent);


#line 391 "src/parse.exc"
 Node *parseArgumentExpressionListL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); if ((node = parseAssignmentExpression(state, parent))) { WRITE_ONE_BUFFER(buf, node); while ((node = expectN(state, parent, TOK_COMMA))) { WRITE_ONE_BUFFER(buf, node); if (!(node = parseAssignmentExpression(state, parent))) { node = buf.buf[buf.bufused-1]; buf.bufused--; pushNode(state, node); freeNode(node); break; } WRITE_ONE_BUFFER(buf, node); } } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_ARGUMENT_EXPRESSION_LIST, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; ret->children[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 391 "src/parse.exc"
 Node *parseArgumentExpressionList(ParseState *state, Node *parent) { return parseArgumentExpressionListL(state, parent, 1); } 
#line 391 "src/parse.exc"
 Node *parseArgumentExpressionListOpt(ParseState *state, Node *parent) { return parseArgumentExpressionListL(state, parent, 0); }


#line 393 "src/parse.exc"
 Node *parsePostfixExpression(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;
    Token *tok;

    node = NULL;
    if ((tok = expect(state, TOK_LPAREN))) {
        /* a compound literal */
        do { ret = newNode(parent, NODE_COMPOUND_LITERAL, tok, 6); if (!ret) { pushToken(state, tok); return NULL; } } while (0);

        { if (!(ret->children[0] = parseTypeName(state, ret))) { { pushNode(state, ret); freeNode(ret); node = NULL; goto notcompound; } } };
        { if (!(ret->children[1] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); node = NULL; goto notcompound; } } };
        { if (!(ret->children[2] = expectN(state, ret, TOK_LBRACE))) { { pushNode(state, ret); freeNode(ret); node = NULL; goto notcompound; } } };
        { if (!(ret->children[3] = parseInitializerList(state, ret))) { { pushNode(state, ret); freeNode(ret); node = NULL; goto notcompound; } } };
        if ((ret->children[4] = expectN(state, ret, TOK_COMMA))) {
            { if (!(ret->children[5] = expectN(state, ret, TOK_RBRACE))) { { pushNode(state, ret); freeNode(ret); node = NULL; goto notcompound; } } };
        } else {
            { if (!(ret->children[4] = expectN(state, ret, TOK_RBRACE))) { { pushNode(state, ret); freeNode(ret); node = NULL; goto notcompound; } } };
        }

        node = ret;

        notcompound: (void) 0;

    }

    if (!node && !(node = parsePrimaryExpression(state, parent))) {
        return NULL;

    }

    while (1) {

        if ((node2 = expectN(state, parent, TOK_LBRACKET))) {
            do { ret = newNode(parent, NODE_INDEX, NULL, 4); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
            { if (!(ret->children[2] = parseExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } };
            { if (!(ret->children[3] = expectN(state, ret, TOK_RBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } };
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, parent, TOK_LPAREN))) {
            do { ret = newNode(parent, NODE_CALL, NULL, 4); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
            { if (!(ret->children[2] = parseArgumentExpressionListOpt(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } };
            { if (!(ret->children[3] = expectN(state, ret, TOK_RPAREN))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } };
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, parent, TOK_DOT))) {
            do { ret = newNode(parent, NODE_MEMBER_DOT, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
            { if (!(ret->children[2] = expectN(state, ret, TOK_ID))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } };
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, parent, TOK_ARROW))) {
            do { ret = newNode(parent, NODE_MEMBER_ARROW, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
            { if (!(ret->children[2] = expectN(state, ret, TOK_ID))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } };
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, parent, TOK_PLUSPLUS))) {
            do { ret = newNode(parent, NODE_POSTINC, NULL, 2); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, parent, TOK_MINUSMINUS))) {
            do { ret = newNode(parent, NODE_POSTDEC, NULL, 2); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
            node = ret;
            continue;
        }


        break;
    }

    return node;
}


#line 475 "src/parse.exc"
 Node *parseUnaryTypeExpression(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_sizeof))) {
        do { ret = newNode(parent, NODE_SIZEOF_TYPE, tok, 3); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = expectN(state, ret, TOK_LPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = parseTypeName(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    if ((tok = expect(state, TOK__Alignof))) {
        do { ret = newNode(parent, NODE_ALIGNOF, tok, 3); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = expectN(state, ret, TOK_LPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = parseTypeName(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return NULL;
}


#line 499 "src/parse.exc"
 Node *parseUnaryExpression(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((ret = parsePostfixExpression(state, parent))) return ret;
    do { if ((tok = expect(state, TOK_PLUSPLUS))) { do { ret = newNode(parent, NODE_PREINC, tok, 1); if (!ret) { pushToken(state, tok); return NULL; } } while (0); { if (!(ret->children[0] = parseUnaryExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); break; } } }; return ret; } } while (0);;
    do { if ((tok = expect(state, TOK_MINUSMINUS))) { do { ret = newNode(parent, NODE_PREDEC, tok, 1); if (!ret) { pushToken(state, tok); return NULL; } } while (0); { if (!(ret->children[0] = parseUnaryExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); break; } } }; return ret; } } while (0);;
    do { if ((tok = expect(state, TOK_AND))) { do { ret = newNode(parent, NODE_ADDROF, tok, 1); if (!ret) { pushToken(state, tok); return NULL; } } while (0); { if (!(ret->children[0] = parseUnaryExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); break; } } }; return ret; } } while (0);;
    do { if ((tok = expect(state, TOK_STAR))) { do { ret = newNode(parent, NODE_DEREF, tok, 1); if (!ret) { pushToken(state, tok); return NULL; } } while (0); { if (!(ret->children[0] = parseUnaryExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); break; } } }; return ret; } } while (0);;
    do { if ((tok = expect(state, TOK_PLUS))) { do { ret = newNode(parent, NODE_POSITIVE, tok, 1); if (!ret) { pushToken(state, tok); return NULL; } } while (0); { if (!(ret->children[0] = parseUnaryExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); break; } } }; return ret; } } while (0);;
    do { if ((tok = expect(state, TOK_MINUS))) { do { ret = newNode(parent, NODE_NEGATIVE, tok, 1); if (!ret) { pushToken(state, tok); return NULL; } } while (0); { if (!(ret->children[0] = parseUnaryExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); break; } } }; return ret; } } while (0);;
    do { if ((tok = expect(state, TOK_BNOT))) { do { ret = newNode(parent, NODE_BNOT, tok, 1); if (!ret) { pushToken(state, tok); return NULL; } } while (0); { if (!(ret->children[0] = parseUnaryExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); break; } } }; return ret; } } while (0);;
    do { if ((tok = expect(state, TOK_NOT))) { do { ret = newNode(parent, NODE_NOT, tok, 1); if (!ret) { pushToken(state, tok); return NULL; } } while (0); { if (!(ret->children[0] = parseUnaryExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); break; } } }; return ret; } } while (0);;

    if ((ret = parseUnaryTypeExpression(state, parent))) return ret;

    do { if ((tok = expect(state, TOK_sizeof))) { do { ret = newNode(parent, NODE_SIZEOF_EXP, tok, 1); if (!ret) { pushToken(state, tok); return NULL; } } while (0); { if (!(ret->children[0] = parseUnaryExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); break; } } }; return ret; } } while (0);;


    return NULL;
}


#line 530 "src/parse.exc"
 Node *parseCastTypeExpression(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_LPAREN))) {
        do { ret = newNode(parent, NODE_CAST, tok, 3); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseTypeName(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = parseCastExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return NULL;
}


#line 546 "src/parse.exc"
 Node *parseCastExpression(ParseState *state, Node *parent)
{
    Node *ret;
    if ((ret = parseCastTypeExpression(state, parent))) return ret;
    if ((ret = parseUnaryExpression(state, parent))) return ret;
    return NULL;
}

#line 573 "src/parse.exc"
 Node *parseMultiplicativeExpression(ParseState *state, Node *parent) { Node *ret, *node, *node2; if (!(node = parseDecorationOpExpression(state, parent))) return NULL; while (1) {
    if ((node2 = expectN(state, parent, TOK_STAR))) { do { ret = newNode(parent, NODE_MUL, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseDecorationOpExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
    if ((node2 = expectN(state, parent, TOK_DIV))) { do { ret = newNode(parent, NODE_DIV, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseDecorationOpExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
    if ((node2 = expectN(state, parent, TOK_MOD))) { do { ret = newNode(parent, NODE_MOD, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseDecorationOpExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
break; } return node; }


#line 579 "src/parse.exc"
 Node *parseAdditiveExpression(ParseState *state, Node *parent) { Node *ret, *node, *node2; if (!(node = parseMultiplicativeExpression(state, parent))) return NULL; while (1) {
    if ((node2 = expectN(state, parent, TOK_PLUS))) { do { ret = newNode(parent, NODE_ADD, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseMultiplicativeExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
    if ((node2 = expectN(state, parent, TOK_MINUS))) { do { ret = newNode(parent, NODE_SUB, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseMultiplicativeExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
break; } return node; }


#line 584 "src/parse.exc"
 Node *parseShiftExpression(ParseState *state, Node *parent) { Node *ret, *node, *node2; if (!(node = parseAdditiveExpression(state, parent))) return NULL; while (1) {
    if ((node2 = expectN(state, parent, TOK_SHL))) { do { ret = newNode(parent, NODE_SHL, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseAdditiveExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
    if ((node2 = expectN(state, parent, TOK_SHR))) { do { ret = newNode(parent, NODE_SHR, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseAdditiveExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
break; } return node; }


#line 589 "src/parse.exc"
 Node *parseRelationalExpression(ParseState *state, Node *parent) { Node *ret, *node, *node2; if (!(node = parseShiftExpression(state, parent))) return NULL; while (1) {
    if ((node2 = expectN(state, parent, TOK_LT))) { do { ret = newNode(parent, NODE_LT, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseShiftExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
    if ((node2 = expectN(state, parent, TOK_GT))) { do { ret = newNode(parent, NODE_GT, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseShiftExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
    if ((node2 = expectN(state, parent, TOK_LTE))) { do { ret = newNode(parent, NODE_LTE, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseShiftExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
    if ((node2 = expectN(state, parent, TOK_GTE))) { do { ret = newNode(parent, NODE_GTE, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseShiftExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
break; } return node; }


#line 596 "src/parse.exc"
 Node *parseEqualityExpression(ParseState *state, Node *parent) { Node *ret, *node, *node2; if (!(node = parseRelationalExpression(state, parent))) return NULL; while (1) {
    if ((node2 = expectN(state, parent, TOK_EQ))) { do { ret = newNode(parent, NODE_EQ, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseRelationalExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
    if ((node2 = expectN(state, parent, TOK_NEQ))) { do { ret = newNode(parent, NODE_NEQ, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseRelationalExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
break; } return node; }


#line 601 "src/parse.exc"
 Node *parseBAndExpression(ParseState *state, Node *parent) { Node *ret, *node, *node2; if (!(node = parseEqualityExpression(state, parent))) return NULL; while (1) {
    if ((node2 = expectN(state, parent, TOK_AND))) { do { ret = newNode(parent, NODE_BAND, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseEqualityExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
break; } return node; }


#line 605 "src/parse.exc"
 Node *parseBXorExpression(ParseState *state, Node *parent) { Node *ret, *node, *node2; if (!(node = parseBAndExpression(state, parent))) return NULL; while (1) {
    if ((node2 = expectN(state, parent, TOK_BXOR))) { do { ret = newNode(parent, NODE_BXOR, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseBAndExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
break; } return node; }


#line 609 "src/parse.exc"
 Node *parseBOrExpression(ParseState *state, Node *parent) { Node *ret, *node, *node2; if (!(node = parseBXorExpression(state, parent))) return NULL; while (1) {
    if ((node2 = expectN(state, parent, TOK_BOR))) { do { ret = newNode(parent, NODE_BOR, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseBXorExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
break; } return node; }


#line 613 "src/parse.exc"
 Node *parseAndExpression(ParseState *state, Node *parent) { Node *ret, *node, *node2; if (!(node = parseBOrExpression(state, parent))) return NULL; while (1) {
    if ((node2 = expectN(state, parent, TOK_ANDAND))) { do { ret = newNode(parent, NODE_AND, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseBOrExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
break; } return node; }


#line 617 "src/parse.exc"
 Node *parseOrExpression(ParseState *state, Node *parent) { Node *ret, *node, *node2; if (!(node = parseAndExpression(state, parent))) return NULL; while (1) {
    if ((node2 = expectN(state, parent, TOK_OROR))) { do { ret = newNode(parent, NODE_OR, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseAndExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; }
break; } return node; }


#line 621 "src/parse.exc"
 Node *parseConditionalExpression(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;

    if (!(node = parseOrExpression(state, parent))) return NULL;

    if ((node2 = expectN(state, parent, TOK_HOOK))) {
        do { ret = newNode(parent, NODE_CONDITIONAL, NULL, 5); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);

        { if (!(ret->children[2] = parseExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return node; } } };
        { if (!(ret->children[3] = expectN(state, ret, TOK_COLON))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return node; } } };
        { if (!(ret->children[4] = parseConditionalExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return node; } } };

        return ret;
    }

    return node;
}


#line 640 "src/parse.exc"
 Node *parseAssignmentExpression(ParseState *state, Node *parent)
{
    Node *ret, *node;
    Token *tok;

    if ((node = parseUnaryExpression(state, parent))) {
        tok = scan(state);
        if (tok->type <= TOK_ASG_START || tok->type >= TOK_ASG_END) {
            /* not an assignment expression */
            pushToken(state, tok);
            pushNode(state, node);
            freeNode(node);
            return parseConditionalExpression(state, parent);
        }

        /* an assignment */
        do { ret = newNode(parent, NODE_ASG, NULL, 3); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
        node = newNode(ret, NODE_TOK, tok, 0);
        ret->children[1] = node;
        if (tok->type > TOK_ASG)
            ret->type = NODE_RASG;
        { if (!(ret->children[2] = parseAssignmentExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return node; } } };
        return ret;
    }

    return parseConditionalExpression(state, parent);
}

#line 667 "src/parse.exc"
 Node *parseAssignmentExpressionOpt(ParseState *state, Node *parent) { Node *ret; if ((ret = parseAssignmentExpression(state, parent))) return ret; return newNode(parent, NODE_NIL, NULL, 0); }


#line 669 "src/parse.exc"
 Node *parseExpression(ParseState *state, Node *parent) { Node *ret, *node, *node2; if (!(node = parseAssignmentExpression(state, parent))) return NULL; while (1) {;
    if ((node2 = expectN(state, parent, TOK_COMMA))) { do { ret = newNode(parent, NODE_COMMA, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0); { if (!(ret->children[2] = parseAssignmentExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } }; node = ret; continue; };
break; } return node; }

#line 672 "src/parse.exc"
 Node *parseExpressionOpt(ParseState *state, Node *parent) { Node *ret; if ((ret = parseExpression(state, parent))) return ret; return newNode(parent, NODE_NIL, NULL, 0); }


#line 674 "src/parse.exc"
 Node *parseGenericAssociation(ParseState *state, Node *parent)
{
    Node *ret, *node;
    Token *tok;

    if ((node = parseTypeName(state, parent))) {
        do { ret = newNode(parent, NODE_GENERIC_ASSOCIATION, NULL, 3); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
        { if (!(ret->children[1] = expectN(state, ret, TOK_COLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = parseAssignmentExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    if ((tok = expect(state, TOK_default))) {
        do { ret = newNode(parent, NODE_GENERIC_ASSOCIATION_DEFAULT, tok, 2); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = expectN(state, ret, TOK_COLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = parseAssignmentExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return NULL;
}


#line 696 "src/parse.exc"
 Node *parseGenericAssocListL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); if ((node = parseGenericAssociation(state, parent))) { WRITE_ONE_BUFFER(buf, node); while ((node = expectN(state, parent, TOK_COMMA))) { WRITE_ONE_BUFFER(buf, node); if (!(node = parseGenericAssociation(state, parent))) { node = buf.buf[buf.bufused-1]; buf.bufused--; pushNode(state, node); freeNode(node); break; } WRITE_ONE_BUFFER(buf, node); } } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_GENERIC_ASSOC_LIST, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; ret->children[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 696 "src/parse.exc"
 Node *parseGenericAssocList(ParseState *state, Node *parent) { return parseGenericAssocListL(state, parent, 1); } 
#line 696 "src/parse.exc"
 Node *parseGenericAssocListOpt(ParseState *state, Node *parent) { return parseGenericAssocListL(state, parent, 0); }


#line 698 "src/parse.exc"
 Node *parseGenericSelection(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK__Generic))) {
        do { ret = newNode(parent, NODE_GENERIC_SELECTION, tok, 5); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = expectN(state, ret, TOK_LPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = parseAssignmentExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = expectN(state, ret, TOK_COMMA))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[3] = parseGenericAssocList(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[4] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return NULL;
}


#line 716 "src/parse.exc"
 Node *parsePrimaryExpression(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((ret = parseIdentifier(state, parent))) return ret;
    if ((ret = parseConstant(state, parent))) return ret;

    if ((tok = expect(state, TOK_LPAREN))) {
        /* parenthesized expression */
        do { ret = newNode(parent, NODE_PAREN, tok, 2); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    if ((ret = parseGenericSelection(state, parent))) return ret;
    if ((ret = parseExpressionDecorator(state, parent))) return ret;

    return NULL;
}

/***************************************************************
 * DECLARATIONS                                                *
 **************************************************************/

#line 741 "src/parse.exc"
 Node *parseDeclarator(ParseState *state, Node *parent);

#line 742 "src/parse.exc"
 Node *parseTypeQualifier(ParseState *state, Node *parent);

#line 743 "src/parse.exc"
 Node *parseAbstractDeclarator(ParseState *state, Node *parent);

#line 744 "src/parse.exc"
 Node *parsePointerOpt(ParseState *state, Node *parent);

#line 745 "src/parse.exc"
 Node *parseSpecifierQualifierList(ParseState *state, Node *parent);

#line 746 "src/parse.exc"
 Node *parseParameterTypeListOpt(ParseState *state, Node *parent);

#line 747 "src/parse.exc"
 Node *parseDeclarationSpecifiers(ParseState *state, Node *parent);

#line 748 "src/parse.exc"
 Node *parseTypeQualifierList(ParseState *state, Node *parent);

#line 749 "src/parse.exc"
 Node *parseTypeQualifierListOpt(ParseState *state, Node *parent);

#line 750 "src/parse.exc"
 Node *parseInitializer(ParseState *state, Node *parent);

#line 751 "src/parse.exc"
 Node *parseDeclarationDecoratorList(ParseState *state, Node *parent);

/* the third argument prevents multiple specifiers, particularly ID ID (since
 * this parser is typedef-ambiguous) */
static Node *parseTypeSpecifier(ParseState *state, Node *parent, int *foundSpecifier);


#line 757 "src/parse.exc"
 Node *parseStaticAssertDeclaration(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if (!(tok = expect(state, TOK__Static_assert))) return NULL;

    do { ret = newNode(parent, NODE_STATIC_ASSERT_DECLARATION, tok, 6); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
    { if (!(ret->children[0] = expectN(state, ret, TOK_LPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
    { if (!(ret->children[1] = parseConditionalExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
    { if (!(ret->children[2] = expectN(state, ret, TOK_COMMA))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
    { if (!(ret->children[3] = parseStrLiteral(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
    { if (!(ret->children[4] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
    { if (!(ret->children[5] = expectN(state, ret, TOK_SEMICOLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };

    return ret;
}


#line 775 "src/parse.exc"
 Node *parseDesignator(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_LBRACKET))) {
        do { ret = newNode(parent, NODE_DESIGNATOR, tok, 2); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseConditionalExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = expectN(state, ret, TOK_RBRACKET))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    if ((tok = expect(state, TOK_DOT))) {
        do { ret = newNode(parent, NODE_DESIGNATOR, tok, 1); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseIdentifier(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return NULL;
}


#line 796 "src/parse.exc"
 Node *parseDesignatorListL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); while ((node = parseDesignator(state, parent))) { WRITE_ONE_BUFFER(buf, node); } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_DESIGNATOR_LIST, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; buf.buf[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 796 "src/parse.exc"
 Node *parseDesignatorList(ParseState *state, Node *parent) { return parseDesignatorListL(state, parent, 1); } 
#line 796 "src/parse.exc"
 Node *parseDesignatorListOpt(ParseState *state, Node *parent) { return parseDesignatorListL(state, parent, 0); }


#line 798 "src/parse.exc"
 Node *parseDesignation(ParseState *state, Node *parent)
{
    Node *ret, *node;

    if (!(node = parseDesignatorList(state, parent))) return NULL;

    do { ret = newNode(parent, NODE_DESIGNATION, NULL, 2); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = expectN(state, ret, TOK_ASG))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
    return ret;
}

#line 808 "src/parse.exc"
 Node *parseDesignationOpt(ParseState *state, Node *parent) { Node *ret; if ((ret = parseDesignation(state, parent))) return ret; return newNode(parent, NODE_NIL, NULL, 0); }


#line 810 "src/parse.exc"
 Node *parseDesignationInitializer(ParseState *state, Node *parent)
{
    Node *ret, *node;

    if (!(node = parseDesignationOpt(state, parent))) return NULL;

    do { ret = newNode(parent, NODE_DESIGNATION_INITIALIZER, NULL, 2); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = parseInitializer(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };

    return ret;
}


#line 822 "src/parse.exc"
 Node *parseInitializerListL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); if ((node = parseDesignationInitializer(state, parent))) { WRITE_ONE_BUFFER(buf, node); while ((node = expectN(state, parent, TOK_COMMA))) { WRITE_ONE_BUFFER(buf, node); if (!(node = parseDesignationInitializer(state, parent))) { node = buf.buf[buf.bufused-1]; buf.bufused--; pushNode(state, node); freeNode(node); break; } WRITE_ONE_BUFFER(buf, node); } } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_INITIALIZER_LIST, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; ret->children[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 822 "src/parse.exc"
 Node *parseInitializerList(ParseState *state, Node *parent) { return parseInitializerListL(state, parent, 1); } 
#line 822 "src/parse.exc"
 Node *parseInitializerListOpt(ParseState *state, Node *parent) { return parseInitializerListL(state, parent, 0); }


#line 824 "src/parse.exc"
 Node *parseInitializer(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((ret = parseAssignmentExpression(state, parent))) return ret;

    if ((tok = expect(state, TOK_LBRACE))) {
        do { ret = newNode(parent, NODE_INITIALIZER, tok, 3); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseInitializerList(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };

        /* ending , is optional */
        if ((ret->children[1] = expectN(state, ret, TOK_COMMA))) {
            { if (!(ret->children[2] = expectN(state, ret, TOK_RBRACE))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        } else {
            { if (!(ret->children[1] = expectN(state, ret, TOK_RBRACE))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        }

        return ret;
    }

    return NULL;
}


static Node *parseDirectAbstractDeclaratorA1(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    do { ret = newNode(parent, NODE_DIRECT_ABSTRACT_DECLARATOR, NULL, 5); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = expectN(state, ret, TOK_LBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[2] = parseTypeQualifierListOpt(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[3] = parseAssignmentExpressionOpt(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[4] = expectN(state, ret, TOK_RBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    return ret;
}

static Node *parseDirectAbstractDeclaratorA2(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    do { ret = newNode(parent, NODE_DIRECT_ABSTRACT_DECLARATOR, NULL, 6); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = expectN(state, ret, TOK_LBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[2] = expectN(state, ret, TOK_static))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[3] = parseTypeQualifierListOpt(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[4] = parseAssignmentExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[5] = expectN(state, ret, TOK_RBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    return ret;
}

static Node *parseDirectAbstractDeclaratorA3(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    do { ret = newNode(parent, NODE_DIRECT_ABSTRACT_DECLARATOR, NULL, 6); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = expectN(state, ret, TOK_LBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[2] = parseTypeQualifierList(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[3] = expectN(state, ret, TOK_static))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[4] = parseAssignmentExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[5] = expectN(state, ret, TOK_RBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    return ret;
}

static Node *parseDirectAbstractDeclaratorA4(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    do { ret = newNode(parent, NODE_DIRECT_ABSTRACT_DECLARATOR, NULL, 4); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = expectN(state, ret, TOK_LBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[2] = expectN(state, ret, TOK_STAR))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[3] = expectN(state, ret, TOK_RBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    return ret;
}

static Node *parseDirectAbstractDeclaratorF(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    do { ret = newNode(parent, NODE_DIRECT_ABSTRACT_DECLARATOR, NULL, 4); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = expectN(state, ret, TOK_LPAREN))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[2] = parseParameterTypeListOpt(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[3] = expectN(state, ret, TOK_RPAREN))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    return ret;
}



#line 905 "src/parse.exc"
 Node *parseDirectAbstractDeclarator(ParseState *state, Node *parent)
{
    Node *ret, *node;
    Token *tok;

    node = NULL;

    if ((tok = expect(state, TOK_LPAREN))) {
        do { ret = newNode(parent, NODE_DIRECT_ABSTRACT_DECLARATOR, tok, 2); if (!ret) { pushToken(state, tok); return NULL; } } while (0);

        { if (!(ret->children[0] = parseAbstractDeclarator(state, ret))) { { pushNode(state, ret); freeNode(ret); goto pdadrestore; } } };
        { if (!(ret->children[1] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); goto pdadrestore; } } };

        node = ret;

    }

    if (0) {
pdadrestore:
        node = NULL;
    }

    if (!node)
        node = newNode(parent, NODE_DIRECT_ABSTRACT_DECLARATOR, NULL, 0);

    while (1) {
        if ((ret = parseDirectAbstractDeclaratorA1(state, parent, node))) {
            node = ret;
            continue;
        }

        if ((ret = parseDirectAbstractDeclaratorA2(state, parent, node))) {
            node = ret;
            continue;
        }

        if ((ret = parseDirectAbstractDeclaratorA3(state, parent, node))) {
            node = ret;
            continue;
        }

        if ((ret = parseDirectAbstractDeclaratorA4(state, parent, node))) {
            node = ret;
            continue;
        }

        if ((ret = parseDirectAbstractDeclaratorF(state, parent, node))) {
            node = ret;
            continue;
        }

        break;
    }

    return node;
}

#line 961 "src/parse.exc"
 Node *parseAbstractDeclaratorOpt(ParseState *state, Node *parent) { Node *ret; if ((ret = parseAbstractDeclarator(state, parent))) return ret; return newNode(parent, NODE_NIL, NULL, 0); }


#line 963 "src/parse.exc"
 Node *parseAbstractDeclarator(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;

    if (!(node = parsePointerOpt(state, parent))) return NULL;

    if (!(node2 = parseDirectAbstractDeclarator(state, parent))) {
        /* if the direct-abstract-declarator is not present, the pointer is mandatory */
        if (!node->children[0]) {
            pushNode(state, node);
            freeNode(node);
            return NULL;
        }
        return node;
    }

    do { ret = newNode(parent, NODE_ABSTRACT_DECLARATOR, NULL, 2); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
    return ret;
}


#line 983 "src/parse.exc"
 Node *parseTypeName(ParseState *state, Node *parent)
{
    Node *ret, *node;

    if (!(node = parseSpecifierQualifierList(state, parent))) return NULL;

    do { ret = newNode(parent, NODE_TYPE_NAME, NULL, 2); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = parseAbstractDeclaratorOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
    return ret;
}


#line 994 "src/parse.exc"
 Node *parseIdentifierListL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); if ((node = parseIdentifier(state, parent))) { WRITE_ONE_BUFFER(buf, node); while ((node = expectN(state, parent, TOK_COMMA))) { WRITE_ONE_BUFFER(buf, node); if (!(node = parseIdentifier(state, parent))) { node = buf.buf[buf.bufused-1]; buf.bufused--; pushNode(state, node); freeNode(node); break; } WRITE_ONE_BUFFER(buf, node); } } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_IDENTIFIER_LIST, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; ret->children[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 994 "src/parse.exc"
 Node *parseIdentifierList(ParseState *state, Node *parent) { return parseIdentifierListL(state, parent, 1); } 
#line 994 "src/parse.exc"
 Node *parseIdentifierListOpt(ParseState *state, Node *parent) { return parseIdentifierListL(state, parent, 0); }


#line 996 "src/parse.exc"
 Node *parseParameterDeclaration(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;

    if (!(node = parseDeclarationSpecifiers(state, parent))) return NULL;

    if ((node2 = parseDeclarator(state, parent))) {
        do { ret = newNode(parent, NODE_PARAMETER_DECLARATION, NULL, 2); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
        return ret;

    } else if ((node2 = parseAbstractDeclaratorOpt(state, parent))) {
        do { ret = newNode(parent, NODE_PARAMETER_DECLARATION, NULL, 2); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
        return ret;

    } else {
        pushNode(state, node);
        freeNode(node);

        return NULL;

    }
}


#line 1019 "src/parse.exc"
 Node *parseParameterListL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); if ((node = parseParameterDeclaration(state, parent))) { WRITE_ONE_BUFFER(buf, node); while ((node = expectN(state, parent, TOK_COMMA))) { WRITE_ONE_BUFFER(buf, node); if (!(node = parseParameterDeclaration(state, parent))) { node = buf.buf[buf.bufused-1]; buf.bufused--; pushNode(state, node); freeNode(node); break; } WRITE_ONE_BUFFER(buf, node); } } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_PARAMETER_LIST, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; ret->children[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 1019 "src/parse.exc"
 Node *parseParameterList(ParseState *state, Node *parent) { return parseParameterListL(state, parent, 1); } 
#line 1019 "src/parse.exc"
 Node *parseParameterListOpt(ParseState *state, Node *parent) { return parseParameterListL(state, parent, 0); }


#line 1021 "src/parse.exc"
 Node *parseParameterTypeList(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;

    if (!(node = parseParameterList(state, parent))) return NULL;

    if ((node2 = expectN(state, parent, TOK_COMMA))) {
        do { ret = newNode(parent, NODE_PARAMETER_TYPE_LIST, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
        { if (!(ret->children[2] = expectN(state, ret, TOK_DOTDOTDOT))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return node; } } };
        return ret;
    }

    return node;
}

#line 1035 "src/parse.exc"
 Node *parseParameterTypeListOpt(ParseState *state, Node *parent) { Node *ret; if ((ret = parseParameterTypeList(state, parent))) return ret; return newNode(parent, NODE_NIL, NULL, 0); }


#line 1037 "src/parse.exc"
 Node *parseTypeQualifierListL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); while ((node = parseTypeQualifier(state, parent))) { WRITE_ONE_BUFFER(buf, node); } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_TYPE_QUALIFIER_LIST, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; buf.buf[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 1037 "src/parse.exc"
 Node *parseTypeQualifierList(ParseState *state, Node *parent) { return parseTypeQualifierListL(state, parent, 1); } 
#line 1037 "src/parse.exc"
 Node *parseTypeQualifierListOpt(ParseState *state, Node *parent) { return parseTypeQualifierListL(state, parent, 0); }


#line 1039 "src/parse.exc"
 Node *parsePointerOpt(ParseState *state, Node *parent)
{
    Node *ret, *node;
    struct Buffer_Nodep buf;
    size_t i;

    INIT_BUFFER(buf);

    while ((node = expectN(state, parent, TOK_STAR))) {
        WRITE_ONE_BUFFER(buf, node);

        if (!(node = parseTypeQualifierListOpt(state, parent))) {
            node = buf.buf[--buf.bufused];
            pushNode(state, node);
            freeNode(node);
            break;
        }

        WRITE_ONE_BUFFER(buf, node);
    }

    ret = newNode(parent, NODE_POINTER, NULL, buf.bufused);
    for (i = 0; i < buf.bufused; i++) {
        ret->children[i] = buf.buf[i];
        buf.buf[i]->parent = ret;
    }

    FREE_BUFFER(buf);

    return ret;
}


static Node *parseDirectDeclaratorA1(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    do { ret = newNode(parent, NODE_DIRECT_DECLARATOR, NULL, 5); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = expectN(state, ret, TOK_LBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[2] = parseTypeQualifierListOpt(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[3] = parseAssignmentExpressionOpt(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[4] = expectN(state, ret, TOK_RBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    return ret;
}

static Node *parseDirectDeclaratorA2(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    do { ret = newNode(parent, NODE_DIRECT_DECLARATOR, NULL, 6); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = expectN(state, ret, TOK_LBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[2] = expectN(state, ret, TOK_static))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[3] = parseTypeQualifierListOpt(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[4] = parseAssignmentExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[5] = expectN(state, ret, TOK_RBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    return ret;
}

static Node *parseDirectDeclaratorA3(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    do { ret = newNode(parent, NODE_DIRECT_DECLARATOR, NULL, 6); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = expectN(state, ret, TOK_LBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[2] = parseTypeQualifierList(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[3] = expectN(state, ret, TOK_static))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[4] = parseAssignmentExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[5] = expectN(state, ret, TOK_RBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    return ret;
}

static Node *parseDirectDeclaratorA4(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    do { ret = newNode(parent, NODE_DIRECT_DECLARATOR, NULL, 5); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = expectN(state, ret, TOK_LBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[2] = parseTypeQualifierListOpt(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[3] = expectN(state, ret, TOK_STAR))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[4] = expectN(state, ret, TOK_RBRACKET))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    return ret;
}

static Node *parseDirectDeclaratorF1(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    do { ret = newNode(parent, NODE_DIRECT_DECLARATOR, NULL, 4); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = expectN(state, ret, TOK_LPAREN))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[2] = parseParameterTypeList(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[3] = expectN(state, ret, TOK_RPAREN))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    return ret;
}

static Node *parseDirectDeclaratorF2(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    do { ret = newNode(parent, NODE_DIRECT_DECLARATOR, NULL, 4); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = expectN(state, ret, TOK_LPAREN))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[2] = parseIdentifierListOpt(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    { if (!(ret->children[3] = expectN(state, ret, TOK_RPAREN))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return NULL; } } };
    return ret;
}



#line 1139 "src/parse.exc"
 Node *parseDirectDeclarator(ParseState *state, Node *parent)
{
    Node *ret, *node;
    Token *tok;

    if ((tok = expect(state, TOK_ID))) {
        do { ret = newNode(parent, NODE_DIRECT_DECLARATOR, tok, 0); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        node = ret;

    } else if ((tok = expect(state, TOK_LPAREN))) {
        do { ret = newNode(parent, NODE_DIRECT_DECLARATOR, tok, 2); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseDeclarator(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        node = ret;

    } else {
        return NULL;

    }

    while (1) {
        if ((ret = parseDirectDeclaratorA1(state, parent, node))) {
            node = ret;
            continue;
        }

        if ((ret = parseDirectDeclaratorA2(state, parent, node))) {
            node = ret;
            continue;
        }

        if ((ret = parseDirectDeclaratorA3(state, parent, node))) {
            node = ret;
            continue;
        }

        if ((ret = parseDirectDeclaratorA4(state, parent, node))) {
            node = ret;
            continue;
        }

        if ((ret = parseDirectDeclaratorF1(state, parent, node))) {
            node = ret;
            continue;
        }

        if ((ret = parseDirectDeclaratorF2(state, parent, node))) {
            node = ret;
            continue;
        }

        break;
    }

    return node;
}


#line 1196 "src/parse.exc"
 Node *parseDeclarator(ParseState *state, Node *parent)
{
    Node *ret, *node;

    if (!(node = parsePointerOpt(state, parent))) return NULL;

    do { ret = newNode(parent, NODE_DECLARATOR, NULL, 2); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = parseDirectDeclarator(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };

    return ret;
}


#line 1208 "src/parse.exc"
 Node *parseAlignmentSpecifier(ParseState *state, Node *parent)
{
    Node *ret, *node;
    Token *tok;

    if (!(tok = expect(state, TOK__Alignas))) return NULL;

    do { ret = newNode(parent, NODE_ALIGNMENT_SPECIFIER, tok, 3); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
    { if (!(ret->children[0] = expectN(state, ret, TOK_LPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };

    if (!(node = parseTypeName(state, ret)) &&
        !(node = parseConditionalExpression(state, ret))) {
        pushNode(state, ret);
        freeNode(ret);
        return NULL;
    }

    ret->children[1] = node;
    { if (!(ret->children[2] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };

    return ret;
}


#line 1231 "src/parse.exc"
 Node *parseFunctionSpecifier(ParseState *state, Node *parent)
{
    Node *ret;

    if ((ret = expectN(state, parent, TOK_inline)) ||
        (ret = expectN(state, parent, TOK__Noreturn))) {
        ret->type = NODE_FUNCTION_SPECIFIER;
        return ret;
    }

    return NULL;
}


#line 1244 "src/parse.exc"
 Node *parseTypeQualifier(ParseState *state, Node *parent)
{
    Node *ret;

    if ((ret = expectN(state, parent, TOK_const)) ||
        (ret = expectN(state, parent, TOK_restrict)) ||
        (ret = expectN(state, parent, TOK_volatile)) ||
        (ret = expectN(state, parent, TOK__Atomic))) {
        ret->type = NODE_TYPE_QUALIFIER;
        return ret;
    }

    return NULL;
}


#line 1259 "src/parse.exc"
 Node *parseAtomicTypeSpecifier(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if (!(tok = expect(state, TOK__Atomic))) return NULL;

    do { ret = newNode(parent, NODE_ATOMIC_TYPE_SPECIFIER, tok, 3); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
    { if (!(ret->children[0] = expectN(state, ret, TOK_LPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
    { if (!(ret->children[1] = parseTypeName(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
    { if (!(ret->children[2] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };

    return ret;
}


#line 1274 "src/parse.exc"
 Node *parseEnumerator(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;

    if (!(node = parseIdentifier(state, parent))) return NULL;

    if ((node2 = expectN(state, parent, TOK_ASG))) {
        do { ret = newNode(parent, NODE_ENUMERATOR, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
        { if (!(ret->children[2] = parseConditionalExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return node; } } };
        return ret;
    }

    return node;
}


#line 1289 "src/parse.exc"
 Node *parseEnumeratorListL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); if ((node = parseEnumerator(state, parent))) { WRITE_ONE_BUFFER(buf, node); while ((node = expectN(state, parent, TOK_COMMA))) { WRITE_ONE_BUFFER(buf, node); if (!(node = parseEnumerator(state, parent))) { node = buf.buf[buf.bufused-1]; buf.bufused--; pushNode(state, node); freeNode(node); break; } WRITE_ONE_BUFFER(buf, node); } } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_ENUMERATOR_LIST, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; ret->children[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 1289 "src/parse.exc"
 Node *parseEnumeratorList(ParseState *state, Node *parent) { return parseEnumeratorListL(state, parent, 1); } 
#line 1289 "src/parse.exc"
 Node *parseEnumeratorListOpt(ParseState *state, Node *parent) { return parseEnumeratorListL(state, parent, 0); }


#line 1291 "src/parse.exc"
 Node *parseEnumSpecifier(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_enum))) {
        do { ret = newNode(parent, NODE_ENUM_SPECIFIER, tok, 5); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseIdentifierOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };

        if (ret->children[0]->tok) {
            /* the rest is optional */
            ret->children[1] = expectN(state, ret, TOK_LBRACE);

        } else {
            /* the rest is mandatory */
            { if (!(ret->children[1] = expectN(state, ret, TOK_LBRACE))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };

        }

        if (ret->children[1]) {
            { if (!(ret->children[2] = parseEnumeratorList(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
            if ((ret->children[3] = expectN(state, ret, TOK_COMMA))) {
                { if (!(ret->children[4] = expectN(state, ret, TOK_RBRACE))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
            } else {
                { if (!(ret->children[3] = expectN(state, ret, TOK_RBRACE))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
            }
        }

        return ret;
    }

    return NULL;
}


#line 1325 "src/parse.exc"
 Node *parseStructDeclarator(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;
    Token *tok;

    if ((node = parseDeclarator(state, parent))) {
        if ((node2 = expectN(state, parent, TOK_COLON))) {
            do { ret = newNode(parent, NODE_BITFIELD_DECLARATOR, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
            { if (!(ret->children[2] = parseConstant(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return node; } } };
            return ret;
        }
        return node;

    } else if ((tok = expect(state, TOK_COLON))) {
        do { ret = newNode(parent, NODE_BITFIELD_PADDING, tok, 1); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseConstant(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;

    }

    return NULL;
}


#line 1348 "src/parse.exc"
 Node *parseStructDeclaratorListL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); if ((node = parseStructDeclarator(state, parent))) { WRITE_ONE_BUFFER(buf, node); while ((node = expectN(state, parent, TOK_COMMA))) { WRITE_ONE_BUFFER(buf, node); if (!(node = parseStructDeclarator(state, parent))) { node = buf.buf[buf.bufused-1]; buf.bufused--; pushNode(state, node); freeNode(node); break; } WRITE_ONE_BUFFER(buf, node); } } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_STRUCT_DECLARATOR_LIST, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; ret->children[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 1348 "src/parse.exc"
 Node *parseStructDeclaratorList(ParseState *state, Node *parent) { return parseStructDeclaratorListL(state, parent, 1); } 
#line 1348 "src/parse.exc"
 Node *parseStructDeclaratorListOpt(ParseState *state, Node *parent) { return parseStructDeclaratorListL(state, parent, 0); }

static Node *parseSpecifierQualifierListL(ParseState *state, Node *parent, int need1)
{
    Node *ret;
    struct Buffer_Nodep buf;
    size_t i;
    int foundSpecifier = 0;

    INIT_BUFFER(buf);

    while ((ret = parseTypeSpecifier(state, parent, &foundSpecifier)) ||
           (ret = parseTypeQualifier(state, parent))) {
        WRITE_ONE_BUFFER(buf, ret);
    }

    if (need1 && buf.bufused == 0) {
        FREE_BUFFER(buf);
        return NULL;
    }

    ret = newNode(parent, NODE_SPECIFIER_QUALIFIER_LIST, NULL, buf.bufused);
    for (i = 0; i < buf.bufused; i++) {
        ret->children[i] = buf.buf[i];
        buf.buf[i]->parent = ret;
    }

    /* and finally, the whole thing can have a decoration */
    {
        Node *node, *node2;
        if ((node2 = parseDeclarationDecoratorList(state, parent))) {
            node = ret;
            do { ret = newNode(parent, NODE_DECORATED_SPECIFIER_QUALIFIER_LIST, NULL, 2); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
        }
    }

    FREE_BUFFER(buf);
    return ret;
}

#line 1387 "src/parse.exc"
 Node *parseSpecifierQualifierList(ParseState *state, Node *parent) { return parseSpecifierQualifierListL(state, parent, 1); } 
#line 1387 "src/parse.exc"
 Node *parseSpecifierQualifierListOpt(ParseState *state, Node *parent) { return parseSpecifierQualifierListL(state, parent, 0); }


#line 1389 "src/parse.exc"
 Node *parseStructDeclaration(ParseState *state, Node *parent)
{
    Node *ret, *node;

    if ((node = parseSpecifierQualifierList(state, parent))) {
        do { ret = newNode(parent, NODE_STRUCT_DECLARATION, NULL, 3); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
        { if (!(ret->children[1] = parseStructDeclaratorListOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = expectN(state, ret, TOK_SEMICOLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    if ((ret = parseStaticAssertDeclaration(state, parent))) return ret;

    return NULL;
}


#line 1405 "src/parse.exc"
 Node *parseStructDeclarationListL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); while ((node = parseStructDeclaration(state, parent))) { WRITE_ONE_BUFFER(buf, node); } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_STRUCT_DECLARATION_LIST, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; buf.buf[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 1405 "src/parse.exc"
 Node *parseStructDeclarationList(ParseState *state, Node *parent) { return parseStructDeclarationListL(state, parent, 1); } 
#line 1405 "src/parse.exc"
 Node *parseStructDeclarationListOpt(ParseState *state, Node *parent) { return parseStructDeclarationListL(state, parent, 0); }


#line 1407 "src/parse.exc"
 Node *parseStructOrUnionSpecifier(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_struct)) ||
        (tok = expect(state, TOK_union))) {
        do { ret = newNode(parent, NODE_STRUCT_OR_UNION_SPECIFIER, tok, 4); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseIdentifierOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        if (ret->children[0]->tok) {
            /* the rest is optional */
            ret->children[1] = expectN(state, ret, TOK_LBRACE);

        } else {
            /* the rest is mandatory */
            { if (!(ret->children[1] = expectN(state, ret, TOK_LBRACE))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };

        }

        if (ret->children[1]) {
            { if (!(ret->children[2] = parseStructDeclarationList(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
            { if (!(ret->children[3] = expectN(state, ret, TOK_RBRACE))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        }

        return ret;
    }

    return NULL;
}

static Node *parseTypeSpecifier(ParseState *state, Node *parent, int *foundSpecifier)
{
    Node *ret;

    if ((ret = expectN(state, parent, TOK_void)) ||
        (ret = expectN(state, parent, TOK_char)) ||
        (ret = expectN(state, parent, TOK_short)) ||
        (ret = expectN(state, parent, TOK_int)) ||
        (ret = expectN(state, parent, TOK_long)) ||
        (ret = expectN(state, parent, TOK_float)) ||
        (ret = expectN(state, parent, TOK_double)) ||
        (ret = expectN(state, parent, TOK_signed)) ||
        (ret = expectN(state, parent, TOK_unsigned)) ||
        (ret = expectN(state, parent, TOK__Bool)) ||
        (ret = expectN(state, parent, TOK__Complex))) {
        *foundSpecifier = 1;
        ret->type = NODE_TYPE_SPECIFIER;
        return ret;
    }

    /* ID's are included as typedef-name */
    if (!*foundSpecifier && (ret = expectN(state, parent, TOK_ID))) {
        *foundSpecifier = 1;
        ret->type = NODE_TYPE_SPECIFIER;
        return ret;
    }

    if ((ret = parseAtomicTypeSpecifier(state, parent))) {
        *foundSpecifier = 1;
        return ret;
    }
    if ((ret = parseStructOrUnionSpecifier(state, parent))) {
        *foundSpecifier = 1;
        return ret;
    }
    if ((ret = parseEnumSpecifier(state, parent))) {
        *foundSpecifier = 1;
        return ret;
    }

    return NULL;
}


#line 1480 "src/parse.exc"
 Node *parseStorageClassSpecifier(ParseState *state, Node *parent)
{
    Node *ret;
    if ((ret = expectN(state, parent, TOK_typedef)) ||
        (ret = expectN(state, parent, TOK_extern)) ||
        (ret = expectN(state, parent, TOK_static)) ||
        (ret = expectN(state, parent, TOK__Thread_local)) ||
        (ret = expectN(state, parent, TOK_auto)) ||
        (ret = expectN(state, parent, TOK_register))) {
        ret->type = NODE_STORAGE_CLASS_SPECIFIER;
        return ret;
    }
    return NULL;
}


#line 1495 "src/parse.exc"
 Node *parseInitDeclarator(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;

    if (!(node = parseDeclarator(state, parent))) return NULL;

    if ((node2 = expectN(state, parent, TOK_ASG))) {
        do { ret = newNode(parent, NODE_INIT_DECLARATOR, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
        { if (!(ret->children[2] = parseInitializer(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); return node; } } };
        return ret;
    }

    return node;
}


#line 1510 "src/parse.exc"
 Node *parseInitDeclaratorListL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); if ((node = parseInitDeclarator(state, parent))) { WRITE_ONE_BUFFER(buf, node); while ((node = expectN(state, parent, TOK_COMMA))) { WRITE_ONE_BUFFER(buf, node); if (!(node = parseInitDeclarator(state, parent))) { node = buf.buf[buf.bufused-1]; buf.bufused--; pushNode(state, node); freeNode(node); break; } WRITE_ONE_BUFFER(buf, node); } } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_INIT_DECLARATOR_LIST, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; ret->children[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 1510 "src/parse.exc"
 Node *parseInitDeclaratorList(ParseState *state, Node *parent) { return parseInitDeclaratorListL(state, parent, 1); } 
#line 1510 "src/parse.exc"
 Node *parseInitDeclaratorListOpt(ParseState *state, Node *parent) { return parseInitDeclaratorListL(state, parent, 0); }


#line 1512 "src/parse.exc"
 Node *parseDeclarationSpecifiers(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;
    struct Buffer_Nodep buf;
    size_t i;
    int foundSpecifier = 0;

    INIT_BUFFER(buf);

    while (1) {
        if ((node = parseStorageClassSpecifier(state, parent))) {
            WRITE_ONE_BUFFER(buf, node);
            continue;
        }

        if ((node = parseTypeSpecifier(state, parent, &foundSpecifier))) {
            WRITE_ONE_BUFFER(buf, node);
            continue;
        }

        if ((node = parseTypeQualifier(state, parent))) {
            WRITE_ONE_BUFFER(buf, node);
            continue;
        }

        if ((node = parseFunctionSpecifier(state, parent))) {
            WRITE_ONE_BUFFER(buf, node);
            continue;
        }

        if ((node = parseAlignmentSpecifier(state, parent))) {
            WRITE_ONE_BUFFER(buf, node);
            continue;
        }

        break;
    }

    if (buf.bufused == 0) {
        FREE_BUFFER(buf);
        return NULL;
    }

    ret = newNode(parent, NODE_DECLARATION_SPECIFIERS, NULL, buf.bufused);
    if (!ret) {
        for (i = 0; i < buf.bufused; i++) {
            pushNode(state, buf.buf[i]);
            freeNode(buf.buf[i]);
        }
        FREE_BUFFER(buf);
        return NULL;
    }

    for (i = 0; i < buf.bufused; i++) {
        ret->children[i] = buf.buf[i];
        buf.buf[i]->parent = ret;
    }

    /* and the whole thing can have a type decoration */
    if ((node2 = parseDeclarationDecoratorList(state, parent))) {
        node = ret;
        do { ret = newNode(parent, NODE_DECORATED_DECLARATION_SPECIFIERS, NULL, 2); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
    }

    FREE_BUFFER(buf);
    return ret;
}


#line 1580 "src/parse.exc"
 Node *parseDeclaration(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;

    if ((node = parseDeclarationDecoratorList(state, parent))) {
        if ((node2 = parseDeclaration(state, parent))) {
            do { ret = newNode(parent, NODE_DECORATED_DECLARATION, NULL, 2); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
            return ret;
        } else if ((node2 = expectN(state, parent, TOK_SEMICOLON))) {
            do { ret = newNode(parent, NODE_DECORATION_DECLARATION, NULL, 2); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
            return ret;
        }

        pushNode(state, node);
        freeNode(node);
        return NULL;
    }

    if ((node = parseDeclarationSpecifiers(state, parent))) {
        do { ret = newNode(parent, NODE_DECLARATION, NULL, 3); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
        { if (!(ret->children[1] = parseInitDeclaratorListOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = expectN(state, ret, TOK_SEMICOLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return parseStaticAssertDeclaration(state, parent);
}

/***************************************************************
 * STATEMENTS                                                  *
 ***************************************************************/

#line 1611 "src/parse.exc"
 Node *parseStatement(ParseState *state, Node *parent);


#line 1613 "src/parse.exc"
 Node *parseJumpStatement(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_goto))) {
        do { ret = newNode(parent, NODE_GOTO_STATEMENT, tok, 2); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseIdentifier(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = expectN(state, ret, TOK_SEMICOLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    if ((tok = expect(state, TOK_continue))) {
        do { ret = newNode(parent, NODE_CONTINUE_STATEMENT, tok, 1); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = expectN(state, ret, TOK_SEMICOLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    if ((tok = expect(state, TOK_break))) {
        do { ret = newNode(parent, NODE_BREAK_STATEMENT, tok, 1); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = expectN(state, ret, TOK_SEMICOLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    if ((tok = expect(state, TOK_return))) {
        do { ret = newNode(parent, NODE_RETURN_STATEMENT, tok, 2); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseExpressionOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = expectN(state, ret, TOK_SEMICOLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return NULL;
}


#line 1647 "src/parse.exc"
 Node *parseForInitializer(ParseState *state, Node *parent)
{
    Node *ret, *node;

    /* declaration | expressionopt ; */
    if ((ret = parseDeclaration(state, parent))) return ret;

    if ((node = parseExpressionOpt(state, parent))) {
        do { ret = newNode(parent, NODE_FOR_INITIALIZER, NULL, 2); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
        { if (!(ret->children[1] = expectN(state, ret, TOK_SEMICOLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return NULL;
}


#line 1663 "src/parse.exc"
 Node *parseIterationStatement(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_while))) {
        do { ret = newNode(parent, NODE_WHILE_STATEMENT, tok, 4); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = expectN(state, ret, TOK_LPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = parseExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[3] = parseStatement(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    if ((tok = expect(state, TOK_do))) {
        do { ret = newNode(parent, NODE_DO_WHILE_STATEMENT, tok, 6); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseStatement(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = expectN(state, ret, TOK_while))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = expectN(state, ret, TOK_LPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[3] = parseExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[4] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[5] = expectN(state, ret, TOK_SEMICOLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    if ((tok = expect(state, TOK_for))) {
        do { ret = newNode(parent, NODE_FOR_STATEMENT, tok, 7); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = expectN(state, ret, TOK_LPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = parseForInitializer(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = parseExpressionOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[3] = expectN(state, ret, TOK_SEMICOLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[4] = parseExpressionOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[5] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[6] = parseStatement(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return NULL;
}


#line 1703 "src/parse.exc"
 Node *parseSelectionStatement(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_if))) {
        do { ret = newNode(parent, NODE_IF_STATEMENT, tok, 6); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = expectN(state, ret, TOK_LPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = parseExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[3] = parseStatement(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };

        /* optional else clause */
        if ((ret->children[4] = expectN(state, ret, TOK_else)))
            { if (!(ret->children[5] = parseStatement(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };

        return ret;
    }

    if ((tok = expect(state, TOK_switch))) {
        do { ret = newNode(parent, NODE_SWITCH_STATEMENT, tok, 4); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = expectN(state, ret, TOK_LPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = parseExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[3] = parseStatement(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return NULL;
}


#line 1734 "src/parse.exc"
 Node *parseExpressionStatement(ParseState *state, Node *parent)
{
    Node *ret, *node;

    if ((node = parseExpressionOpt(state, parent))) {
        do { ret = newNode(parent, NODE_EXPRESSION_STATEMENT, NULL, 2); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
        { if (!(ret->children[1] = expectN(state, ret, TOK_SEMICOLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return NULL;
}


#line 1747 "src/parse.exc"
 Node *parseBlockItem(ParseState *state, Node *parent)
{
    Node *ret;
    if ((ret = parseDeclaration(state, parent))) return ret;
    if ((ret = parseStatement(state, parent))) return ret;
    return NULL;
}


#line 1755 "src/parse.exc"
 Node *parseBlockItemListL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); while ((node = parseBlockItem(state, parent))) { WRITE_ONE_BUFFER(buf, node); } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_BLOCK_ITEM_LIST, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; buf.buf[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 1755 "src/parse.exc"
 Node *parseBlockItemList(ParseState *state, Node *parent) { return parseBlockItemListL(state, parent, 1); } 
#line 1755 "src/parse.exc"
 Node *parseBlockItemListOpt(ParseState *state, Node *parent) { return parseBlockItemListL(state, parent, 0); }


#line 1757 "src/parse.exc"
 Node *parseCompoundStatement(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_LBRACE))) {
        do { ret = newNode(parent, NODE_BLOCK, tok, 2); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseBlockItemListOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = expectN(state, ret, TOK_RBRACE))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return NULL;
}


#line 1772 "src/parse.exc"
 Node *parseLabeledStatement(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_ID))) {
        do { ret = newNode(parent, NODE_LABELED_STATEMENT, tok, 2); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = expectN(state, ret, TOK_COLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = parseStatement(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    if ((tok = expect(state, TOK_case))) {
        do { ret = newNode(parent, NODE_CASE_STATEMENT, tok, 3); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseConditionalExpression(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = expectN(state, ret, TOK_COLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = parseStatement(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    if ((tok = expect(state, TOK_default))) {
        do { ret = newNode(parent, NODE_DEFAULT_STATEMENT, tok, 2); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = expectN(state, ret, TOK_COLON))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = parseStatement(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return NULL;
}


#line 1802 "src/parse.exc"
 Node *parseStatement(ParseState *state, Node *parent)
{
    Node *ret;
    if ((ret = parseLabeledStatement(state, parent))) return ret;
    if ((ret = parseCompoundStatement(state, parent))) return ret;
    if ((ret = parseExpressionStatement(state, parent))) return ret;
    if ((ret = parseSelectionStatement(state, parent))) return ret;
    if ((ret = parseIterationStatement(state, parent))) return ret;
    if ((ret = parseJumpStatement(state, parent))) return ret;
    return NULL;
}

/***************************************************************
 * TRANSLATION UNIT                                            *
 ***************************************************************/

#line 1817 "src/parse.exc"
 Node *parseDeclarationListL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); while ((node = parseDeclaration(state, parent))) { WRITE_ONE_BUFFER(buf, node); } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_DECLARATION_LIST, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; buf.buf[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 1817 "src/parse.exc"
 Node *parseDeclarationList(ParseState *state, Node *parent) { return parseDeclarationListL(state, parent, 1); } 
#line 1817 "src/parse.exc"
 Node *parseDeclarationListOpt(ParseState *state, Node *parent) { return parseDeclarationListL(state, parent, 0); }


#line 1819 "src/parse.exc"
 Node *parseFunctionDefinition(ParseState *state, Node *parent)
{
    Node *ret, *node;

    if ((node = parseDeclarationDecoratorList(state, parent))) {
        do { ret = newNode(parent, NODE_DECORATED_FUNCTION_DEFINITION, NULL, 2); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
        { if (!(ret->children[1] = parseFunctionDefinition(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    if (!(node = parseDeclarationSpecifiers(state, parent))) return NULL;
    do { ret = newNode(parent, NODE_FUNCTION_DEFINITION, NULL, 3); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = parseDeclarator(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
    { if (!(ret->children[2] = parseCompoundStatement(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
    return ret;
}


#line 1836 "src/parse.exc"
 Node *parseExternalDeclaration(ParseState *state, Node *parent)
{
    Node *ret;
    if ((ret = parseFunctionDefinition(state, parent))) return ret;
    if ((ret = parseDeclaration(state, parent))) return ret;
    return NULL;
}


#line 1844 "src/parse.exc"
 Node *parseTranslationUnitL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); while ((node = parseExternalDeclaration(state, parent))) { WRITE_ONE_BUFFER(buf, node); } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_TRANSLATION_UNIT, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; buf.buf[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 1844 "src/parse.exc"
 Node *parseTranslationUnit(ParseState *state, Node *parent) { return parseTranslationUnitL(state, parent, 1); } 
#line 1844 "src/parse.exc"
 Node *parseTranslationUnitOpt(ParseState *state, Node *parent) { return parseTranslationUnitL(state, parent, 0); }


#line 1846 "src/parse.exc"
 Node *parseTop(ParseState *state, Node *parent)
{
    Node *ret, *node;
    if (!(node = parseTranslationUnit(state, parent)))
        /* allow empty translation units */
        node = newNode(parent, NODE_TRANSLATION_UNIT, NULL, 0);
    do { ret = newNode(parent, NODE_FILE, NULL, 2); if (!ret) { pushNode(state, node); freeNode(node); return NULL; } ret->children[0] = node; node->parent = ret; } while (0);
    { if (!(ret->children[1] = expectN(state, ret, TOK_TERM))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
    return ret;
}

/***************************************************************
 * DECORATION                                                  *
 ***************************************************************/

#line 1860 "src/parse.exc"
 Node *parseDecorationOpenOpt(ParseState *state, Node *parent);


#line 1862 "src/parse.exc"
 Node *parseDecorationSubDeclaration(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if (!(tok = expect(state, TOK_LBRACE))) return NULL;

    do { ret = newNode(parent, NODE_DECORATION_SUB_DECLARATION, tok, 2); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
    { if (!(ret->children[0] = parseTranslationUnitOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
    { if (!(ret->children[1] = expectN(state, ret, TOK_RBRACE))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
    return ret;
}

#line 1874 "src/parse.exc"
 Node *parseDecorationSubDeclarationOpt(ParseState *state, Node *parent) { Node *ret; if ((ret = parseDecorationSubDeclaration(state, parent))) return ret; return newNode(parent, NODE_NIL, NULL, 0); }


#line 1876 "src/parse.exc"
 Node *parseDeclarationDecorator(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_DECORATION))) {
        do { ret = newNode(parent, NODE_DECLARATION_DECORATOR, tok, 3); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseDecorationName(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = parseDecorationOpenOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = parseDecorationSubDeclarationOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    if ((tok = expect(state, TOK_OPEN_DECORATION))) {
        do { ret = newNode(parent, NODE_DECLARATION_DECORATOR, tok, 4); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseDecorationName(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = parseDecorationOpenOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = parseDecorationSubDeclarationOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[3] = expectN(state, ret, TOK_CLOSE_DECORATION))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return NULL;
}


#line 1901 "src/parse.exc"
 Node *parseDeclarationDecoratorListL(ParseState *state, Node *parent, int need1) { Node *ret, *node; struct Buffer_Nodep buf; size_t i; INIT_BUFFER(buf); while ((node = parseDeclarationDecorator(state, parent))) { WRITE_ONE_BUFFER(buf, node); } if (need1 && buf.bufused == 0) { FREE_BUFFER(buf); return NULL; } ret = newNode(parent, NODE_DECLARATION_DECORATOR_LIST, NULL, buf.bufused); for (i = 0; i < buf.bufused; i++) { ret->children[i] = buf.buf[i]; buf.buf[i]->parent = ret; } FREE_BUFFER(buf); return ret; } 
#line 1901 "src/parse.exc"
 Node *parseDeclarationDecoratorList(ParseState *state, Node *parent) { return parseDeclarationDecoratorListL(state, parent, 1); } 
#line 1901 "src/parse.exc"
 Node *parseDeclarationDecoratorListOpt(ParseState *state, Node *parent) { return parseDeclarationDecoratorListL(state, parent, 0); }


#line 1903 "src/parse.exc"
 Node *parseDecorationSubExpression(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;
    Token *tok;

    if (!(tok = expect(state, TOK_LBRACE))) return NULL;

    do { ret = newNode(parent, NODE_DECORATION_SUB_EXPRESSION, tok, 2); if (!ret) { pushToken(state, tok); return NULL; } } while (0);

    /* expression form */
    if ((node = parseExpressionOpt(state, ret))) {
        if ((node2 = expectN(state, ret, TOK_RBRACE))) {
            ret->children[0] = node;
            ret->children[1] = node2;
            return ret;
        }
        pushNode(state, node);
        freeNode(node);
    }

    /* statement form */
    if ((node = parseBlockItemList(state, ret))) {
        ret->children[0] = node;
        { if (!(ret->children[1] = expectN(state, ret, TOK_RBRACE))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    /* failure form! */
    pushNode(state, ret);
    freeNode(ret);
    return NULL;
}

#line 1935 "src/parse.exc"
 Node *parseDecorationSubExpressionOpt(ParseState *state, Node *parent) { Node *ret; if ((ret = parseDecorationSubExpression(state, parent))) return ret; return newNode(parent, NODE_NIL, NULL, 0); }


#line 1937 "src/parse.exc"
 Node *parseExpressionDecorator(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_DECORATION))) {
        do { ret = newNode(parent, NODE_EXPRESSION_DECORATOR, tok, 3); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseDecorationName(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = parseDecorationOpenOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = parseDecorationSubExpressionOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    if ((tok = expect(state, TOK_OPEN_DECORATION))) {
        do { ret = newNode(parent, NODE_EXPRESSION_DECORATOR, tok, 4); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseDecorationName(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = parseDecorationOpenOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[2] = parseDecorationSubExpressionOpt(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[3] = expectN(state, ret, TOK_CLOSE_DECORATION))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return NULL;
}


#line 1962 "src/parse.exc"
 Node *parseDecorationOpExpression(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;

    if (!(node = parseCastExpression(state, parent))) return NULL;

    while ((node2 = parseExpressionDecorator(state, parent))) {
        do { ret = newNode(parent, NODE_DECORATION_OP, NULL, 3); if (!ret) { pushNode(state, node); pushNode(state, node2); freeNode(node); freeNode(node2); return NULL; } ret->children[0] = node; ret->children[1] = node2; node->parent = ret; node2->parent = ret; } while (0);
        { if (!(ret->children[2] = parseCastExpression(state, ret))) { { do { node = ret->children[0]; node->parent = parent; do { size_t sri_; for (sri_ = 0; ret->children[sri_]; sri_++) ret->children[sri_] = ret->children[sri_+1]; } while (0); pushNode(state, ret); freeNode(ret); } while (0); break; } } };
        node = ret;
    }

    return node;
}


#line 1977 "src/parse.exc"
 Node *parseDecorationOpenCont(ParseState *state, Node *parent)
{
    Node *ret, *node;
    Token *tok;
    struct Buffer_Nodep buf;
    size_t i;
    ssize_t depth = 0;

    INIT_BUFFER(buf);

    while (1) {
        tok = scan(state); /* FIXME: NULL */
        if (tok->type == TOK_LPAREN) {
            depth++;

        } else if (tok->type == TOK_RPAREN) {
            if ((depth--) == 0) {
                pushToken(state, tok);
                break;
            }

        } else if (tok->type == TOK_TERM) {
            pushToken(state, tok);
            break;

        }

        node = newNode(parent, NODE_TOK, tok, 0); /* FIXME: NULL */
        WRITE_ONE_BUFFER(buf, node);
    }

    /* FXIME: NULL */
    ret = newNode(parent, NODE_DECORATION_OPEN_CONT, NULL, buf.bufused);

    for (i = 0; i < buf.bufused; i++) {
        ret->children[i] = buf.buf[i];
        buf.buf[i]->parent = ret;
    }

    FREE_BUFFER(buf);

    return ret;
}


#line 2021 "src/parse.exc"
 Node *parseDecorationOpen(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_LPAREN))) {
        do { ret = newNode(parent, NODE_DECORATION_OPEN, tok, 2); if (!ret) { pushToken(state, tok); return NULL; } } while (0);
        { if (!(ret->children[0] = parseDecorationOpenCont(state, ret))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        { if (!(ret->children[1] = expectN(state, ret, TOK_RPAREN))) { { pushNode(state, ret); freeNode(ret); return NULL; } } };
        return ret;
    }

    return NULL;
}

#line 2035 "src/parse.exc"
 Node *parseDecorationOpenOpt(ParseState *state, Node *parent) { Node *ret; if ((ret = parseDecorationOpen(state, parent))) return ret; return newNode(parent, NODE_NIL, NULL, 0); }


/***************************************************************
 * ENTRY POINT                                                 *
 ***************************************************************/

#line 2041 "src/parse.exc"
 Node *cparse(ScanState *state, char **error)
{
    ParseState pState;
    Node *ret;
    size_t i;

    pState.scanState = state;
    INIT_BUFFER(pState.buf);
    pState.eidx = pState.el = pState.ec = 0;
    INIT_BUFFER(pState.eexpected);

    ret = parseTop(&pState, NULL);

    if (!ret) {
        /* failed to parse, find out why */
        struct Buffer_char ebuf;
        char *fname;
        size_t tmps;

        INIT_BUFFER(ebuf);

        /* get the appropriate filename */
        if (pState.ef < state->filenames->bufused)
            fname = state->filenames->buf[pState.ef];
        else
            fname = "???";

        tmps = 32 + strlen(fname) + 2*4*sizeof(int);
        while (BUFFER_SPACE(ebuf) <= tmps)
            EXPAND_BUFFER(ebuf);
        ebuf.bufused += sprintf(ebuf.buf + ebuf.bufused,
                                "File %s line %d column %d\n expected: ", fname, (int) pState.el, (int) pState.ec);

        /* remove redundant expected names */
        for (i = 0; i < pState.eexpected.bufused; i++) {
            size_t j;
            int expi = pState.eexpected.buf[i];
            for (j = i + 1; j < pState.eexpected.bufused; j++) {
                if (expi == pState.eexpected.buf[j]) {
                    pState.eexpected.buf[j] = TOK_FIRST;
                }
            }
        }

        /* write out each of the expected names */
        for (i = 0; i < pState.eexpected.bufused; i++) {
            const char *tname;
            int ttype = pState.eexpected.buf[i];
            if (ttype != TOK_FIRST) {
                if (i != 0) WRITE_ONE_BUFFER(ebuf, ',');
                tname = tokenName(ttype);
                WRITE_BUFFER(ebuf, tname, strlen(tname));
            }
        }

        WRITE_BUFFER(ebuf, "\n found: ", 9);

        /* and the found name */
        {
            const char *tname = tokenName(pState.efound);
            WRITE_BUFFER(ebuf, tname, strlen(tname));
        }

        WRITE_BUFFER(ebuf, "\n", 2);

        *error = ebuf.buf;
    }

    for (i = 0; i < pState.buf.bufused; i++)
        freeToken(pState.buf.buf[i]);
    FREE_BUFFER(pState.buf);
    FREE_BUFFER(pState.eexpected);

    return ret;
}


#line 2117 "src/parse.exc"
 Node *cquickparse(struct Buffer_char *buf, parser_func_t func)
{
    ScanState state;
    ParseState pState;
    Node *ret;
    int i;

    state = newScanState(NULL);
    state.buf = *buf;

    pState.scanState = &state;
    INIT_BUFFER(pState.buf);
    pState.eidx = pState.el = pState.ec = 0;
    INIT_BUFFER(pState.eexpected);

    ret = func(&pState, NULL);

    for (i = 0; i < pState.buf.bufused; i++)
        freeToken(pState.buf.buf[i]);
    FREE_BUFFER(pState.buf);
    FREE_BUFFER(pState.eexpected);

    return ret;
}
#line 1 "<stdin>"
