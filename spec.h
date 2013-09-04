#ifndef SPEC_H
#define SPEC_H

#include "buffer.h"

#ifndef EXC_DEFAULT_SPEC_FILE
#define EXC_DEFAULT_SPEC_FILE "gcc.excspec"
#endif

#ifndef EXC_DEFAULT_SPEC_DIR
#define EXC_DEFAULT_SPEC_DIR "/../share/exc/spec/"
#endif

BUFFER(charp, char *);

typedef struct SpecCmd_ {
    struct Buffer_charp cmd;
    struct Buffer_int repPositions; /* which parameters are replaceable */
    int i, o; /* replacements for input and output file, or -1 for stdin/stdout */
} SpecCmd;

typedef struct Spec_ {
    SpecCmd *cpp, *cc;
} Spec;

/* load the default or specified spec file (return allocated) */
Spec *excLoadSpec(const char *bindir, const char *file);

/* run a spec command with the given replacements */
struct Buffer_char execSpec(
    SpecCmd *cmd,
    char *const repNames[],
    char *const repVals[],
    struct Buffer_char input,
    int *status);

#endif
