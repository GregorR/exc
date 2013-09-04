#ifndef EXC_unparse
#define EXC_unparse 1
#include "stdio.h"

#include "stdlib.h"

#include "sys/types.h"

#include "buffer.h"

#include "node.h"

/* convert a token type to a name */
 const char *tokenName(int n);
/* convert a node type (numeric) to a name */
 const char *nodeName(int n);
 struct Buffer_char cunparse(Node *node);
 struct Buffer_char cunparseStrLiteral(Token *tok);
 struct Buffer_char cunparseJSON(Node *node);
#endif
