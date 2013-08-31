#ifndef UNPARSE_H
#define UNPARSE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "buffer.h"
#include "node.h"

struct Buffer_char cunparse(Node *node);

#endif
