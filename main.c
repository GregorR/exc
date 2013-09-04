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
    int i;
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
        struct Buffer_char cfname;
        char *file;
        FILE *f;

        /* remove .exc */
        SF(file, strdup, NULL, (argv[i]));
        for (si = 0; file[si]; si++) {
            if (!strcmp(file + si, ".exc")) {
                file[si] = '\0';
                break;
            }
        }

        /* handle the file */
        state = transformFile(file);

        /* unparse it */
        if (state.files.buf[0]) {
            struct Buffer_char unparsed = cunparse(state.files.buf[0]);
            unparsed.bufused--;

            /* write it out to the C file */
            INIT_BUFFER(cfname);
            WRITE_BUFFER(cfname, file, strlen(file));
            WRITE_BUFFER(cfname, ".c", 3);
            f = fopen(cfname.buf, "w");
            if (f == NULL) {
                perror(cfname.buf);
                exit(1);
            }
            if (fwrite(unparsed.buf, 1, unparsed.bufused, f) != unparsed.bufused) {
                perror(cfname.buf);
                exit(1);
            }
            fclose(f);
            FREE_BUFFER(cfname);
            FREE_BUFFER(unparsed);
        }

        freeTransformState(&state);
    }

    return 0;
}
