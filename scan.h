#ifndef SCAN_H
#define SCAN_H

#include "buffer.h"
#include "node.h"

Token *cscan(struct Buffer_char *buf, size_t from);

#endif
