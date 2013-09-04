
#include "string.h"

#include "unparse.h"

#include "scan.h"

#include "parse.h"

/* convert a token type to a name */
 const char *tokenName(int n)
{
    switch (n) {
case TOK_KEY_FIRST: return "KEY_FIRST";
case TOK_auto: return "auto";
case TOK_break: return "break";
case TOK_case: return "case";
case TOK_char: return "char";
case TOK_const: return "const";
case TOK_continue: return "continue";
case TOK_default: return "default";
case TOK_do: return "do";
case TOK_double: return "double";
case TOK_else: return "else";
case TOK_enum: return "enum";
case TOK_extern: return "extern";
case TOK_float: return "float";
case TOK_for: return "for";
case TOK_goto: return "goto";
case TOK_if: return "if";
case TOK_inline: return "inline";
case TOK_int: return "int";
case TOK_long: return "long";
case TOK_register: return "register";
case TOK_restrict: return "restrict";
case TOK_return: return "return";
case TOK_short: return "short";
case TOK_signed: return "signed";
case TOK_sizeof: return "sizeof";
case TOK_static: return "static";
case TOK_struct: return "struct";
case TOK_switch: return "switch";
case TOK_typedef: return "typedef";
case TOK_union: return "union";
case TOK_unsigned: return "unsigned";
case TOK_void: return "void";
case TOK_volatile: return "volatile";
case TOK_while: return "while";
case TOK__Alignas: return "_Alignas";
case TOK__Alignof: return "_Alignof";
case TOK__Atomic: return "_Atomic";
case TOK__Bool: return "_Bool";
case TOK__Complex: return "_Complex";
case TOK__Generic: return "_Generic";
case TOK__Imaginary: return "_Imaginary";
case TOK__Noreturn: return "_Noreturn";
case TOK__Static_assert: return "_Static_assert";
case TOK__Thread_local: return "_Thread_local";
case TOK_KEY_LAST: return "KEY_LAST";
case TOK_ID: return "ID";
case TOK_LITERAL_FIRST: return "LITERAL_FIRST";
case TOK_INT_LITERAL: return "INT_LITERAL";
case TOK_FLOAT_LITERAL: return "FLOAT_LITERAL";
case TOK_CHAR_LITERAL: return "CHAR_LITERAL";
case TOK_STR_LITERAL: return "STR_LITERAL";
case TOK_LITERAL_LAST: return "LITERAL_LAST";
case TOK_PUNC_FIRST: return "PUNC_FIRST";
case TOK_LBRACKET: return "LBRACKET";
case TOK_RBRACKET: return "RBRACKET";
case TOK_LPAREN: return "LPAREN";
case TOK_RPAREN: return "RPAREN";
case TOK_LBRACE: return "LBRACE";
case TOK_RBRACE: return "RBRACE";
case TOK_DOT: return "DOT";
case TOK_ARROW: return "ARROW";
case TOK_PLUSPLUS: return "PLUSPLUS";
case TOK_MINUSMINUS: return "MINUSMINUS";
case TOK_AND: return "AND";
case TOK_STAR: return "STAR";
case TOK_PLUS: return "PLUS";
case TOK_MINUS: return "MINUS";
case TOK_BNOT: return "BNOT";
case TOK_NOT: return "NOT";
case TOK_DIV: return "DIV";
case TOK_MOD: return "MOD";
case TOK_SHL: return "SHL";
case TOK_SHR: return "SHR";
case TOK_LT: return "LT";
case TOK_GT: return "GT";
case TOK_LTE: return "LTE";
case TOK_GTE: return "GTE";
case TOK_EQ: return "EQ";
case TOK_NEQ: return "NEQ";
case TOK_BXOR: return "BXOR";
case TOK_BOR: return "BOR";
case TOK_ANDAND: return "ANDAND";
case TOK_OROR: return "OROR";
case TOK_HOOK: return "HOOK";
case TOK_COLON: return "COLON";
case TOK_SEMICOLON: return "SEMICOLON";
case TOK_DOTDOTDOT: return "DOTDOTDOT";
case TOK_ASG_START: return "ASG_START";
case TOK_ASG: return "ASG";
case TOK_MULASG: return "MULASG";
case TOK_DIVASG: return "DIVASG";
case TOK_MODASG: return "MODASG";
case TOK_ADDASG: return "ADDASG";
case TOK_SUBASG: return "SUBASG";
case TOK_SHLASG: return "SHLASG";
case TOK_SHRASG: return "SHRASG";
case TOK_BANDASG: return "BANDASG";
case TOK_BXORASG: return "BXORASG";
case TOK_BORASG: return "BORASG";
case TOK_ASG_END: return "ASG_END";
case TOK_COMMA: return "COMMA";
case TOK_HASH: return "HASH";
case TOK_HASHHASH: return "HASHHASH";
/* exc additions */
case TOK_DECORATION: return "DECORATION";
case TOK_OPEN_DECORATION: return "OPEN_DECORATION";
case TOK_CLOSE_DECORATION: return "CLOSE_DECORATION";
/* /exc additions */
case TOK_PUNC_UNKNOWN: return "PUNC_UNKNOWN";
case TOK_PUNC_LAST: return "PUNC_LAST";
case TOK_TERM: return "TERM";
        default: return "???";
    }
}
/* convert a node type (numeric) to a name */
 const char *nodeName(int n)
{
    switch (n) {
/* Constants/literals */
case NODE_NIL: return "NIL";
case NODE_TOK: return "TOK";
case NODE_ID: return "ID";
case NODE_DECORATION_NAME: return "DECORATION_NAME";
case NODE_INT_LITERAL: return "INT_LITERAL";
case NODE_FLOAT_LITERAL: return "FLOAT_LITERAL";
case NODE_CHAR_LITERAL: return "CHAR_LITERAL";
case NODE_STR_LITERAL: return "STR_LITERAL";
/* Expressions */
case NODE_PAREN: return "PAREN";
case NODE_GENERIC_SELECTION: return "GENERIC_SELECTION";
case NODE_GENERIC_ASSOC_LIST: return "GENERIC_ASSOC_LIST";
case NODE_GENERIC_ASSOCIATION: return "GENERIC_ASSOCIATION";
case NODE_GENERIC_ASSOCIATION_DEFAULT: return "GENERIC_ASSOCIATION_DEFAULT";
case NODE_INDEX: return "INDEX";
case NODE_CALL: return "CALL";
case NODE_MEMBER_DOT: return "MEMBER_DOT";
case NODE_MEMBER_ARROW: return "MEMBER_ARROW";
case NODE_POSTINC: return "POSTINC";
case NODE_POSTDEC: return "POSTDEC";
case NODE_COMPOUND_LITERAL: return "COMPOUND_LITERAL";
case NODE_ARGUMENT_EXPRESSION_LIST: return "ARGUMENT_EXPRESSION_LIST";
case NODE_PREINC: return "PREINC";
case NODE_PREDEC: return "PREDEC";
case NODE_ADDROF: return "ADDROF";
case NODE_DEREF: return "DEREF";
case NODE_POSITIVE: return "POSITIVE";
case NODE_NEGATIVE: return "NEGATIVE";
case NODE_BNOT: return "BNOT";
case NODE_NOT: return "NOT";
case NODE_SIZEOF_EXP: return "SIZEOF_EXP";
case NODE_SIZEOF_TYPE: return "SIZEOF_TYPE";
case NODE_ALIGNOF: return "ALIGNOF";
case NODE_CAST: return "CAST";
case NODE_MUL: return "MUL";
case NODE_DIV: return "DIV";
case NODE_MOD: return "MOD";
case NODE_ADD: return "ADD";
case NODE_SUB: return "SUB";
case NODE_SHL: return "SHL";
case NODE_SHR: return "SHR";
case NODE_LT: return "LT";
case NODE_GT: return "GT";
case NODE_LTE: return "LTE";
case NODE_GTE: return "GTE";
case NODE_EQ: return "EQ";
case NODE_NEQ: return "NEQ";
case NODE_BAND: return "BAND";
case NODE_BXOR: return "BXOR";
case NODE_BOR: return "BOR";
case NODE_AND: return "AND";
case NODE_OR: return "OR";
case NODE_CONDITIONAL: return "CONDITIONAL";
case NODE_ASG: return "ASG";
case NODE_RASG: return "RASG";
case NODE_COMMA: return "COMMA";
/* Declarations */
case NODE_DECLARATION: return "DECLARATION";
case NODE_DECLARATION_SPECIFIERS: return "DECLARATION_SPECIFIERS";
case NODE_INIT_DECLARATOR_LIST: return "INIT_DECLARATOR_LIST";
case NODE_INIT_DECLARATOR: return "INIT_DECLARATOR";
case NODE_STORAGE_CLASS_SPECIFIER: return "STORAGE_CLASS_SPECIFIER";
case NODE_TYPE_SPECIFIER: return "TYPE_SPECIFIER";
case NODE_STRUCT_OR_UNION_SPECIFIER: return "STRUCT_OR_UNION_SPECIFIER";
case NODE_STRUCT_DECLARATION_LIST: return "STRUCT_DECLARATION_LIST";
case NODE_STRUCT_DECLARATION: return "STRUCT_DECLARATION";
case NODE_SPECIFIER_QUALIFIER_LIST: return "SPECIFIER_QUALIFIER_LIST";
case NODE_STRUCT_DECLARATOR_LIST: return "STRUCT_DECLARATOR_LIST";
case NODE_BITFIELD_DECLARATOR: return "BITFIELD_DECLARATOR";
case NODE_BITFIELD_PADDING: return "BITFIELD_PADDING";
case NODE_ENUM_SPECIFIER: return "ENUM_SPECIFIER";
case NODE_ENUMERATOR_LIST: return "ENUMERATOR_LIST";
case NODE_ENUMERATOR: return "ENUMERATOR";
case NODE_ATOMIC_TYPE_SPECIFIER: return "ATOMIC_TYPE_SPECIFIER";
case NODE_TYPE_QUALIFIER: return "TYPE_QUALIFIER";
case NODE_FUNCTION_SPECIFIER: return "FUNCTION_SPECIFIER";
case NODE_ALIGNMENT_SPECIFIER: return "ALIGNMENT_SPECIFIER";
case NODE_DECLARATOR: return "DECLARATOR";
case NODE_DIRECT_DECLARATOR: return "DIRECT_DECLARATOR";
case NODE_POINTER: return "POINTER";
case NODE_TYPE_QUALIFIER_LIST: return "TYPE_QUALIFIER_LIST";
case NODE_PARAMETER_TYPE_LIST: return "PARAMETER_TYPE_LIST";
case NODE_PARAMETER_LIST: return "PARAMETER_LIST";
case NODE_PARAMETER_DECLARATION: return "PARAMETER_DECLARATION";
case NODE_IDENTIFIER_LIST: return "IDENTIFIER_LIST";
case NODE_TYPE_NAME: return "TYPE_NAME";
case NODE_ABSTRACT_DECLARATOR: return "ABSTRACT_DECLARATOR";
case NODE_DIRECT_ABSTRACT_DECLARATOR: return "DIRECT_ABSTRACT_DECLARATOR";
case NODE_INITIALIZER: return "INITIALIZER";
case NODE_DESIGNATION_INITIALIZER: return "DESIGNATION_INITIALIZER";
case NODE_INITIALIZER_LIST: return "INITIALIZER_LIST";
case NODE_DESIGNATION: return "DESIGNATION";
case NODE_DESIGNATOR_LIST: return "DESIGNATOR_LIST";
case NODE_DESIGNATOR: return "DESIGNATOR";
case NODE_STATIC_ASSERT_DECLARATION: return "STATIC_ASSERT_DECLARATION";
/* Statements */
case NODE_LABELED_STATEMENT: return "LABELED_STATEMENT";
case NODE_CASE_STATEMENT: return "CASE_STATEMENT";
case NODE_DEFAULT_STATEMENT: return "DEFAULT_STATEMENT";
case NODE_BLOCK: return "BLOCK";
case NODE_BLOCK_ITEM_LIST: return "BLOCK_ITEM_LIST";
case NODE_EXPRESSION_STATEMENT: return "EXPRESSION_STATEMENT";
case NODE_IF_STATEMENT: return "IF_STATEMENT";
case NODE_SWITCH_STATEMENT: return "SWITCH_STATEMENT";
case NODE_WHILE_STATEMENT: return "WHILE_STATEMENT";
case NODE_DO_WHILE_STATEMENT: return "DO_WHILE_STATEMENT";
case NODE_FOR_STATEMENT: return "FOR_STATEMENT";
case NODE_FOR_INITIALIZER: return "FOR_INITIALIZER";
case NODE_GOTO_STATEMENT: return "GOTO_STATEMENT";
case NODE_CONTINUE_STATEMENT: return "CONTINUE_STATEMENT";
case NODE_BREAK_STATEMENT: return "BREAK_STATEMENT";
case NODE_RETURN_STATEMENT: return "RETURN_STATEMENT";
/* Translation unit/top level */
case NODE_TRANSLATION_UNIT: return "TRANSLATION_UNIT";
case NODE_FUNCTION_DEFINITION: return "FUNCTION_DEFINITION";
case NODE_DECLARATION_LIST: return "DECLARATION_LIST";
case NODE_FILE: return "FILE";
case NODE_TERM: return "TERM";
/* Decoration */
case NODE_DECORATION_OPEN: return "DECORATION_OPEN";
case NODE_DECORATION_OPEN_CONT: return "DECORATION_OPEN_CONT";
case NODE_DECORATION_OP: return "DECORATION_OP";
case NODE_EXPRESSION_DECORATOR: return "EXPRESSION_DECORATOR";
case NODE_DECORATION_SUB_EXPRESSION: return "DECORATION_SUB_EXPRESSION";
case NODE_DECORATED_DECLARATION: return "DECORATED_DECLARATION";
case NODE_DECORATED_FUNCTION_DEFINITION: return "DECORATED_FUNCTION_DEFINITION";
case NODE_DECORATION_DECLARATION: return "DECORATION_DECLARATION";
case NODE_DECORATED_DECLARATION_SPECIFIERS: return "DECORATED_DECLARATION_SPECIFIERS";
case NODE_DECORATED_SPECIFIER_QUALIFIER_LIST: return "DECORATED_SPECIFIER_QUALIFIER_LIST";
case NODE_DECLARATION_DECORATOR_LIST: return "DECLARATION_DECORATOR_LIST";
case NODE_DECLARATION_DECORATOR: return "DECLARATION_DECORATOR";
case NODE_DECORATION_SUB_DECLARATION: return "DECORATION_SUB_DECLARATION";
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
