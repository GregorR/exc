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

#define REQUIREP(chn, parser) do { \
if (!(ret->children[chn] = parser(state, ret))) { \
    pushNode(state, ret); \
    freeNode(ret); \
    return NULL; \
} \
} while(0)

#define REQUIRET(chn, toktype) do { \
if (!(ret->children[chn] = expectN(state, ret, toktype))) { \
    pushNode(state, ret); \
    freeNode(ret); \
    return NULL; \
} \
} while (0)

/***************************************************************
 * IDENTIFIERS/CONSTANTS                                       *
 ***************************************************************/
static Node *parseIdentifierOpt(ParseState *state, Node *parent)
{
    Node *ret;
    Token *tok;

    ret = newNode(parent, NODE_ID, NULL, 0);
    if ((tok = expect(state, TOK_ID)))
        ret->tok = tok;

    return ret;
}

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

#define COMMA_LIST(parserName, genNode, nextParser) \
static Node *parserName(ParseState *state, Node *parent) \
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
}

/***************************************************************
 * EXPRESSIONS                                                 *
 ***************************************************************/
static Node *parsePrimaryExpression(ParseState *state, Node *parent);
static Node *parseCastExpression(ParseState *state, Node *parent);
static Node *parseAssignmentExpression(ParseState *state, Node *parent);
static Node *parseExpression(ParseState *state, Node *parent);

COMMA_LIST(parseArgumentExpressionList, NODE_ARGUMENT_EXPRESSION_LIST, parseAssignmentExpression)

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
        if ((node2 = expectN(state, parent, TOK_LBRACKET))) {
            MKRETN2(NODE_INDEX, 4);
            REQUIREP(2, parseExpression);
            REQUIRET(3, TOK_RBRACKET);
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, parent, TOK_LPAREN))) {
            MKRETN2(NODE_CALL, 4);
            REQUIREP(2, parseArgumentExpressionList);
            REQUIRET(3, TOK_RPAREN);
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, parent, TOK_DOT))) {
            MKRETN2(NODE_MEMBER_DOT, 3);
            REQUIRET(2, TOK_ID);
            node = ret;
            continue;
        }

        if ((node2 = expectN(state, parent, TOK_ARROW))) {
            MKRETN2(NODE_MEMBER_ARROW, 3);
            REQUIRET(2, TOK_ID);
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
        REQUIREP(0, parseUnaryExpression); \
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
#define BINARY_OP_END() \
            break; \
        } \
        return node; \
    }
#define BINARY_OP(reqTok, genNode, nextNode) \
    if ((node2 = expectN(state, parent, reqTok))) { \
        MKRETN2(genNode, 3); \
        REQUIREP(2, nextNode); \
        continue; \
    } \

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
        REQUIREP(2, parseExpression);
        REQUIRET(3, TOK_COLON);
        REQUIREP(4, parseConditionalExpression);
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
        if (tok->type <= TOK_ASG_START && tok->type >= TOK_ASG_END) {
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
        REQUIREP(2, parseAssignmentExpression);
        return ret;
    }

    return parseConditionalExpression(state, parent);
}

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

    if ((ret = expectN(state, parent, TOK_ID))) {
        ret->type = NODE_ID;
        return ret;
    }
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

static Node *parseAlignmentSpecifier(ParseState *state, Node *parent)
{
    Node *ret, *node;
    Token *tok;

    if (!(tok = expect(state, TOK__Alignas))) return NULL;

    MKRETT(NODE_ALIGNMENT_SPECIFIER, 3);
    REQUIRET(0, TOK_LPAREN);

    if (!(node = parseTypeName(state, ret)) &&
        !(node = parseConstantExpression(state, ret))) {
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
        REQUIREP(2, parseConditionalExpression);
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
            REQUIREP(2, parseConstant);
            return ret;
        }
        return ret;

    } else if ((tok = expect(state, TOK_COLON))) {
        MKRETT(NODE_BITFIELD_PADDING, 1);
        REQUIREP(0, parseConstant);
        return ret;

    }

    return NULL;
}

COMMA_LIST(parseStructDeclaratorList, NODE_STRUCT_DECLARATOR_LIST, parseStructDeclarator)

static Node *parseSpecifierQualifierList(ParseState *state, Node *parent)
{
    Node *ret;
    struct Buffer_Nodep buf;
    size_t i;

    INIT_BUFFER(buf);

    while ((ret = parseTypeSpecifier(state, parent)) ||
           (ret = parseTypeQualifier(state, parent))) {
        WRITE_ONE_BUFFER(buf, ret);
    }

    if (buf.bufused == 0) {
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

static Node *parseStructDeclaration(ParseState *state, Node *parent)
{
    Node *ret, *node;

    if ((node = parseSpecifierQualifierList(state, parent))) {
        MKRETN(NODE_STRUCT_DECLARATION, 3);
        REQUIREP(1, parseStructDeclaratorList);
        REQUIRET(2, TOK_SEMICOLON);
        return ret;
    }

    if ((ret = parseStaticAssertDeclaration(state, parent))) return ret;

    return NULL;
}

static Node *parseStructDeclarationList(ParseState *state, Node *parent)
{
    Node *ret;
    struct Buffer_Nodep buf;
    size_t i;

    INIT_BUFFER(buf);

    while ((ret = parseStructDeclaration(state, parent))) {
        WRITE_ONE_BUFFER(buf, ret);
    }

    ret = newNode(parent, NODE_STRUCT_DECLARATION_LIST, NULL, buf.bufused);
    for (i = 0; i < buf.bufused; i++) {
        ret->children[i] = buf.buf[i];
        buf.buf[i]->parent = ret;
    }

    FREE_BUFFER(buf);

    return ret;
}

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
        REQUIREP(2, parseInitializer);
        return ret;
    }

    return node;
}

COMMA_LIST(parseInitDeclaratorList, NODE_INIT_DECLARATOR_LIST, parseInitDeclarator)

static Node *parseInitDeclaratorListOpt(ParseState *state, Node *parent)
{
    Node *ret;
    if ((ret = parseInitDeclaratorList(state, parent))) return ret;
    return newNode(parent, NODE_INIT_DECLARATOR_LIST, NULL, 0);
}

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
