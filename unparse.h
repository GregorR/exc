#ifndef UNPARSE_H
#define UNPARSE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "buffer.h"
#include "node.h"

const char *tokenName(int n);
const char *nodeName(int n);
struct Buffer_char cunparse(Node *node);
struct Buffer_char cunparseStrLiteral(Token *tok);
struct Buffer_char cunparseJSON(Node *node);

#endif
