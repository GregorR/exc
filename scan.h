#ifndef SCAN_H
#define SCAN_H

#include "buffer.h"
#include "node.h"

enum Tokens {
    TOK_FIRST = 0,

#define TOKEN(x) TOK_ ## x,
#include "tokens.h"
#undef TOKEN

    TOK_LAST
};

typedef struct ScanState_ {
    struct Buffer_char buf;
    size_t idx, f, l, c;
} ScanState;

ScanState newScanState(size_t f);
Token *cscan(ScanState *state);

#endif
