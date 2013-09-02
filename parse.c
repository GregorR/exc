#include <string.h>

#include "parse.h"

#include "unparse.h"

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

/* prepare ret with a single node `node` */
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

/* prepare ret with nodes `node` and `node2` */
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

/* prepare ret with token `tok` */
#define MKRETT(nodeType, chn) do { \
    ret = newNode(parent, nodeType, tok, chn); \
    if (!ret) { \
        pushToken(state, tok); \
        return NULL; \
    } \
} while (0)

/* shift the first child off of `ret` (usually to restore it before
 * pushNode(ret)) */
#define SHIFTRET() do { \
    size_t sri_; \
    for (sri_ = 0; ret->children[sri_]; sri_++) \
        ret->children[sri_] = ret->children[sri_+1]; \
} while (0)

/* restore `node` from ret->children[0] and push/delete node */
#define RESTOREN() do { \
    node = ret->children[0]; \
    node->parent = parent; \
    SHIFTRET(); \
    pushNode(state, ret); \
    freeNode(ret); \
} while (0)

/* require a parser succeed, else iffail 
 * This is INTENTIONALLY not a do{}while, to allow for breaks */
#define REQUIREPO(chn, parser, iffail) { \
if (!(ret->children[chn] = parse ## parser(state, ret))) { \
    iffail \
} \
}

/* require a parser succeed else return NULL */
#define REQUIREP(chn, parser) REQUIREPO(chn, parser, { \
    pushNode(state, ret); \
    freeNode(ret); \
    return NULL; \
})

/* require a parser succeed else RESTORE */
#define REQUIREPR(chn, parser) REQUIREPO(chn, parser, RESTORE)

/* require a token be found else iffail */
#define REQUIRETO(chn, toktype, iffail) { \
if (!(ret->children[chn] = expectN(state, ret, toktype))) { \
    iffail \
} \
}

/* require a token be found else return NULL */
#define REQUIRET(chn, toktype) REQUIRETO(chn, toktype, { \
    pushNode(state, ret); \
    freeNode(ret); \
    return NULL; \
})

/* require a token be found else RESTORE */
#define REQUIRETR(chn, toktype) REQUIRETO(chn, toktype, RESTORE)

/* type for a parser */
#define PARSER(name) static Node *parse ## name(ParseState *state, Node *parent)

/* make an optional parser based on a parser `name` */
#define OPT(name) \
PARSER(name ## Opt) \
{ \
    Node *ret; \
    if ((ret = parse ## name(state, parent))) return ret; \
    return newNode(parent, NODE_NIL, NULL, 0); \
}

/* temporary */
#define FAKEPARSER(name) \
static Node *parse ## name(ParseState *state, Node *parent) { return NULL; }

/***************************************************************
 * IDENTIFIERS/CONSTANTS                                       *
 ***************************************************************/
PARSER(DecorationName)
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
            MKRETT(NODE_DECORATION_NAME, 0);
            return ret;
    }

    return NULL;
}

PARSER(Identifier)
{
    Node *ret = expectN(state, parent, TOK_ID);
    if (!ret) return NULL;
    ret->type = NODE_ID;
    return ret;
}
OPT(Identifier)

PARSER(Constant)
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
PARSER(parserName) { return parse ## parserName ## L(state, parent, 1); } \
PARSER(parserName ## Opt) { return parse ## parserName ## L(state, parent, 0); }

#define COMMA_LIST(parserName, genNode, nextParser) \
static Node *parse ## parserName ## L(ParseState *state, Node *parent, int need1) \
{ \
    Node *ret, *node; \
    struct Buffer_Nodep buf; \
    size_t i; \
    \
    INIT_BUFFER(buf); \
    \
    if ((node = parse ## nextParser(state, parent))) { \
        WRITE_ONE_BUFFER(buf, node); \
        \
        while ((node = expectN(state, parent, TOK_COMMA))) { \
            WRITE_ONE_BUFFER(buf, node); \
            \
            if (!(node = parse ## nextParser(state, parent))) { \
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

#define CONCAT_LIST(parserName, genNode, nextParser) \
static Node *parse ## parserName ## L(ParseState *state, Node *parent, int need1) \
{ \
    Node *ret, *node; \
    struct Buffer_Nodep buf; \
    size_t i; \
    \
    INIT_BUFFER(buf); \
    \
    while ((node = parse ## nextParser(state, parent))) { \
        WRITE_ONE_BUFFER(buf, node); \
    } \
    \
    if (need1 && buf.bufused == 0) { \
        FREE_BUFFER(buf); \
        return NULL; \
    } \
    \
    ret = newNode(parent, genNode, NULL, buf.bufused); \
    for (i = 0; i < buf.bufused; i++) { \
        ret->children[i] = buf.buf[i]; \
        buf.buf[i]->parent = ret; \
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
PARSER(PrimaryExpression);
PARSER(CastExpression);
PARSER(AssignmentExpression);
PARSER(Expression);
PARSER(TypeName);
PARSER(InitializerList);
PARSER(DecorationOpExpression);
PARSER(ExpressionDecorator);

COMMA_LIST(ArgumentExpressionList, NODE_ARGUMENT_EXPRESSION_LIST, AssignmentExpression)

PARSER(PostfixExpression)
{
    Node *ret, *node, *node2;
    Token *tok;

    node = NULL;
    if ((tok = expect(state, TOK_LPAREN))) {
        /* a compound literal */
        MKRETT(NODE_COMPOUND_LITERAL, 6);
#define RESTORE { pushNode(state, ret); freeNode(ret); node = NULL; goto notcompound; }
        REQUIREPR(0, TypeName);
        REQUIRETR(1, TOK_RPAREN);
        REQUIRETR(2, TOK_LBRACE);
        REQUIREPR(3, InitializerList);
        if ((ret->children[4] = expectN(state, ret, TOK_COMMA))) {
            REQUIRETR(5, TOK_RBRACE);
        } else {
            REQUIRETR(4, TOK_RBRACE);
        }
#undef RESTORE
        node = ret;

        notcompound: (void) 0;

    }

    if (!node && !(node = parsePrimaryExpression(state, parent))) {
        return NULL;

    }

    while (1) {
#define RESTORE { RESTOREN(); break; }
        if ((node2 = expectN(state, parent, TOK_LBRACKET))) {
            MKRETN2(NODE_INDEX, 4);
            REQUIREPR(2, Expression);
            REQUIRETR(3, TOK_RBRACKET);
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, parent, TOK_LPAREN))) {
            MKRETN2(NODE_CALL, 4);
            REQUIREPR(2, ArgumentExpressionListOpt);
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

PARSER(UnaryTypeExpression)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_sizeof))) {
        MKRETT(NODE_SIZEOF_TYPE, 3);
        REQUIRET(0, TOK_LPAREN);
        REQUIREP(1, TypeName);
        REQUIRET(2, TOK_RPAREN);
        return ret;
    }

    if ((tok = expect(state, TOK__Alignof))) {
        MKRETT(NODE_ALIGNOF, 3);
        REQUIRET(0, TOK_LPAREN);
        REQUIREP(1, TypeName);
        REQUIRET(2, TOK_RPAREN);
        return ret;
    }

    return NULL;
}

PARSER(UnaryExpression)
{
    Node *ret;
    Token *tok;

    if ((ret = parsePostfixExpression(state, parent))) return ret;

#define UNARY_OP(reqTok, genNode) do { \
    if ((tok = expect(state, reqTok))) { \
        MKRETT(genNode, 1); \
        REQUIREPO(0, UnaryExpression, { pushNode(state, ret); freeNode(ret); break; }); \
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

PARSER(CastTypeExpression)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_LPAREN))) {
        MKRETT(NODE_CAST, 3);
        REQUIREP(0, TypeName);
        REQUIRET(1, TOK_RPAREN);
        REQUIREP(2, CastExpression);
        return ret;
    }

    return NULL;
}

PARSER(CastExpression)
{
    Node *ret;
    if ((ret = parseCastTypeExpression(state, parent))) return ret;
    if ((ret = parseUnaryExpression(state, parent))) return ret;
    return NULL;
}

#define BINARY_OP_START(name, nextNode) \
    PARSER(name) \
    { \
        Node *ret, *node, *node2; \
        if (!(node = parse ## nextNode(state, parent))) return NULL; \
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

BINARY_OP_START(MultiplicativeExpression, DecorationOpExpression)
    BINARY_OP(TOK_STAR, NODE_MUL, DecorationOpExpression)
    BINARY_OP(TOK_DIV, NODE_DIV, DecorationOpExpression)
    BINARY_OP(TOK_MOD, NODE_MOD, DecorationOpExpression)
BINARY_OP_END()

BINARY_OP_START(AdditiveExpression, MultiplicativeExpression)
    BINARY_OP(TOK_PLUS, NODE_ADD, MultiplicativeExpression)
    BINARY_OP(TOK_MINUS, NODE_SUB, MultiplicativeExpression)
BINARY_OP_END()

BINARY_OP_START(ShiftExpression, AdditiveExpression)
    BINARY_OP(TOK_SHL, NODE_SHL, AdditiveExpression)
    BINARY_OP(TOK_SHR, NODE_SHR, AdditiveExpression)
BINARY_OP_END()

BINARY_OP_START(RelationalExpression, ShiftExpression)
    BINARY_OP(TOK_LT, NODE_LT, ShiftExpression)
    BINARY_OP(TOK_GT, NODE_GT, ShiftExpression)
    BINARY_OP(TOK_LTE, NODE_LTE, ShiftExpression)
    BINARY_OP(TOK_GTE, NODE_GTE, ShiftExpression)
BINARY_OP_END()

BINARY_OP_START(EqualityExpression, RelationalExpression)
    BINARY_OP(TOK_EQ, NODE_EQ, RelationalExpression)
    BINARY_OP(TOK_NEQ, NODE_NEQ, RelationalExpression)
BINARY_OP_END()

BINARY_OP_START(BAndExpression, EqualityExpression)
    BINARY_OP(TOK_AND, NODE_BAND, EqualityExpression)
BINARY_OP_END()

BINARY_OP_START(BXorExpression, BAndExpression)
    BINARY_OP(TOK_BXOR, NODE_BXOR, BAndExpression)
BINARY_OP_END()

BINARY_OP_START(BOrExpression, BXorExpression)
    BINARY_OP(TOK_BOR, NODE_BOR, BXorExpression)
BINARY_OP_END()

BINARY_OP_START(AndExpression, BOrExpression)
    BINARY_OP(TOK_ANDAND, NODE_AND, BOrExpression)
BINARY_OP_END()

BINARY_OP_START(OrExpression, AndExpression)
    BINARY_OP(TOK_OROR, NODE_OR, AndExpression)
BINARY_OP_END()

PARSER(ConditionalExpression)
{
    Node *ret, *node, *node2;

    if (!(node = parseOrExpression(state, parent))) return NULL;

    if ((node2 = expectN(state, parent, TOK_HOOK))) {
        MKRETN2(NODE_CONDITIONAL, 5);
#define RESTORE { RESTOREN(); return node; }
        REQUIREPR(2, Expression);
        REQUIRETR(3, TOK_COLON);
        REQUIREPR(4, ConditionalExpression);
#undef RESTORE
        return ret;
    }

    return node;
}

PARSER(AssignmentExpression)
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
        REQUIREPO(2, AssignmentExpression, { RESTOREN(); return node; });
        return ret;
    }

    return parseConditionalExpression(state, parent);
}
OPT(AssignmentExpression)

BINARY_OP_START(Expression, AssignmentExpression);
    BINARY_OP(TOK_COMMA, NODE_COMMA, AssignmentExpression);
BINARY_OP_END()
OPT(Expression)

PARSER(GenericAssociation)
{
    Node *ret, *node;
    Token *tok;

    if ((node = parseTypeName(state, parent))) {
        MKRETN(NODE_GENERIC_ASSOCIATION, 3);
        REQUIRET(1, TOK_COLON);
        REQUIREP(2, AssignmentExpression);
        return ret;
    }

    if ((tok = expect(state, TOK_default))) {
        MKRETT(NODE_GENERIC_ASSOCIATION_DEFAULT, 2);
        REQUIRET(0, TOK_COLON);
        REQUIREP(1, AssignmentExpression);
        return ret;
    }

    return NULL;
}

COMMA_LIST(GenericAssocList, NODE_GENERIC_ASSOC_LIST, GenericAssociation)

PARSER(GenericSelection)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK__Generic))) {
        MKRETT(NODE_GENERIC_SELECTION, 5);
        REQUIRET(0, TOK_LPAREN);
        REQUIREP(1, AssignmentExpression);
        REQUIRET(2, TOK_COMMA);
        REQUIREP(3, GenericAssocList);
        REQUIRET(4, TOK_RPAREN);
        return ret;
    }

    return NULL;
}

PARSER(PrimaryExpression)
{
    Node *ret;
    Token *tok;

    if ((ret = parseIdentifier(state, parent))) return ret;
    if ((ret = parseConstant(state, parent))) return ret;

    if ((tok = expect(state, TOK_LPAREN))) {
        /* parenthesized expression */
        MKRETT(NODE_PAREN, 2);
        REQUIREP(0, Expression);
        REQUIRET(1, TOK_RPAREN);
        return ret;
    }

    if ((ret = parseGenericSelection(state, parent))) return ret;
    if ((ret = parseExpressionDecorator(state, parent))) return ret;

    return NULL;
}

/***************************************************************
 * DECLARATIONS                                                *
 **************************************************************/
PARSER(Declarator);
PARSER(TypeQualifier);
PARSER(AbstractDeclarator);
PARSER(PointerOpt);
PARSER(SpecifierQualifierList);
PARSER(ParameterTypeListOpt);
PARSER(DeclarationSpecifiers);
PARSER(TypeQualifierList);
PARSER(TypeQualifierListOpt);
PARSER(Initializer);
PARSER(DeclarationDecoratorList);

/* the third argument prevents multiple specifiers, particularly ID ID (since
 * this parser is typedef-ambiguous) */
static Node *parseTypeSpecifier(ParseState *state, Node *parent, int *foundSpecifier);

PARSER(StaticAssertDeclaration)
{
    Node *ret;
    Token *tok;

    if (!(tok = expect(state, TOK__Static_assert))) return NULL;

    MKRETT(NODE_STATIC_ASSERT_DECLARATION, 6);
    REQUIRET(0, TOK_LPAREN);
    REQUIREP(1, ConditionalExpression);
    REQUIRET(2, TOK_COMMA);
    REQUIRET(3, TOK_STR_LITERAL);
    REQUIRET(4, TOK_RPAREN);
    REQUIRET(5, TOK_SEMICOLON);

    return ret;
}

PARSER(Designator)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_LBRACKET))) {
        MKRETT(NODE_DESIGNATOR, 2);
        REQUIREP(0, ConditionalExpression);
        REQUIRET(1, TOK_RBRACKET);
        return ret;
    }

    if ((tok = expect(state, TOK_DOT))) {
        MKRETT(NODE_DESIGNATOR, 1);
        REQUIREP(0, Identifier);
        return ret;
    }

    return NULL;
}

CONCAT_LIST(DesignatorList, NODE_DESIGNATOR_LIST, Designator)

PARSER(Designation)
{
    Node *ret, *node;

    if (!(node = parseDesignatorList(state, parent))) return NULL;

    MKRETN(NODE_DESIGNATION, 2);
    REQUIRET(1, TOK_ASG);
    return ret;
}
OPT(Designation)

PARSER(DesignationInitializer)
{
    Node *ret, *node;

    if (!(node = parseDesignationOpt(state, parent))) return NULL;

    MKRETN(NODE_DESIGNATION_INITIALIZER, 2);
    REQUIREP(1, Initializer);

    return ret;
}

COMMA_LIST(InitializerList, NODE_INITIALIZER_LIST, DesignationInitializer)

PARSER(Initializer)
{
    Node *ret;
    Token *tok;

    if ((ret = parseAssignmentExpression(state, parent))) return ret;

    if ((tok = expect(state, TOK_LBRACE))) {
        MKRETT(NODE_INITIALIZER, 3);
        REQUIREP(0, InitializerList);

        /* ending , is optional */
        if ((ret->children[1] = expectN(state, ret, TOK_COMMA))) {
            REQUIRET(2, TOK_RBRACE);
        } else {
            REQUIRET(1, TOK_RBRACE);
        }

        return ret;
    }

    return NULL;
}

#define RESTORE { RESTOREN(); return NULL; }
static Node *parseDirectAbstractDeclaratorA1(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_ABSTRACT_DECLARATOR, 5);
    REQUIRETR(1, TOK_LBRACKET);
    REQUIREPR(2, TypeQualifierListOpt);
    REQUIREPR(3, AssignmentExpressionOpt);
    REQUIRETR(4, TOK_RBRACKET);
    return ret;
}

static Node *parseDirectAbstractDeclaratorA2(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_ABSTRACT_DECLARATOR, 6);
    REQUIRETR(1, TOK_LBRACKET);
    REQUIRETR(2, TOK_static);
    REQUIREPR(3, TypeQualifierListOpt);
    REQUIREPR(4, AssignmentExpression);
    REQUIRETR(5, TOK_RBRACKET);
    return ret;
}

static Node *parseDirectAbstractDeclaratorA3(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_ABSTRACT_DECLARATOR, 6);
    REQUIRETR(1, TOK_LBRACKET);
    REQUIREPR(2, TypeQualifierList);
    REQUIRETR(3, TOK_static);
    REQUIREPR(4, AssignmentExpression);
    REQUIRETR(5, TOK_RBRACKET);
    return ret;
}

static Node *parseDirectAbstractDeclaratorA4(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_ABSTRACT_DECLARATOR, 4);
    REQUIRETR(1, TOK_LBRACKET);
    REQUIRETR(2, TOK_STAR);
    REQUIRETR(3, TOK_RBRACKET);
    return ret;
}

static Node *parseDirectAbstractDeclaratorF(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_ABSTRACT_DECLARATOR, 4);
    REQUIRETR(1, TOK_LPAREN);
    REQUIREPR(2, ParameterTypeListOpt);
    REQUIRETR(3, TOK_RPAREN);
    return ret;
}
#undef RESTORE

PARSER(DirectAbstractDeclarator)
{
    Node *ret, *node;
    Token *tok;

    node = NULL;

    if ((tok = expect(state, TOK_LPAREN))) {
        MKRETT(NODE_DIRECT_ABSTRACT_DECLARATOR, 2);
#define RESTORE { pushNode(state, ret); freeNode(ret); goto pdadrestore; }
        REQUIREPR(0, AbstractDeclarator);
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
OPT(AbstractDeclarator)

PARSER(AbstractDeclarator)
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

PARSER(TypeName)
{
    Node *ret, *node;

    if (!(node = parseSpecifierQualifierList(state, parent))) return NULL;

    MKRETN(NODE_TYPE_NAME, 2);
    REQUIREP(1, AbstractDeclaratorOpt);
    return ret;
}

COMMA_LIST(IdentifierList, NODE_IDENTIFIER_LIST, Identifier)

PARSER(ParameterDeclaration)
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

COMMA_LIST(ParameterList, NODE_PARAMETER_LIST, ParameterDeclaration)

PARSER(ParameterTypeList)
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
OPT(ParameterTypeList)

CONCAT_LIST(TypeQualifierList, NODE_TYPE_QUALIFIER_LIST, TypeQualifier)

PARSER(PointerOpt)
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

#define RESTORE { RESTOREN(); return NULL; }
static Node *parseDirectDeclaratorA1(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_DECLARATOR, 5);
    REQUIRETR(1, TOK_LBRACKET);
    REQUIREPR(2, TypeQualifierListOpt);
    REQUIREPR(3, AssignmentExpressionOpt);
    REQUIRETR(4, TOK_RBRACKET);
    return ret;
}

static Node *parseDirectDeclaratorA2(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_DECLARATOR, 6);
    REQUIRETR(1, TOK_LBRACKET);
    REQUIRETR(2, TOK_static);
    REQUIREPR(3, TypeQualifierListOpt);
    REQUIREPR(4, AssignmentExpression);
    REQUIRETR(5, TOK_RBRACKET);
    return ret;
}

static Node *parseDirectDeclaratorA3(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_DECLARATOR, 6);
    REQUIRETR(1, TOK_LBRACKET);
    REQUIREPR(2, TypeQualifierList);
    REQUIRETR(3, TOK_static);
    REQUIREPR(4, AssignmentExpression);
    REQUIRETR(5, TOK_RBRACKET);
    return ret;
}

static Node *parseDirectDeclaratorA4(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_DECLARATOR, 5);
    REQUIRETR(1, TOK_LBRACKET);
    REQUIREPR(2, TypeQualifierListOpt);
    REQUIRETR(3, TOK_STAR);
    REQUIRETR(4, TOK_RBRACKET);
    return ret;
}

static Node *parseDirectDeclaratorF1(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_DECLARATOR, 4);
    REQUIRETR(1, TOK_LPAREN);
    REQUIREPR(2, ParameterTypeList);
    REQUIRETR(3, TOK_RPAREN);
    return ret;
}

static Node *parseDirectDeclaratorF2(ParseState *state, Node *parent, Node *node)
{
    Node *ret;
    MKRETN(NODE_DIRECT_DECLARATOR, 4);
    REQUIRETR(1, TOK_LPAREN);
    REQUIREPR(2, IdentifierListOpt);
    REQUIRETR(3, TOK_RPAREN);
    return ret;
}
#undef RESTORE

PARSER(DirectDeclarator)
{
    Node *ret, *node;
    Token *tok;

    if ((tok = expect(state, TOK_ID))) {
        MKRETT(NODE_DIRECT_DECLARATOR, 0);
        node = ret;

    } else if ((tok = expect(state, TOK_LPAREN))) {
        MKRETT(NODE_DIRECT_DECLARATOR, 2);
        REQUIREP(0, Declarator);
        REQUIRET(1, TOK_RPAREN);
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

PARSER(Declarator)
{
    Node *ret, *node;

    if (!(node = parsePointerOpt(state, parent))) return NULL;

    MKRETN(NODE_DECLARATOR, 2);
    REQUIREP(1, DirectDeclarator);

    return ret;
}

PARSER(AlignmentSpecifier)
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

PARSER(FunctionSpecifier)
{
    Node *ret;

    if ((ret = expectN(state, parent, TOK_inline)) ||
        (ret = expectN(state, parent, TOK__Noreturn))) {
        ret->type = NODE_FUNCTION_SPECIFIER;
        return ret;
    }

    return NULL;
}

PARSER(TypeQualifier)
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

PARSER(AtomicTypeSpecifier)
{
    Node *ret;
    Token *tok;

    if (!(tok = expect(state, TOK__Atomic))) return NULL;

    MKRETT(NODE_ATOMIC_TYPE_SPECIFIER, 3);
    REQUIRET(0, TOK_LPAREN);
    REQUIREP(1, TypeName);
    REQUIRET(2, TOK_RPAREN);

    return ret;
}

PARSER(Enumerator)
{
    Node *ret, *node, *node2;

    if (!(node = parseIdentifier(state, parent))) return NULL;

    if ((node2 = expectN(state, parent, TOK_ASG))) {
        MKRETN2(NODE_ENUMERATOR, 3);
        REQUIREPO(2, ConditionalExpression, { RESTOREN(); return node; });
        return ret;
    }

    return node;
}

COMMA_LIST(EnumeratorList, NODE_ENUMERATOR_LIST, Enumerator)

PARSER(EnumSpecifier)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_enum))) {
        MKRETT(NODE_ENUM_SPECIFIER, 5);
        REQUIREP(0, IdentifierOpt);

        if (ret->children[0]->tok) {
            /* the rest is optional */
            ret->children[1] = expectN(state, ret, TOK_LBRACE);

        } else {
            /* the rest is mandatory */
            REQUIRET(1, TOK_LBRACE);

        }

        if (ret->children[1]) {
            REQUIREP(2, EnumeratorList);
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

PARSER(StructDeclarator)
{
    Node *ret, *node, *node2;
    Token *tok;

    if ((node = parseDeclarator(state, parent))) {
        if ((node2 = expectN(state, parent, TOK_COLON))) {
            MKRETN2(NODE_BITFIELD_DECLARATOR, 3);
            REQUIREPO(2, Constant, { RESTOREN(); return node; });
            return ret;
        }
        return node;

    } else if ((tok = expect(state, TOK_COLON))) {
        MKRETT(NODE_BITFIELD_PADDING, 1);
        REQUIREP(0, Constant);
        return ret;

    }

    return NULL;
}

COMMA_LIST(StructDeclaratorList, NODE_STRUCT_DECLARATOR_LIST, StructDeclarator)

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
            MKRETN2(NODE_DECORATED_SPECIFIER_QUALIFIER_LIST, 2);
        }
    }

    FREE_BUFFER(buf);
    return ret;
}
PARSER_LIST(SpecifierQualifierList)

PARSER(StructDeclaration)
{
    Node *ret, *node;

    if ((node = parseSpecifierQualifierList(state, parent))) {
        MKRETN(NODE_STRUCT_DECLARATION, 3);
        REQUIREP(1, StructDeclaratorListOpt);
        REQUIRET(2, TOK_SEMICOLON);
        return ret;
    }

    if ((ret = parseStaticAssertDeclaration(state, parent))) return ret;

    return NULL;
}

CONCAT_LIST(StructDeclarationList, NODE_STRUCT_DECLARATION_LIST, StructDeclaration)

PARSER(StructOrUnionSpecifier)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_struct)) ||
        (tok = expect(state, TOK_union))) {
        MKRETT(NODE_STRUCT_OR_UNION_SPECIFIER, 4);
        REQUIREP(0, IdentifierOpt);
        if (ret->children[0]->tok) {
            /* the rest is optional */
            ret->children[1] = expectN(state, ret, TOK_LBRACE);

        } else {
            /* the rest is mandatory */
            REQUIRET(1, TOK_LBRACE);

        }

        if (ret->children[1]) {
            REQUIREP(2, StructDeclarationList);
            REQUIRET(3, TOK_RBRACE);
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

PARSER(StorageClassSpecifier)
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

PARSER(InitDeclarator)
{
    Node *ret, *node, *node2;

    if (!(node = parseDeclarator(state, parent))) return NULL;

    if ((node2 = expectN(state, parent, TOK_ASG))) {
        MKRETN2(NODE_INIT_DECLARATOR, 3);
        REQUIREPO(2, Initializer, { RESTOREN(); return node; });
        return ret;
    }

    return node;
}

COMMA_LIST(InitDeclaratorList, NODE_INIT_DECLARATOR_LIST, InitDeclarator)

PARSER(DeclarationSpecifiers)
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
        MKRETN2(NODE_DECORATED_DECLARATION_SPECIFIERS, 2);
    }

    FREE_BUFFER(buf);
    return ret;
}

PARSER(Declaration)
{
    Node *ret, *node, *node2;

    if ((node = parseDeclarationDecoratorList(state, parent))) {
        if ((node2 = parseDeclaration(state, parent))) {
            MKRETN2(NODE_DECORATED_DECLARATION, 2);
            return ret;
        } else if ((node2 = expectN(state, parent, TOK_SEMICOLON))) {
            MKRETN2(NODE_DECORATION_DECLARATION, 2);
            return ret;
        }

        pushNode(state, node);
        freeNode(node);
        return NULL;
    }

    if ((node = parseDeclarationSpecifiers(state, parent))) {
        MKRETN(NODE_DECLARATION, 3);
        REQUIREP(1, InitDeclaratorListOpt);
        REQUIRET(2, TOK_SEMICOLON);
        return ret;
    }

    return parseStaticAssertDeclaration(state, parent);
}

/***************************************************************
 * STATEMENTS                                                  *
 ***************************************************************/
PARSER(Statement);

PARSER(JumpStatement)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_goto))) {
        MKRETT(NODE_GOTO_STATEMENT, 2);
        REQUIREP(0, Identifier);
        REQUIRET(1, TOK_SEMICOLON);
        return ret;
    }

    if ((tok = expect(state, TOK_continue))) {
        MKRETT(NODE_CONTINUE_STATEMENT, 1);
        REQUIRET(0, TOK_SEMICOLON);
        return ret;
    }

    if ((tok = expect(state, TOK_break))) {
        MKRETT(NODE_BREAK_STATEMENT, 1);
        REQUIRET(0, TOK_SEMICOLON);
        return ret;
    }

    if ((tok = expect(state, TOK_return))) {
        MKRETT(NODE_RETURN_STATEMENT, 2);
        REQUIREP(0, ExpressionOpt);
        REQUIRET(1, TOK_SEMICOLON);
        return ret;
    }

    return NULL;
}

PARSER(ForInitializer)
{
    Node *ret, *node;

    /* declaration | expressionopt ; */
    if ((ret = parseDeclaration(state, parent))) return ret;

    if ((node = parseExpressionOpt(state, parent))) {
        MKRETN(NODE_FOR_INITIALIZER, 2);
        REQUIRET(1, TOK_SEMICOLON);
        return ret;
    }

    return NULL;
}

PARSER(IterationStatement)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_while))) {
        MKRETT(NODE_WHILE_STATEMENT, 4);
        REQUIRET(0, TOK_LPAREN);
        REQUIREP(1, Expression);
        REQUIRET(2, TOK_RPAREN);
        REQUIREP(3, Statement);
        return ret;
    }

    if ((tok = expect(state, TOK_do))) {
        MKRETT(NODE_DO_WHILE_STATEMENT, 6);
        REQUIREP(0, Statement);
        REQUIRET(1, TOK_while);
        REQUIRET(2, TOK_LPAREN);
        REQUIREP(3, Expression);
        REQUIRET(4, TOK_RPAREN);
        REQUIRET(5, TOK_SEMICOLON);
        return ret;
    }

    if ((tok = expect(state, TOK_for))) {
        MKRETT(NODE_FOR_STATEMENT, 7);
        REQUIRET(0, TOK_LPAREN);
        REQUIREP(1, ForInitializer);
        REQUIREP(2, ExpressionOpt);
        REQUIRET(3, TOK_SEMICOLON);
        REQUIREP(4, ExpressionOpt);
        REQUIRET(5, TOK_RPAREN);
        REQUIREP(6, Statement);
        return ret;
    }

    return NULL;
}

PARSER(SelectionStatement)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_if))) {
        MKRETT(NODE_IF_STATEMENT, 6);
        REQUIRET(0, TOK_LPAREN);
        REQUIREP(1, Expression);
        REQUIRET(2, TOK_RPAREN);
        REQUIREP(3, Statement);

        /* optional else clause */
        if ((ret->children[4] = expectN(state, ret, TOK_else)))
            REQUIREP(5, Statement);

        return ret;
    }

    if ((tok = expect(state, TOK_switch))) {
        MKRETT(NODE_SWITCH_STATEMENT, 4);
        REQUIRET(0, TOK_LPAREN);
        REQUIREP(1, Expression);
        REQUIRET(2, TOK_RPAREN);
        REQUIREP(3, Statement);
        return ret;
    }

    return NULL;
}

PARSER(ExpressionStatement)
{
    Node *ret, *node;

    if ((node = parseExpressionOpt(state, parent))) {
        MKRETN(NODE_EXPRESSION_STATEMENT, 2);
        REQUIRET(1, TOK_SEMICOLON);
        return ret;
    }

    return NULL;
}

PARSER(BlockItem)
{
    Node *ret;
    if ((ret = parseDeclaration(state, parent))) return ret;
    if ((ret = parseStatement(state, parent))) return ret;
    return NULL;
}

CONCAT_LIST(BlockItemList, NODE_BLOCK_ITEM_LIST, BlockItem)

PARSER(CompoundStatement)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_LBRACE))) {
        MKRETT(NODE_BLOCK, 2);
        REQUIREP(0, BlockItemListOpt);
        REQUIRET(1, TOK_RBRACE);
        return ret;
    }

    return NULL;
}

PARSER(LabeledStatement)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_ID))) {
        MKRETT(NODE_LABELED_STATEMENT, 2);
        REQUIRET(0, TOK_COLON);
        REQUIREP(1, Statement);
        return ret;
    }

    if ((tok = expect(state, TOK_case))) {
        MKRETT(NODE_CASE_STATEMENT, 3);
        REQUIREP(0, ConditionalExpression);
        REQUIRET(1, TOK_COLON);
        REQUIREP(2, Statement);
        return ret;
    }

    if ((tok = expect(state, TOK_default))) {
        MKRETT(NODE_DEFAULT_STATEMENT, 2);
        REQUIRET(0, TOK_COLON);
        REQUIREP(1, Statement);
        return ret;
    }

    return NULL;
}

PARSER(Statement)
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
CONCAT_LIST(DeclarationList, NODE_DECLARATION_LIST, Declaration)

PARSER(FunctionDefinition)
{
    Node *ret, *node;
    if (!(node = parseDeclarationSpecifiers(state, parent))) return NULL;
    MKRETN(NODE_FUNCTION_DEFINITION, 4);
    REQUIREP(1, Declarator);
    REQUIREP(2, DeclarationListOpt);
    REQUIREP(3, CompoundStatement);
    return ret;
}

PARSER(ExternalDeclaration)
{
    Node *ret;
    if ((ret = parseFunctionDefinition(state, parent))) return ret;
    if ((ret = parseDeclaration(state, parent))) return ret;
    return NULL;
}

CONCAT_LIST(TranslationUnit, NODE_TRANSLATION_UNIT, ExternalDeclaration)

PARSER(Top)
{
    Node *ret, *node;
    if (!(node = parseTranslationUnit(state, parent))) return NULL;
    MKRETN(NODE_FILE, 2);
    REQUIRET(1, TOK_TERM);
    return ret;
}

/***************************************************************
 * DECORATION                                                  *
 ***************************************************************/
PARSER(DecorationOpenOpt);

PARSER(DecorationSubDeclaration)
{
    Node *ret;
    Token *tok;

    if (!(tok = expect(state, TOK_LBRACE))) return NULL;

    MKRETT(NODE_DECORATION_SUB_DECLARATION, 2);
    REQUIREP(0, TranslationUnitOpt);
    REQUIRET(1, TOK_RBRACE);
    return ret;
}
OPT(DecorationSubDeclaration)

PARSER(DeclarationDecorator)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_DECORATION))) {
        MKRETT(NODE_DECLARATION_DECORATOR, 3);
        REQUIREP(0, DecorationName);
        REQUIREP(1, DecorationOpenOpt);
        REQUIREP(2, DecorationSubDeclarationOpt);
        return ret;
    }

    if ((tok = expect(state, TOK_OPEN_DECORATION))) {
        MKRETT(NODE_DECLARATION_DECORATOR, 4);
        REQUIREP(0, DecorationName);
        REQUIREP(1, DecorationOpenOpt);
        REQUIREP(2, DecorationSubDeclarationOpt);
        REQUIRET(3, TOK_CLOSE_DECORATION);
        return ret;
    }

    return NULL;
}

CONCAT_LIST(DeclarationDecoratorList, NODE_DECLARATION_DECORATOR_LIST, DeclarationDecorator)

PARSER(DecorationSubExpression)
{
    Node *ret, *node, *node2;
    Token *tok;

    if (!(tok = expect(state, TOK_LBRACE))) return NULL;

    MKRETT(NODE_DECORATION_SUB_EXPRESSION, 2);

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
        REQUIRET(1, TOK_RBRACE);
        return ret;
    }

    /* failure form! */
    pushNode(state, ret);
    freeNode(ret);
    return NULL;
}
OPT(DecorationSubExpression)

PARSER(ExpressionDecorator)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_DECORATION))) {
        MKRETT(NODE_EXPRESSION_DECORATOR, 3);
        REQUIREP(0, DecorationName);
        REQUIREP(1, DecorationOpenOpt);
        REQUIREP(2, DecorationSubExpressionOpt);
        return ret;
    }

    if ((tok = expect(state, TOK_OPEN_DECORATION))) {
        MKRETT(NODE_EXPRESSION_DECORATOR, 4);
        REQUIREP(0, DecorationName);
        REQUIREP(1, DecorationOpenOpt);
        REQUIREP(2, DecorationSubExpressionOpt);
        REQUIRET(3, TOK_CLOSE_DECORATION);
        return ret;
    }

    return NULL;
}

PARSER(DecorationOpExpression)
{
    Node *ret, *node, *node2;

    if (!(node = parseCastExpression(state, parent))) return NULL;

    while ((node2 = parseExpressionDecorator(state, parent))) {
        MKRETN2(NODE_DECORATION_OP, 3);
        REQUIREPO(2, CastExpression, { RESTOREN(); break; });
        node = ret;
    }

    return node;
}

PARSER(DecorationOpenCont)
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

    return ret;
}

PARSER(DecorationOpen)
{
    Node *ret;
    Token *tok;

    if ((tok = expect(state, TOK_LPAREN))) {
        MKRETT(NODE_DECORATION_OPEN, 2);
        REQUIREP(0, DecorationOpenCont);
        REQUIRET(1, TOK_RPAREN);
        return ret;
    }

    return NULL;
}
OPT(DecorationOpen)


/***************************************************************
 * ENTRY POINT                                                 *
 ***************************************************************/
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
        INIT_BUFFER(ebuf);

        while (BUFFER_SPACE(ebuf) <= 29 + 2*3*sizeof(int))
            EXPAND_BUFFER(ebuf);
        ebuf.bufused += sprintf(ebuf.buf + ebuf.bufused,
                                "At line %d column %d\n expected: ", (int) pState.el, (int) pState.ec);

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
