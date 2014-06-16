#ifndef EXC_src_parse
#define EXC_src_parse 1
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

#include "node.h"

#include "scan.h"


#include "stdio.h"

#include "stdlib.h"

#include "sys/types.h"


#include "buffer.h"


#line 26 "src/parse.exc"
BUFFER(Tokenp, Token *);


#line 28 "src/parse.exc"
 enum Nodes {
    NODE_FIRST = 0,


/* Constants/literals */
#line 2 "src/nodes.h"
NODE_NIL,

NODE_TOK,

NODE_ID,
NODE_DECORATION_NAME,

NODE_INT_LITERAL,
NODE_FLOAT_LITERAL,
NODE_CHAR_LITERAL,
NODE_STR_LITERAL,

/* Expressions */
NODE_PAREN,

NODE_GENERIC_SELECTION,
NODE_GENERIC_ASSOC_LIST,
NODE_GENERIC_ASSOCIATION,
NODE_GENERIC_ASSOCIATION_DEFAULT,

NODE_INDEX,
NODE_CALL,
NODE_MEMBER_DOT,
NODE_MEMBER_ARROW,
NODE_POSTINC,
NODE_POSTDEC,
NODE_COMPOUND_LITERAL,

NODE_ARGUMENT_EXPRESSION_LIST,

NODE_PREINC,
NODE_PREDEC,
NODE_ADDROF,
NODE_DEREF,
NODE_POSITIVE,
NODE_NEGATIVE,
NODE_BNOT,
NODE_NOT,
NODE_SIZEOF_EXP,
NODE_SIZEOF_TYPE,
NODE_ALIGNOF,

NODE_CAST,

NODE_MUL,
NODE_DIV,
NODE_MOD,

NODE_ADD,
NODE_SUB,

NODE_SHL,
NODE_SHR,

NODE_LT,
NODE_GT,
NODE_LTE,
NODE_GTE,

NODE_EQ,
NODE_NEQ,

NODE_BAND,

NODE_BXOR,

NODE_BOR,

NODE_AND,

NODE_OR,

NODE_CONDITIONAL,

NODE_ASG,
NODE_RASG,

NODE_COMMA,

/* Declarations */
NODE_DECLARATION,
NODE_DECLARATION_SPECIFIERS,
NODE_INIT_DECLARATOR_LIST,
NODE_INIT_DECLARATOR,
NODE_STORAGE_CLASS_SPECIFIER,
NODE_TYPE_SPECIFIER,
NODE_STRUCT_OR_UNION_SPECIFIER,
NODE_STRUCT_DECLARATION_LIST,
NODE_STRUCT_DECLARATION,
NODE_SPECIFIER_QUALIFIER_LIST,
NODE_STRUCT_DECLARATOR_LIST,
NODE_BITFIELD_DECLARATOR,
NODE_BITFIELD_PADDING,
NODE_ENUM_SPECIFIER,
NODE_ENUMERATOR_LIST,
NODE_ENUMERATOR,
NODE_ATOMIC_TYPE_SPECIFIER,
NODE_TYPE_QUALIFIER,
NODE_FUNCTION_SPECIFIER,
NODE_ALIGNMENT_SPECIFIER,
NODE_DECLARATOR,
NODE_DIRECT_DECLARATOR,
NODE_POINTER,
NODE_TYPE_QUALIFIER_LIST,
NODE_PARAMETER_TYPE_LIST,
NODE_PARAMETER_LIST,
NODE_PARAMETER_DECLARATION,
NODE_IDENTIFIER_LIST,
NODE_TYPE_NAME,
NODE_ABSTRACT_DECLARATOR,
NODE_DIRECT_ABSTRACT_DECLARATOR,
NODE_INITIALIZER,
NODE_DESIGNATION_INITIALIZER,
NODE_INITIALIZER_LIST,
NODE_DESIGNATION,
NODE_DESIGNATOR_LIST,
NODE_DESIGNATOR,
NODE_STATIC_ASSERT_DECLARATION,

/* Statements */
NODE_LABELED_STATEMENT,
NODE_CASE_STATEMENT,
NODE_DEFAULT_STATEMENT,
NODE_BLOCK,
NODE_BLOCK_ITEM_LIST,
NODE_EXPRESSION_STATEMENT,
NODE_IF_STATEMENT,
NODE_SWITCH_STATEMENT,
NODE_WHILE_STATEMENT,
NODE_DO_WHILE_STATEMENT,
NODE_FOR_STATEMENT,
NODE_FOR_INITIALIZER,
NODE_GOTO_STATEMENT,
NODE_CONTINUE_STATEMENT,
NODE_BREAK_STATEMENT,
NODE_RETURN_STATEMENT,

/* Translation unit/top level */
NODE_TRANSLATION_UNIT,
NODE_FUNCTION_DEFINITION,
NODE_DECLARATION_LIST,
NODE_FILE,
NODE_TERM,

/* Decoration */
NODE_DECORATION_OPEN,
NODE_DECORATION_OPEN_CONT,
NODE_DECORATION_OP,
NODE_EXPRESSION_DECORATOR,
NODE_DECORATION_SUB_EXPRESSION,
NODE_DECORATED_DECLARATION,
NODE_DECORATED_FUNCTION_DEFINITION,
NODE_DECORATION_DECLARATION,
NODE_DECORATED_DECLARATION_SPECIFIERS,
NODE_DECORATED_SPECIFIER_QUALIFIER_LIST,
NODE_DECLARATION_DECORATOR_LIST,
NODE_DECLARATION_DECORATOR,
NODE_DECORATION_SUB_DECLARATION,


#line 35 "src/parse.exc"
    NODE_LAST
};


#line 38 "src/parse.exc"
 typedef struct ParseState_ {
    ScanState *scanState;

    /* pushback tokens */
    struct Buffer_Tokenp buf;

    /* error state */
    size_t eidx, ef, el, ec;
    struct Buffer_int eexpected;
    int efound;
} ParseState;

/* prepare ret with a single node `node` */
/* prepare ret with nodes `node` and `node2` */
/* prepare ret with token `tok` */
/* shift the first child off of `ret` (usually to restore it before
 * pushNode(ret)) */






/* restore `node` from ret->children[0] and push/delete node */
/* require a parser succeed, else iffail 
 * This is INTENTIONALLY not a do{}while, to allow for breaks */






/* require a parser succeed else return NULL */






/* require a parser succeed else RESTORE */


/* require a token be found else iffail */






/* require a token be found else return NULL */






/* require a token be found else RESTORE */


/* type for a parser */

#line 215 "src/parse.exc"
 typedef Node *(*parser_func_t)(ParseState *, Node *);


/* make an optional parser based on a parser `name` */
/***************************************************************
 * IDENTIFIERS/CONSTANTS                                       *
 ***************************************************************/

#line 309 "src/parse.exc"
 Node *parseDecorationName(ParseState *state, Node *parent);


#line 338 "src/parse.exc"
 Node *parseIdentifier(ParseState *state, Node *parent);

#line 345 "src/parse.exc"
 Node *parseIdentifierOpt(ParseState *state, Node *parent);


#line 347 "src/parse.exc"
 Node *parseStrLiteralPart(ParseState *state, Node *parent);


#line 352 "src/parse.exc"
 Node *parseStrLiteralL(ParseState *state, Node *parent, int need1); 
#line 352 "src/parse.exc"
 Node *parseStrLiteral(ParseState *state, Node *parent); 
#line 352 "src/parse.exc"
 Node *parseStrLiteralOpt(ParseState *state, Node *parent);


#line 354 "src/parse.exc"
 Node *parseConstant(ParseState *state, Node *parent);


/***************************************************************
 * EXPRESSIONS                                                 *
 ***************************************************************/
 extern
#line 382 "src/parse.exc"
 Node *parsePrimaryExpression(ParseState *state, Node *parent);
 extern
#line 383 "src/parse.exc"
 Node *parseCastExpression(ParseState *state, Node *parent);
 extern
#line 384 "src/parse.exc"
 Node *parseAssignmentExpression(ParseState *state, Node *parent);
 extern
#line 385 "src/parse.exc"
 Node *parseExpression(ParseState *state, Node *parent);
 extern
#line 386 "src/parse.exc"
 Node *parseTypeName(ParseState *state, Node *parent);
 extern
#line 387 "src/parse.exc"
 Node *parseInitializerList(ParseState *state, Node *parent);
 extern
#line 388 "src/parse.exc"
 Node *parseDecorationOpExpression(ParseState *state, Node *parent);
 extern
#line 389 "src/parse.exc"
 Node *parseExpressionDecorator(ParseState *state, Node *parent);


#line 391 "src/parse.exc"
 Node *parseArgumentExpressionListL(ParseState *state, Node *parent, int need1); 
#line 391 "src/parse.exc"
 Node *parseArgumentExpressionList(ParseState *state, Node *parent); 
#line 391 "src/parse.exc"
 Node *parseArgumentExpressionListOpt(ParseState *state, Node *parent);


#line 393 "src/parse.exc"
 Node *parsePostfixExpression(ParseState *state, Node *parent);


#line 475 "src/parse.exc"
 Node *parseUnaryTypeExpression(ParseState *state, Node *parent);


#line 499 "src/parse.exc"
 Node *parseUnaryExpression(ParseState *state, Node *parent);


#line 530 "src/parse.exc"
 Node *parseCastTypeExpression(ParseState *state, Node *parent);


#line 546 "src/parse.exc"
 Node *parseCastExpression(ParseState *state, Node *parent);

#line 573 "src/parse.exc"
 Node *parseMultiplicativeExpression(ParseState *state, Node *parent);


#line 579 "src/parse.exc"
 Node *parseAdditiveExpression(ParseState *state, Node *parent);


#line 584 "src/parse.exc"
 Node *parseShiftExpression(ParseState *state, Node *parent);


#line 589 "src/parse.exc"
 Node *parseRelationalExpression(ParseState *state, Node *parent);


#line 596 "src/parse.exc"
 Node *parseEqualityExpression(ParseState *state, Node *parent);


#line 601 "src/parse.exc"
 Node *parseBAndExpression(ParseState *state, Node *parent);


#line 605 "src/parse.exc"
 Node *parseBXorExpression(ParseState *state, Node *parent);


#line 609 "src/parse.exc"
 Node *parseBOrExpression(ParseState *state, Node *parent);


#line 613 "src/parse.exc"
 Node *parseAndExpression(ParseState *state, Node *parent);


#line 617 "src/parse.exc"
 Node *parseOrExpression(ParseState *state, Node *parent);


#line 621 "src/parse.exc"
 Node *parseConditionalExpression(ParseState *state, Node *parent);


#line 640 "src/parse.exc"
 Node *parseAssignmentExpression(ParseState *state, Node *parent);

#line 667 "src/parse.exc"
 Node *parseAssignmentExpressionOpt(ParseState *state, Node *parent);


#line 669 "src/parse.exc"
 Node *parseExpression(ParseState *state, Node *parent);

#line 672 "src/parse.exc"
 Node *parseExpressionOpt(ParseState *state, Node *parent);


#line 674 "src/parse.exc"
 Node *parseGenericAssociation(ParseState *state, Node *parent);


#line 696 "src/parse.exc"
 Node *parseGenericAssocListL(ParseState *state, Node *parent, int need1); 
#line 696 "src/parse.exc"
 Node *parseGenericAssocList(ParseState *state, Node *parent); 
#line 696 "src/parse.exc"
 Node *parseGenericAssocListOpt(ParseState *state, Node *parent);


#line 698 "src/parse.exc"
 Node *parseGenericSelection(ParseState *state, Node *parent);


#line 716 "src/parse.exc"
 Node *parsePrimaryExpression(ParseState *state, Node *parent);

/***************************************************************
 * DECLARATIONS                                                *
 **************************************************************/
 extern
#line 741 "src/parse.exc"
 Node *parseDeclarator(ParseState *state, Node *parent);
 extern
#line 742 "src/parse.exc"
 Node *parseTypeQualifier(ParseState *state, Node *parent);
 extern
#line 743 "src/parse.exc"
 Node *parseAbstractDeclarator(ParseState *state, Node *parent);
 extern
#line 744 "src/parse.exc"
 Node *parsePointerOpt(ParseState *state, Node *parent);
 extern
#line 745 "src/parse.exc"
 Node *parseSpecifierQualifierList(ParseState *state, Node *parent);
 extern
#line 746 "src/parse.exc"
 Node *parseParameterTypeListOpt(ParseState *state, Node *parent);
 extern
#line 747 "src/parse.exc"
 Node *parseDeclarationSpecifiers(ParseState *state, Node *parent);
 extern
#line 748 "src/parse.exc"
 Node *parseTypeQualifierList(ParseState *state, Node *parent);
 extern
#line 749 "src/parse.exc"
 Node *parseTypeQualifierListOpt(ParseState *state, Node *parent);
 extern
#line 750 "src/parse.exc"
 Node *parseInitializer(ParseState *state, Node *parent);
 extern
#line 751 "src/parse.exc"
 Node *parseDeclarationDecoratorList(ParseState *state, Node *parent);


#line 757 "src/parse.exc"
 Node *parseStaticAssertDeclaration(ParseState *state, Node *parent);


#line 775 "src/parse.exc"
 Node *parseDesignator(ParseState *state, Node *parent);


#line 796 "src/parse.exc"
 Node *parseDesignatorListL(ParseState *state, Node *parent, int need1); 
#line 796 "src/parse.exc"
 Node *parseDesignatorList(ParseState *state, Node *parent); 
#line 796 "src/parse.exc"
 Node *parseDesignatorListOpt(ParseState *state, Node *parent);


#line 798 "src/parse.exc"
 Node *parseDesignation(ParseState *state, Node *parent);

#line 808 "src/parse.exc"
 Node *parseDesignationOpt(ParseState *state, Node *parent);


#line 810 "src/parse.exc"
 Node *parseDesignationInitializer(ParseState *state, Node *parent);


#line 822 "src/parse.exc"
 Node *parseInitializerListL(ParseState *state, Node *parent, int need1); 
#line 822 "src/parse.exc"
 Node *parseInitializerList(ParseState *state, Node *parent); 
#line 822 "src/parse.exc"
 Node *parseInitializerListOpt(ParseState *state, Node *parent);


#line 824 "src/parse.exc"
 Node *parseInitializer(ParseState *state, Node *parent);



#line 905 "src/parse.exc"
 Node *parseDirectAbstractDeclarator(ParseState *state, Node *parent);

#line 961 "src/parse.exc"
 Node *parseAbstractDeclaratorOpt(ParseState *state, Node *parent);


#line 963 "src/parse.exc"
 Node *parseAbstractDeclarator(ParseState *state, Node *parent);


#line 983 "src/parse.exc"
 Node *parseTypeName(ParseState *state, Node *parent);


#line 994 "src/parse.exc"
 Node *parseIdentifierListL(ParseState *state, Node *parent, int need1); 
#line 994 "src/parse.exc"
 Node *parseIdentifierList(ParseState *state, Node *parent); 
#line 994 "src/parse.exc"
 Node *parseIdentifierListOpt(ParseState *state, Node *parent);


#line 996 "src/parse.exc"
 Node *parseParameterDeclaration(ParseState *state, Node *parent);


#line 1019 "src/parse.exc"
 Node *parseParameterListL(ParseState *state, Node *parent, int need1); 
#line 1019 "src/parse.exc"
 Node *parseParameterList(ParseState *state, Node *parent); 
#line 1019 "src/parse.exc"
 Node *parseParameterListOpt(ParseState *state, Node *parent);


#line 1021 "src/parse.exc"
 Node *parseParameterTypeList(ParseState *state, Node *parent);

#line 1035 "src/parse.exc"
 Node *parseParameterTypeListOpt(ParseState *state, Node *parent);


#line 1037 "src/parse.exc"
 Node *parseTypeQualifierListL(ParseState *state, Node *parent, int need1); 
#line 1037 "src/parse.exc"
 Node *parseTypeQualifierList(ParseState *state, Node *parent); 
#line 1037 "src/parse.exc"
 Node *parseTypeQualifierListOpt(ParseState *state, Node *parent);


#line 1039 "src/parse.exc"
 Node *parsePointerOpt(ParseState *state, Node *parent);



#line 1139 "src/parse.exc"
 Node *parseDirectDeclarator(ParseState *state, Node *parent);


#line 1196 "src/parse.exc"
 Node *parseDeclarator(ParseState *state, Node *parent);


#line 1208 "src/parse.exc"
 Node *parseAlignmentSpecifier(ParseState *state, Node *parent);


#line 1231 "src/parse.exc"
 Node *parseFunctionSpecifier(ParseState *state, Node *parent);


#line 1244 "src/parse.exc"
 Node *parseTypeQualifier(ParseState *state, Node *parent);


#line 1259 "src/parse.exc"
 Node *parseAtomicTypeSpecifier(ParseState *state, Node *parent);


#line 1274 "src/parse.exc"
 Node *parseEnumerator(ParseState *state, Node *parent);


#line 1289 "src/parse.exc"
 Node *parseEnumeratorListL(ParseState *state, Node *parent, int need1); 
#line 1289 "src/parse.exc"
 Node *parseEnumeratorList(ParseState *state, Node *parent); 
#line 1289 "src/parse.exc"
 Node *parseEnumeratorListOpt(ParseState *state, Node *parent);


#line 1291 "src/parse.exc"
 Node *parseEnumSpecifier(ParseState *state, Node *parent);


#line 1325 "src/parse.exc"
 Node *parseStructDeclarator(ParseState *state, Node *parent);


#line 1348 "src/parse.exc"
 Node *parseStructDeclaratorListL(ParseState *state, Node *parent, int need1); 
#line 1348 "src/parse.exc"
 Node *parseStructDeclaratorList(ParseState *state, Node *parent); 
#line 1348 "src/parse.exc"
 Node *parseStructDeclaratorListOpt(ParseState *state, Node *parent);

#line 1387 "src/parse.exc"
 Node *parseSpecifierQualifierList(ParseState *state, Node *parent); 
#line 1387 "src/parse.exc"
 Node *parseSpecifierQualifierListOpt(ParseState *state, Node *parent);


#line 1389 "src/parse.exc"
 Node *parseStructDeclaration(ParseState *state, Node *parent);


#line 1405 "src/parse.exc"
 Node *parseStructDeclarationListL(ParseState *state, Node *parent, int need1); 
#line 1405 "src/parse.exc"
 Node *parseStructDeclarationList(ParseState *state, Node *parent); 
#line 1405 "src/parse.exc"
 Node *parseStructDeclarationListOpt(ParseState *state, Node *parent);


#line 1407 "src/parse.exc"
 Node *parseStructOrUnionSpecifier(ParseState *state, Node *parent);


#line 1480 "src/parse.exc"
 Node *parseStorageClassSpecifier(ParseState *state, Node *parent);


#line 1495 "src/parse.exc"
 Node *parseInitDeclarator(ParseState *state, Node *parent);


#line 1510 "src/parse.exc"
 Node *parseInitDeclaratorListL(ParseState *state, Node *parent, int need1); 
#line 1510 "src/parse.exc"
 Node *parseInitDeclaratorList(ParseState *state, Node *parent); 
#line 1510 "src/parse.exc"
 Node *parseInitDeclaratorListOpt(ParseState *state, Node *parent);


#line 1512 "src/parse.exc"
 Node *parseDeclarationSpecifiers(ParseState *state, Node *parent);


#line 1580 "src/parse.exc"
 Node *parseDeclaration(ParseState *state, Node *parent);

/***************************************************************
 * STATEMENTS                                                  *
 ***************************************************************/
 extern
#line 1611 "src/parse.exc"
 Node *parseStatement(ParseState *state, Node *parent);


#line 1613 "src/parse.exc"
 Node *parseJumpStatement(ParseState *state, Node *parent);


#line 1647 "src/parse.exc"
 Node *parseForInitializer(ParseState *state, Node *parent);


#line 1663 "src/parse.exc"
 Node *parseIterationStatement(ParseState *state, Node *parent);


#line 1703 "src/parse.exc"
 Node *parseSelectionStatement(ParseState *state, Node *parent);


#line 1734 "src/parse.exc"
 Node *parseExpressionStatement(ParseState *state, Node *parent);


#line 1747 "src/parse.exc"
 Node *parseBlockItem(ParseState *state, Node *parent);


#line 1755 "src/parse.exc"
 Node *parseBlockItemListL(ParseState *state, Node *parent, int need1); 
#line 1755 "src/parse.exc"
 Node *parseBlockItemList(ParseState *state, Node *parent); 
#line 1755 "src/parse.exc"
 Node *parseBlockItemListOpt(ParseState *state, Node *parent);


#line 1757 "src/parse.exc"
 Node *parseCompoundStatement(ParseState *state, Node *parent);


#line 1772 "src/parse.exc"
 Node *parseLabeledStatement(ParseState *state, Node *parent);


#line 1802 "src/parse.exc"
 Node *parseStatement(ParseState *state, Node *parent);

/***************************************************************
 * TRANSLATION UNIT                                            *
 ***************************************************************/

#line 1817 "src/parse.exc"
 Node *parseDeclarationListL(ParseState *state, Node *parent, int need1); 
#line 1817 "src/parse.exc"
 Node *parseDeclarationList(ParseState *state, Node *parent); 
#line 1817 "src/parse.exc"
 Node *parseDeclarationListOpt(ParseState *state, Node *parent);


#line 1819 "src/parse.exc"
 Node *parseFunctionDefinition(ParseState *state, Node *parent);


#line 1836 "src/parse.exc"
 Node *parseExternalDeclaration(ParseState *state, Node *parent);


#line 1844 "src/parse.exc"
 Node *parseTranslationUnitL(ParseState *state, Node *parent, int need1); 
#line 1844 "src/parse.exc"
 Node *parseTranslationUnit(ParseState *state, Node *parent); 
#line 1844 "src/parse.exc"
 Node *parseTranslationUnitOpt(ParseState *state, Node *parent);


#line 1846 "src/parse.exc"
 Node *parseTop(ParseState *state, Node *parent);

/***************************************************************
 * DECORATION                                                  *
 ***************************************************************/
 extern
#line 1860 "src/parse.exc"
 Node *parseDecorationOpenOpt(ParseState *state, Node *parent);


#line 1862 "src/parse.exc"
 Node *parseDecorationSubDeclaration(ParseState *state, Node *parent);

#line 1874 "src/parse.exc"
 Node *parseDecorationSubDeclarationOpt(ParseState *state, Node *parent);


#line 1876 "src/parse.exc"
 Node *parseDeclarationDecorator(ParseState *state, Node *parent);


#line 1901 "src/parse.exc"
 Node *parseDeclarationDecoratorListL(ParseState *state, Node *parent, int need1); 
#line 1901 "src/parse.exc"
 Node *parseDeclarationDecoratorList(ParseState *state, Node *parent); 
#line 1901 "src/parse.exc"
 Node *parseDeclarationDecoratorListOpt(ParseState *state, Node *parent);


#line 1903 "src/parse.exc"
 Node *parseDecorationSubExpression(ParseState *state, Node *parent);

#line 1935 "src/parse.exc"
 Node *parseDecorationSubExpressionOpt(ParseState *state, Node *parent);


#line 1937 "src/parse.exc"
 Node *parseExpressionDecorator(ParseState *state, Node *parent);


#line 1962 "src/parse.exc"
 Node *parseDecorationOpExpression(ParseState *state, Node *parent);


#line 1977 "src/parse.exc"
 Node *parseDecorationOpenCont(ParseState *state, Node *parent);


#line 2021 "src/parse.exc"
 Node *parseDecorationOpen(ParseState *state, Node *parent);

#line 2035 "src/parse.exc"
 Node *parseDecorationOpenOpt(ParseState *state, Node *parent);


/***************************************************************
 * ENTRY POINT                                                 *
 ***************************************************************/

#line 2041 "src/parse.exc"
 Node *cparse(ScanState *state, char **error);


#line 2117 "src/parse.exc"
 Node *cquickparse(struct Buffer_char *buf, parser_func_t func);
#endif
