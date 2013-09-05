#ifndef EXC_spec
#define EXC_spec 1
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
#include "sys/types.h"

#include "buffer.h"

/* FIXME: Should this be at the C level or the exc level? */
BUFFER(charp, char *);
 typedef struct SpecCmd_ {
    struct Buffer_charp cmd;
    struct Buffer_int repPositions; /* which parameters are replaceable */
    int i, o; /* replacements for input and output file, or -1 for stdin/stdout */
} SpecCmd;
 typedef struct Spec_ {
    SpecCmd *cpp, *cc, *ld;
} Spec;
/* load the default or specified spec file (return allocated) */
 Spec *excLoadSpec(const char *bindir, const char *file);
/* run a spec command with the given replacements */
 struct Buffer_char execSpec(
    SpecCmd *cmd,
    char *const addlFlags[],
    char *const repNames[],
    char *const repVals[],
    struct Buffer_char input,
    int *status);
#endif
