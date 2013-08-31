#include "parse.h"

BUFFER(Nodep, Node *);

Node *newNode(Node *parent, int type, Token *tok, size_t children)
{
    Node *ret = calloc(sizeof(Node) + children * sizeof(Node *), 1);
    ret->parent = parent;
    ret->type = type;
    ret->tok = tok;
    return ret;
}

void freeNode(Node *node)
{
    size_t i;
    for (i = 0; node->children[i]; i++)
        freeNode(node->children[i]);
    if (node->tok)
        freeToken(node->tok);
    free(node);
}

static Token *scan(ParseState *state)
{
    if (state->buf.bufused) {
        state->buf.bufused--;
        return state->buf.buf[state->buf.bufused];
    }

    return cscan(state->scanState);
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
        pushToken(state, node->tok);
        node->tok = NULL;
    }
}

static Token *expect(ParseState *state, int type)
{
    Token *ret = scan(state);

    if (ret->type == type) return ret;

    pushToken(state, ret);
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
        pushNode(state, node); \
        freeNode(node); \
        return NULL; \
    } \
    ret->children[0] = node; \
    node->parent = ret; \
} while (0)

#define MKRETN2(nodeType, chn) do { \
    ret = newNode(parent, nodeType, NULL, chn); \
    if (!ret) { \
        pushNode(state, node); \
        pushNode(state, node2); \
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
        pushToken(state, tok); \
        return NULL; \
    } \
} while (0)

#define SHIFTRET() do { \
    size_t sri_; \
    for (sri_ = 0; ret->children[sri_]; sri_++) \
        ret->children[sri_] = ret->children[sri_+1]; \
} while (0)

#define RESTOREN() do { \
    node = ret->children[0]; \
    node->parent = parent; \
    SHIFTRET(); \
    pushNode(state, ret); \
    freeNode(ret); \
} while (0)

#define RESTORET() do { \
    tok = ret->children[0]->tok; \
    ret->children[0]->tok = NULL; \
    pushNode(state, ret); \
    freeNode(ret); \
} while (0)

#define REQUIREPO(chn, parser, iffail) do { \
if (!(ret->children[chn] = parser(state, ret))) { \
    iffail \
} \
} while(0)

#define REQUIREP(chn, parser) REQUIREPO(chn, parser, { \
    pushNode(state, ret); \
    freeNode(ret); \
    return NULL; \
})

#define REQUIREPR(chn, parser) REQUIREPO(chn, parser, RESTORE)

#define REQUIRETO(chn, toktype, iffail) do { \
if (!(ret->children[chn] = expectN(state, ret, toktype))) { \
    iffail \
} \
} while (0)

#define REQUIRET(chn, toktype) REQUIRETO(chn, toktype, { \
    pushNode(state, ret); \
    freeNode(ret); \
    return NULL; \
})

#define REQUIRETR(chn, toktype) REQUIRETO(chn, toktype, RESTORE)

#define OPT(name) \
static Node *name ## Opt(ParseState *state, Node *parent) \
{ \
    Node *ret; \
    if ((ret = name(state, parent))) return ret; \
    return newNode(parent, NODE_NIL, NULL, 0); \
}

/* temporary */
#define FAKEPARSER(name) \
static Node *name(ParseState *state, Node *parent) { return NULL; }

FAKEPARSER(parseInitializerList)
FAKEPARSER(parseEnumerationConstant)
FAKEPARSER(parseStaticAssertDeclaration)
FAKEPARSER(parseInitializer)

/***************************************************************
 * IDENTIFIERS/CONSTANTS                                       *
 ***************************************************************/
static Node *parseIdentifier(ParseState *state, Node *parent)
{
    Node *ret = expectN(state, parent, TOK_ID);
    if (!ret) return NULL;
    ret->type = NODE_ID;
    return ret;
}
OPT(parseIdentifier)

static Node *parseConstant(ParseState *state, Node *parent)
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

    if ((ret = expectN(state, parent, TOK_STR_LITERAL))) {
        ret->type = NODE_STR_LITERAL;
        return ret;
    }

    return NULL;
}

#define PARSER_LIST(parserName) \
static Node *parserName(ParseState *state, Node *parent) { return parserName ## L(state, parent, 1); } \
static Node *parserName ## Opt(ParseState *state, Node *parent) { return parserName ## L(state, parent, 0); }

#define COMMA_LIST(parserName, genNode, nextParser) \
static Node *parserName ## L(ParseState *state, Node *parent, int need1) \
{ \
    Node *ret, *node; \
    struct Buffer_Nodep buf; \
    size_t i; \
    \
    INIT_BUFFER(buf); \
    \
    if ((node = nextParser(state, parent))) { \
        WRITE_ONE_BUFFER(buf, node); \
        \
        while ((node = expectN(state, parent, TOK_COMMA))) { \
            WRITE_ONE_BUFFER(buf, node); \
            \
            if (!(node = nextParser(state, parent))) { \
                node = buf.buf[buf.bufused-1]; \
                buf.bufused--; \
                pushNode(state, node); \
                freeNode(node); \
                break; \
            } \
            \
            WRITE_ONE_BUFFER(buf, node); \
        } \
    } \
    \
    if (need1 && buf.bufused == 0) { \
        FREE_BUFFER(buf); \
        return NULL; \
    } \
    \
    /* OK, got the whole list */ \
    ret = newNode(parent, genNode, NULL, buf.bufused); \
    for (i = 0; i < buf.bufused; i++) { \
        ret->children[i] = buf.buf[i]; \
        ret->children[i]->parent = ret; \
    } \
    \
    FREE_BUFFER(buf); \
    \
    return ret; \
} \
PARSER_LIST(parserName)

/***************************************************************
 * EXPRESSIONS                                                 *
 ***************************************************************/
static Node *parsePrimaryExpression(ParseState *state, Node *parent);
static Node *parseCastExpression(ParseState *state, Node *parent);
static Node *parseAssignmentExpression(ParseState *state, Node *parent);
static Node *parseExpression(ParseState *state, Node *parent);
static Node *parseTypeName(ParseState *state, Node *parent);

COMMA_LIST(parseArgumentExpressionList, NODE_ARGUMENT_EXPRESSION_LIST, parseAssignmentExpression)

static Node *parsePostfixExpression(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;
    Token *tok;

    if ((tok = expect(state, TOK_LPAREN))) {
        /* a compound literal */
        MKRETT(NODE_COMPOUND_LITERAL, 6);
        REQUIREP(0, parseTypeName);
        REQUIRET(1, TOK_LPAREN);
        REQUIRET(2, TOK_LBRACE);
        REQUIREP(3, parseInitializerList);
        if ((ret->children[4] = expectN(state, ret, TOK_COMMA))) {
            REQUIRET(5, TOK_RBRACE);
        } else {
            REQUIRET(4, TOK_RBRACE);
        }
        node = ret;

    } else if (!(node = parsePrimaryExpression(state, parent))) {
        return NULL;

    }

    while (1) {
#define RESTORE { RESTOREN(); break; }
        if ((node2 = expectN(state, parent, TOK_LBRACKET))) {
            MKRETN2(NODE_INDEX, 4);
            REQUIREPR(2, parseExpression);
            REQUIRETR(3, TOK_RBRACKET);
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, parent, TOK_LPAREN))) {
            MKRETN2(NODE_CALL, 4);
            REQUIREPR(2, parseArgumentExpressionListOpt);
            REQUIRETR(3, TOK_RPAREN);
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, parent, TOK_DOT))) {
            MKRETN2(NODE_MEMBER_DOT, 3);
            REQUIRETR(2, TOK_ID);
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, parent, TOK_ARROW))) {
            MKRETN2(NODE_MEMBER_ARROW, 3);
            REQUIRETR(2, TOK_ID);
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, parent, TOK_PLUSPLUS))) {
            MKRETN2(NODE_POSTINC, 2);
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, parent, TOK_MINUSMINUS))) {
            MKRETN2(NODE_POSTDEC, 2);
            node = ret;
            continue;
        }
#undef RESTORE

        break;
    }

    return node;
}

static Node *parseUnaryTypeExpression(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_sizeof))) {
        MKRETT(NODE_SIZEOF_TYPE, 3);
        REQUIRET(0, TOK_LPAREN);
        REQUIREP(1, parseTypeName);
        REQUIRET(2, TOK_RPAREN);
        return ret;
    }

    if ((tok = expect(state, TOK__Alignof))) {
        MKRETT(NODE_ALIGNOF, 3);
        REQUIRET(0, TOK_LPAREN);
        REQUIREP(1, parseTypeName);
        REQUIRET(2, TOK_RPAREN);
        return ret;
    }

    return NULL;
}

static Node *parseUnaryExpression(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((ret = parsePostfixExpression(state, parent))) return ret;

#define UNARY_OP(reqTok, genNode) do { \
    if ((tok = expect(state, reqTok))) { \
        MKRETT(genNode, 1); \
        REQUIREPO(0, parseUnaryExpression, { pushNode(state, ret); freeNode(ret); break; }); \
        return ret; \
    } \
} while (0);
    UNARY_OP(TOK_PLUSPLUS, NODE_PREINC);
    UNARY_OP(TOK_MINUSMINUS, NODE_PREDEC);
    UNARY_OP(TOK_AND, NODE_ADDROF);
    UNARY_OP(TOK_STAR, NODE_DEREF);
    UNARY_OP(TOK_PLUS, NODE_POSITIVE);
    UNARY_OP(TOK_MINUS, NODE_NEGATIVE);
    UNARY_OP(TOK_BNOT, NODE_BNOT);
    UNARY_OP(TOK_NOT, NODE_NOT);

    if ((ret = parseUnaryTypeExpression(state, parent))) return ret;

    UNARY_OP(TOK_sizeof, NODE_SIZEOF_EXP);
#undef UNARY_OP

    return NULL;
}

static Node *parseCastTypeExpression(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_LPAREN))) {
        MKRETT(NODE_CAST, 3);
        REQUIREP(0, parseTypeName);
        REQUIRET(1, TOK_RPAREN);
        REQUIREP(2, parseCastExpression);
        return ret;
    }

    return NULL;
}

static Node *parseCastExpression(ParseState *state, Node *parent)
{
    Node *ret;
    if ((ret = parseCastTypeExpression(state, parent))) return ret;
    if ((ret = parseUnaryExpression(state, parent))) return ret;
    return NULL;
}

#define BINARY_OP_START(name, nextNode) \
    static Node *name(ParseState *state, Node *parent) \
    { \
        Node *ret, *node, *node2; \
        if (!(node = nextNode(state, parent))) return NULL; \
        while (1) {
#define BINARY_OP(reqTok, genNode, nextNode) \
            if ((node2 = expectN(state, parent, reqTok))) { \
                MKRETN2(genNode, 3); \
                REQUIREPO(2, nextNode, { RESTOREN(); break; }); \
                node = ret; \
                continue; \
            } 
#define BINARY_OP_END() \
            break; \
        } \
        return node; \
    }

BINARY_OP_START(parseMultiplicativeExpression, parseCastExpression)
    BINARY_OP(TOK_STAR, NODE_MUL, parseCastExpression)
    BINARY_OP(TOK_DIV, NODE_DIV, parseCastExpression)
    BINARY_OP(TOK_MOD, NODE_MOD, parseCastExpression)
BINARY_OP_END()

BINARY_OP_START(parseAdditiveExpression, parseMultiplicativeExpression)
    BINARY_OP(TOK_PLUS, NODE_ADD, parseMultiplicativeExpression)
    BINARY_OP(TOK_MINUS, NODE_SUB, parseMultiplicativeExpression)
BINARY_OP_END()

BINARY_OP_START(parseShiftExpression, parseAdditiveExpression)
    BINARY_OP(TOK_SHL, NODE_SHL, parseAdditiveExpression)
    BINARY_OP(TOK_SHR, NODE_SHR, parseAdditiveExpression)
BINARY_OP_END()

BINARY_OP_START(parseRelationalExpression, parseShiftExpression)
    BINARY_OP(TOK_LT, NODE_LT, parseShiftExpression)
    BINARY_OP(TOK_GT, NODE_GT, parseShiftExpression)
    BINARY_OP(TOK_LTE, NODE_LTE, parseShiftExpression)
    BINARY_OP(TOK_GTE, NODE_GTE, parseShiftExpression)
BINARY_OP_END()

BINARY_OP_START(parseEqualityExpression, parseRelationalExpression)
    BINARY_OP(TOK_EQ, NODE_EQ, parseRelationalExpression)
    BINARY_OP(TOK_NEQ, NODE_NEQ, parseRelationalExpression)
BINARY_OP_END()

BINARY_OP_START(parseBAndExpression, parseEqualityExpression)
    BINARY_OP(TOK_AND, NODE_BAND, parseEqualityExpression)
BINARY_OP_END()

BINARY_OP_START(parseBXorExpression, parseBAndExpression)
    BINARY_OP(TOK_BXOR, NODE_BXOR, parseBAndExpression)
BINARY_OP_END()

BINARY_OP_START(parseBOrExpression, parseBXorExpression)
    BINARY_OP(TOK_BOR, NODE_BOR, parseBXorExpression)
BINARY_OP_END()

BINARY_OP_START(parseAndExpression, parseBOrExpression)
    BINARY_OP(TOK_ANDAND, NODE_AND, parseBOrExpression)
BINARY_OP_END()

BINARY_OP_START(parseOrExpression, parseAndExpression)
    BINARY_OP(TOK_OROR, NODE_OR, parseAndExpression)
BINARY_OP_END()

static Node *parseConditionalExpression(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;

    if (!(node = parseOrExpression(state, parent))) return NULL;

    if ((node2 = expectN(state, parent, TOK_HOOK))) {
        MKRETN2(NODE_CONDITIONAL, 5);
#define RESTORE { RESTOREN(); return node; }
        REQUIREPR(2, parseExpression);
        REQUIRETR(3, TOK_COLON);
        REQUIREPR(4, parseConditionalExpression);
#undef RESTORE
        return ret;
    }

    return node;
}

static Node *parseAssignmentExpression(ParseState *state, Node *parent)
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
        MKRETN(NODE_ASG, 3);
        node = newNode(ret, NODE_TOK, tok, 0);
        ret->children[1] = node;
        if (tok->type > TOK_ASG)
            ret->type = NODE_RASG;
        REQUIREPO(2, parseAssignmentExpression, { RESTOREN(); return node; });
        return ret;
    }

    return parseConditionalExpression(state, parent);
}
OPT(parseAssignmentExpression)

BINARY_OP_START(parseExpression, parseAssignmentExpression);
    BINARY_OP(TOK_COMMA, NODE_COMMA, parseAssignmentExpression);
BINARY_OP_END()

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

    if ((tok = expect(state, TOK_default))) {
        MKRETT(NODE_GENERIC_ASSOCIATION_DEFAULT, 2);
        REQUIRET(0, TOK_COLON);
        REQUIREP(1, parseAssignmentExpression);
        return ret;
    }

    return NULL;
}

COMMA_LIST(parseGenericAssocList, NODE_GENERIC_ASSOC_LIST, parseGenericAssociation)

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

    if ((tok = expect(state, TOK_LPAREN))) {
        /* parenthesized expression */
        MKRETT(NODE_PAREN, 2);
        REQUIREP(0, parseExpression);
        REQUIRET(1, TOK_RPAREN);
        return ret;
    }

    if ((ret = parseGenericSelection(state, parent))) return ret;

    return NULL;
}

/***************************************************************
 * DECLARATIONS                                                *
 **************************************************************/
static Node *parseTypeSpecifier(ParseState *state, Node *parent);
static Node *parseDeclarator(ParseState *state, Node *parent);
static Node *parseTypeQualifier(ParseState *state, Node *parent);
static Node *parseAbstractDeclarator(ParseState *state, Node *parent);
static Node *parsePointerOpt(ParseState *state, Node *parent);
static Node *parseSpecifierQualifierList(ParseState *state, Node *parent);
static Node *parseParameterTypeListOpt(ParseState *state, Node *parent);
static Node *parseDeclarationSpecifiers(ParseState *state, Node *parent);
static Node *parseTypeQualifierList(ParseState *state, Node *parent);
static Node *parseTypeQualifierListOpt(ParseState *state, Node *parent);

static Node *parseDirectAbstractDeclaratorA1(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_ABSTRACT_DECLARATOR, 5);
    REQUIRET(1, TOK_LBRACE);
    REQUIREP(2, parseTypeQualifierListOpt);
    REQUIREP(3, parseAssignmentExpressionOpt);
    REQUIRET(4, TOK_RBRACE);
    return ret;
}

static Node *parseDirectAbstractDeclaratorA2(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_ABSTRACT_DECLARATOR, 6);
    REQUIRET(1, TOK_LBRACE);
    REQUIRET(2, TOK_static);
    REQUIREP(3, parseTypeQualifierListOpt);
    REQUIREP(4, parseAssignmentExpression);
    REQUIRET(5, TOK_RBRACE);
    return ret;
}

static Node *parseDirectAbstractDeclaratorA3(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_ABSTRACT_DECLARATOR, 6);
    REQUIRET(1, TOK_LBRACE);
    REQUIREP(2, parseTypeQualifierList);
    REQUIRET(3, TOK_static);
    REQUIREP(4, parseAssignmentExpression);
    REQUIRET(5, TOK_RBRACE);
    return ret;
}

static Node *parseDirectAbstractDeclaratorA4(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_ABSTRACT_DECLARATOR, 4);
    REQUIRET(1, TOK_LBRACE);
    REQUIRET(2, TOK_STAR);
    REQUIRET(3, TOK_RBRACE);
    return ret;
}

static Node *parseDirectAbstractDeclaratorF(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_ABSTRACT_DECLARATOR, 4);
    REQUIRET(1, TOK_LPAREN);
    REQUIREP(2, parseParameterTypeListOpt);
    REQUIRET(3, TOK_RPAREN);
    return ret;
}

static Node *parseDirectAbstractDeclarator(ParseState *state, Node *parent)
{
    Node *ret, *node;
    Token *tok;

    node = NULL;

    if ((tok = expect(state, TOK_LPAREN))) {
        MKRETT(NODE_DIRECT_ABSTRACT_DECLARATOR, 2);
#define RESTORE { RESTORET(); goto pdadrestore; }
        REQUIREPR(0, parseAbstractDeclarator);
        REQUIRETR(1, TOK_RPAREN);
#undef RESTORE
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
OPT(parseAbstractDeclarator)

static Node *parseAbstractDeclarator(ParseState *state, Node *parent)
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

    MKRETN2(NODE_ABSTRACT_DECLARATOR, 2);
    return ret;
}

static Node *parseTypeName(ParseState *state, Node *parent)
{
    Node *ret, *node;

    if (!(node = parseSpecifierQualifierList(state, parent))) return NULL;

    MKRETN(NODE_TYPE_NAME, 2);
    REQUIREP(1, parseAbstractDeclaratorOpt);
    return ret;
}

COMMA_LIST(parseIdentifierList, NODE_IDENTIFIER_LIST, parseIdentifier)

static Node *parseParameterDeclaration(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;

    if (!(node = parseDeclarationSpecifiers(state, parent))) return NULL;

    if ((node2 = parseDeclarator(state, parent))) {
        MKRETN2(NODE_PARAMETER_DECLARATION, 2);
        return ret;

    } else if ((node2 = parseAbstractDeclaratorOpt(state, parent))) {
        MKRETN2(NODE_PARAMETER_DECLARATION, 2);
        return ret;

    } else {
        pushNode(state, node);
        freeNode(node);

        return NULL;

    }
}

COMMA_LIST(parseParameterList, NODE_PARAMETER_LIST, parseParameterDeclaration)

static Node *parseParameterTypeList(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;

    if (!(node = parseParameterList(state, parent))) return NULL;

    if ((node2 = expectN(state, parent, TOK_COMMA))) {
        MKRETN2(NODE_PARAMETER_TYPE_LIST, 3);
        REQUIRETO(2, TOK_DOTDOTDOT, { RESTOREN(); return node; });
        return ret;
    }

    return node;
}
OPT(parseParameterTypeList)

static Node *parseTypeQualifierListL(ParseState *state, Node *parent, int need1)
{
    Node *ret, *node;
    struct Buffer_Nodep buf;
    size_t i;

    INIT_BUFFER(buf);

    while ((node = parseTypeQualifier(state, parent))) {
        WRITE_ONE_BUFFER(buf, node);
    }

    if (need1 && buf.bufused == 0) {
        FREE_BUFFER(buf);
        return NULL;
    }

    ret = newNode(parent, NODE_TYPE_QUALIFIER_LIST, NULL, buf.bufused);
    for (i = 0; i < buf.bufused; i++) {
        ret->children[i] = buf.buf[i];
        buf.buf[i]->parent = ret;
    }

    return ret;
}
PARSER_LIST(parseTypeQualifierList)

static Node *parsePointerOpt(ParseState *state, Node *parent)
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
    MKRETN(NODE_DIRECT_DECLARATOR, 5);
    REQUIRET(1, TOK_LBRACE);
    REQUIREP(2, parseTypeQualifierListOpt);
    REQUIREP(3, parseAssignmentExpressionOpt);
    REQUIRET(4, TOK_RBRACE);
    return ret;
}

static Node *parseDirectDeclaratorA2(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_DECLARATOR, 6);
    REQUIRET(1, TOK_LBRACE);
    REQUIRET(2, TOK_static);
    REQUIREP(3, parseTypeQualifierListOpt);
    REQUIREP(4, parseAssignmentExpression);
    REQUIRET(5, TOK_RBRACE);
    return ret;
}

static Node *parseDirectDeclaratorA3(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_DECLARATOR, 6);
    REQUIRET(1, TOK_LBRACE);
    REQUIREP(2, parseTypeQualifierList);
    REQUIRET(3, TOK_static);
    REQUIREP(4, parseAssignmentExpression);
    REQUIRET(5, TOK_RBRACE);
    return ret;
}

static Node *parseDirectDeclaratorA4(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_DECLARATOR, 5);
    REQUIRET(1, TOK_LBRACE);
    REQUIREP(2, parseTypeQualifierListOpt);
    REQUIRET(3, TOK_STAR);
    REQUIRET(4, TOK_RBRACE);
    return ret;
}

static Node *parseDirectDeclaratorF1(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_DECLARATOR, 4);
    REQUIRET(1, TOK_LPAREN);
    REQUIREP(2, parseParameterTypeList);
    REQUIRET(3, TOK_RPAREN);
    return ret;
}

static Node *parseDirectDeclaratorF2(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_DECLARATOR, 4);
    REQUIRET(1, TOK_LPAREN);
    REQUIREP(2, parseIdentifierListOpt);
    REQUIRET(3, TOK_RPAREN);
    return ret;
}

static Node *parseDirectDeclarator(ParseState *state, Node *parent)
{
    Node *ret, *node;
    Token *tok;

    if ((tok = expect(state, TOK_ID))) {
        MKRETT(NODE_DIRECT_DECLARATOR, 0);
        node = ret;
    } else if ((tok = expect(state, TOK_LPAREN))) {
        MKRETT(NODE_DIRECT_DECLARATOR, 2);
        REQUIREP(0, parseDeclarator);
        REQUIRET(1, TOK_RPAREN);
        node = ret;
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

static Node *parseDeclarator(ParseState *state, Node *parent)
{
    Node *ret, *node;

    if (!(node = parsePointerOpt(state, parent))) return NULL;

    MKRETN(NODE_DECLARATOR, 2);
    REQUIREP(1, parseDirectDeclarator);

    return ret;
}

static Node *parseAlignmentSpecifier(ParseState *state, Node *parent)
{
    Node *ret, *node;
    Token *tok;

    if (!(tok = expect(state, TOK__Alignas))) return NULL;

    MKRETT(NODE_ALIGNMENT_SPECIFIER, 3);
    REQUIRET(0, TOK_LPAREN);

    if (!(node = parseTypeName(state, ret)) &&
        !(node = parseConditionalExpression(state, ret))) {
        pushNode(state, ret);
        freeNode(ret);
        return NULL;
    }

    ret->children[1] = node;
    REQUIRET(2, TOK_RPAREN);

    return ret;
}

static Node *parseFunctionSpecifier(ParseState *state, Node *parent)
{
    Node *ret;

    if ((ret = expectN(state, parent, TOK_inline)) ||
        (ret = expectN(state, parent, TOK__Noreturn))) {
        ret->type = NODE_FUNCTION_SPECIFIER;
        return ret;
    }

    return NULL;
}

static Node *parseTypeQualifier(ParseState *state, Node *parent)
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

static Node *parseAtomicTypeSpecifier(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if (!(tok = expect(state, TOK__Atomic))) return NULL;

    MKRETT(NODE_ATOMIC_TYPE_SPECIFIER, 3);
    REQUIRET(0, TOK_LPAREN);
    REQUIREP(1, parseTypeName);
    REQUIRET(2, TOK_RPAREN);

    return ret;
}

static Node *parseEnumerator(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;

    if (!(node = parseEnumerationConstant(state, parent))) return NULL;

    if ((node2 = expectN(state, parent, TOK_ASG))) {
        MKRETN2(NODE_ENUMERATOR, 3);
        REQUIREPO(2, parseConditionalExpression, { RESTOREN(); return node; });
        return ret;
    }

    return node;
}

COMMA_LIST(parseEnumeratorList, NODE_ENUMERATOR_LIST, parseEnumerator)

static Node *parseEnumSpecifier(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_enum))) {
        MKRETT(NODE_ENUM_SPECIFIER, 5);
        REQUIREP(0, parseIdentifierOpt);

        if (ret->children[0]->tok) {
            /* the rest is optional */
            ret->children[1] = expectN(state, ret, TOK_LBRACE);

        } else {
            /* the rest is mandatory */
            REQUIRET(1, TOK_LBRACE);

        }

        if (ret->children[1]) {
            REQUIREP(2, parseEnumeratorList);
            if ((ret->children[3] = expectN(state, ret, TOK_COMMA))) {
                REQUIRET(4, TOK_RBRACE);
            } else {
                REQUIRET(3, TOK_RBRACE);
            }
        }

        return ret;
    }

    return NULL;
}

static Node *parseStructDeclarator(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;
    Token *tok;

    if ((node = parseDeclarator(state, parent))) {
        if ((node2 = expectN(state, parent, TOK_COLON))) {
            MKRETN2(NODE_BITFIELD_DECLARATOR, 3);
            REQUIREPO(2, parseConstant, { RESTOREN(); return node; });
            return ret;
        }
        return node;

    } else if ((tok = expect(state, TOK_COLON))) {
        MKRETT(NODE_BITFIELD_PADDING, 1);
        REQUIREP(0, parseConstant);
        return ret;

    }

    return NULL;
}

COMMA_LIST(parseStructDeclaratorList, NODE_STRUCT_DECLARATOR_LIST, parseStructDeclarator)

static Node *parseSpecifierQualifierListL(ParseState *state, Node *parent, int need1)
{
    Node *ret;
    struct Buffer_Nodep buf;
    size_t i;

    INIT_BUFFER(buf);

    while ((ret = parseTypeSpecifier(state, parent)) ||
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

    FREE_BUFFER(buf);

    return ret;
}
PARSER_LIST(parseSpecifierQualifierList)

static Node *parseStructDeclaration(ParseState *state, Node *parent)
{
    Node *ret, *node;

    if ((node = parseSpecifierQualifierList(state, parent))) {
        MKRETN(NODE_STRUCT_DECLARATION, 3);
        REQUIREP(1, parseStructDeclaratorListOpt);
        REQUIRET(2, TOK_SEMICOLON);
        return ret;
    }

    if ((ret = parseStaticAssertDeclaration(state, parent))) return ret;

    return NULL;
}

static Node *parseStructDeclarationListL(ParseState *state, Node *parent, int need1)
{
    Node *ret;
    struct Buffer_Nodep buf;
    size_t i;

    INIT_BUFFER(buf);

    while ((ret = parseStructDeclaration(state, parent))) {
        WRITE_ONE_BUFFER(buf, ret);
    }

    if (need1 && buf.bufused == 0) {
        FREE_BUFFER(buf);
        return NULL;
    }

    ret = newNode(parent, NODE_STRUCT_DECLARATION_LIST, NULL, buf.bufused);
    for (i = 0; i < buf.bufused; i++) {
        ret->children[i] = buf.buf[i];
        buf.buf[i]->parent = ret;
    }

    FREE_BUFFER(buf);

    return ret;
}
PARSER_LIST(parseStructDeclarationList)

static Node *parseStructOrUnionSpecifier(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_struct)) ||
        (tok = expect(state, TOK_union))) {
        MKRETT(NODE_STRUCT_OR_UNION_SPECIFIER, 4);
        REQUIREP(0, parseIdentifierOpt);
        if (ret->children[0]->tok) {
            /* the rest is optional */
            ret->children[1] = expectN(state, ret, TOK_LBRACE);

        } else {
            /* the rest is mandatory */
            REQUIRET(1, TOK_LBRACE);

        }

        if (ret->children[1]) {
            REQUIREP(2, parseStructDeclarationList);
            REQUIRET(3, TOK_RBRACE);
        }

        return ret;
    }

    return NULL;
}

static Node *parseTypeSpecifier(ParseState *state, Node *parent)
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
        (ret = expectN(state, parent, TOK__Complex)) ||
        (ret = expectN(state, parent, TOK_ID))) {
        /* ID's are included as typedef-name */
        ret->type = NODE_TYPE_SPECIFIER;
        return ret;
    }

    if ((ret = parseAtomicTypeSpecifier(state, parent))) return ret;
    if ((ret = parseStructOrUnionSpecifier(state, parent))) return ret;
    if ((ret = parseEnumSpecifier(state, parent))) return ret;

    return NULL;
}

static Node *parseStorageClassSpecifier(ParseState *state, Node *parent)
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

static Node *parseInitDeclarator(ParseState *state, Node *parent)
{
    Node *ret, *node, *node2;

    if (!(node = parseDeclarator(state, parent))) return NULL;

    if ((node2 = expectN(state, parent, TOK_ASG))) {
        MKRETN2(NODE_INIT_DECLARATOR, 3);
        REQUIREPO(2, parseInitializer, { RESTOREN(); return node; });
        return ret;
    }

    return node;
}

COMMA_LIST(parseInitDeclaratorList, NODE_INIT_DECLARATOR_LIST, parseInitDeclarator)

static Node *parseDeclarationSpecifiers(ParseState *state, Node *parent)
{
    Node *ret, *node;
    struct Buffer_Nodep buf;
    size_t i;

    INIT_BUFFER(buf);

    while (1) {
        if ((node = parseStorageClassSpecifier(state, parent))) {
            WRITE_ONE_BUFFER(buf, node);
            continue;
        }

        if ((node = parseTypeSpecifier(state, parent))) {
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

    return ret;
}

static Node *parseDeclaration(ParseState *state, Node *parent)
{
    Node *ret, *node;

    if ((node = parseDeclarationSpecifiers(state, parent))) {
        MKRETN(NODE_DECLARATION, 3);
        REQUIREP(1, parseInitDeclaratorListOpt);
        REQUIRET(2, TOK_SEMICOLON);
        return ret;
    }

    return parseStaticAssertDeclaration(state, parent);
}


/***************************************************************
 * ENTRY POINT                                                 *
 ***************************************************************/
Node *cparse(ScanState *state)
{
    ParseState pState;
    Node *ret;

    pState.scanState = state;
    INIT_BUFFER(pState.buf);

    /* FIXME: obviously this would refer to the top-level parser, not expression */
    ret = parseExpression(&pState, NULL);

    FREE_BUFFER(pState.buf);

    return ret;
}
