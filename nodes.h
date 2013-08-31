/* Constants/literals */
NODE(NIL)

NODE(TOK)

NODE(ID)

NODE(INT_LITERAL)
NODE(FLOAT_LITERAL)
NODE(CHAR_LITERAL)
NODE(STR_LITERAL)

/* Expressions */
NODE(PAREN)

NODE(GENERIC_SELECTION)
NODE(GENERIC_ASSOC_LIST)
NODE(GENERIC_ASSOCIATION)
NODE(GENERIC_ASSOCIATION_DEFAULT)

NODE(INDEX)
NODE(CALL)
NODE(MEMBER_DOT)
NODE(MEMBER_ARROW)
NODE(POSTINC)
NODE(POSTDEC)
NODE(COMPOUND_LITERAL)

NODE(ARGUMENT_EXPRESSION_LIST)

NODE(PREINC)
NODE(PREDEC)
NODE(ADDROF)
NODE(DEREF)
NODE(POSITIVE)
NODE(NEGATIVE)
NODE(BNOT)
NODE(NOT)
NODE(SIZEOF_EXP)
NODE(SIZEOF_TYPE)
NODE(ALIGNOF)

NODE(CAST)

NODE(MUL)
NODE(DIV)
NODE(MOD)

NODE(ADD)
NODE(SUB)

NODE(SHL)
NODE(SHR)

NODE(LT)
NODE(GT)
NODE(LTE)
NODE(GTE)

NODE(EQ)
NODE(NEQ)

NODE(BAND)

NODE(BXOR)

NODE(BOR)

NODE(AND)

NODE(OR)

NODE(CONDITIONAL)

NODE(ASG)
NODE(RASG)

NODE(COMMA)

/* Declarations */
NODE(DECLARATION)
NODE(DECLARATION_SPECIFIERS)
NODE(INIT_DECLARATOR_LIST)
NODE(INIT_DECLARATOR)
NODE(STORAGE_CLASS_SPECIFIER)
NODE(TYPE_SPECIFIER)
NODE(STRUCT_OR_UNION_SPECIFIER)
NODE(STRUCT_DECLARATION_LIST)
NODE(STRUCT_DECLARATION)
NODE(SPECIFIER_QUALIFIER_LIST)
NODE(STRUCT_DECLARATOR_LIST)
NODE(BITFIELD_DECLARATOR)
NODE(BITFIELD_PADDING)
NODE(ENUM_SPECIFIER)
NODE(ENUMERATOR_LIST)
NODE(ENUMERATOR)
NODE(ATOMIC_TYPE_SPECIFIER)
NODE(TYPE_QUALIFIER)
NODE(FUNCTION_SPECIFIER)
NODE(ALIGNMENT_SPECIFIER)
NODE(DECLARATOR)
NODE(DIRECT_DECLARATOR)
NODE(POINTER)
NODE(TYPE_QUALIFIER_LIST)
NODE(PARAMETER_TYPE_LIST)
NODE(PARAMETER_LIST)
NODE(PARAMETER_DECLARATION)
NODE(IDENTIFIER_LIST)
NODE(TYPE_NAME)
NODE(ABSTRACT_DECLARATOR)
NODE(DIRECT_ABSTRACT_DECLARATOR)
NODE(INITIALIZER)
NODE(DESIGNATION_INITIALIZER)
NODE(INITIALIZER_LIST)
NODE(DESIGNATION)
NODE(DESIGNATOR_LIST)
NODE(DESIGNATOR)
NODE(STATIC_ASSERT_DECLARATION)
