#define _XOPEN_SOURCE 700 /* for strdup */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "spec.h"
#include "transform.h"
#include "unparse.h"
#include "whereami.h"

int main(int argc, char **argv)
{
    int i, tmpi;
    size_t si;
    char *bindir, *binfil;
    Spec *spec;

    if (!whereAmI(argv[0], &bindir, &binfil)) {
        fprintf(stderr, "Failed to find binary location, assuming .!\n");
        bindir = ".";
    }

    spec = excLoadSpec(bindir, NULL);
    (void) spec;

    for (i = 1; i < argc; i++) {
        TransformState state;
        struct Buffer_char ofname;
        char *file;

        /* remove .exc */
        SF(file, strdup, NULL, (argv[i]));
        for (si = 0; file[si]; si++) {
            if (!strcmp(file + si, ".exc")) {
                file[si] = '\0';
                break;
            }
        }

        /* handle the file */
        state = transformFile(spec, file);

        /* unparse it */
        if (state.files.buf[0]) {
            char *repNames[] = {"of", NULL};
            char *repVals[] = {NULL, NULL};
            struct Buffer_char tmpb;
            struct Buffer_char unparsed = cunparse(state.files.buf[0]);
            unparsed.bufused--;

            /* get the .o file name */
            INIT_BUFFER(ofname);
            WRITE_BUFFER(ofname, file, strlen(file));
            WRITE_BUFFER(ofname, ".o", 3);

            /* compile */
            repVals[0] = ofname.buf;
            tmpb = execSpec(spec->cc, repNames, repVals,
                            unparsed, &tmpi);
            FREE_BUFFER(tmpb);
            FREE_BUFFER(ofname);
            FREE_BUFFER(unparsed);

            if (tmpi != 0) {
                fprintf(stderr, "Failed to compile %s\n", file);
                exit(1);
            }
        }

        freeTransformState(&state);
    }

    return 0;
}
