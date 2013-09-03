#ifndef EXEC_H
#define EXEC_H

#include "buffer.h"

/* run this command with the given input (as a buffer, non-null-terminated),
 * returning output as a buffer, again non-null-terminated */
struct Buffer_char execBuffered(
    char *const cmd[],
    struct Buffer_char input,
    int *status);

#endif
