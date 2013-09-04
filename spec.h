#ifndef SPEC_H
#define SPEC_H

#include "buffer.h"
#include "transform.h" /* FIXME: for Buffer_charp */

#ifndef EXC_DEFAULT_SPEC_FILE
#define EXC_DEFAULT_SPEC_FILE "gcc.excspec"
#endif

#ifndef EXC_DEFAULT_SPEC_DIR
#define EXC_DEFAULT_SPEC_DIR "/../share/exc/spec/"
#endif

typedef struct SpecCmd_ {
    struct Buffer_charp cmd;
    struct Buffer_int repPositions; /* which parameters are replaceable */
    struct Buffer_charp repNames; /* and the names of the replacements */
    int i, o; /* replacements for input and output file, or -1 for stdin/stdout */
} SpecCmd;

typedef struct Spec_ {
    SpecCmd *cpp;
} Spec;

/* load the default or specified spec file (return allocated) */
Spec *excLoadSpec(const char *bindir, const char *file);

/* run a spec command with the given replacements */
struct Buffer_char execSpec(
    SpecCmd *cmd,
    char *const replNames[],
    char *const replVals[],
    struct Buffer_char input,
    int *status);

#endif
