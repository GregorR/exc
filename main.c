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
    struct Buffer_charp cflags, excfiles, cfiles;

    /* flag options */
    int excOnly = 0,
        compileOnly = 0;
    char *specFile = NULL;
    char *outFile = NULL;

    if (!whereAmI(argv[0], &bindir, &binfil)) {
        fprintf(stderr, "Failed to find binary location, assuming .!\n");
        bindir = ".";
    }

    /* first handle the - arguments */
    INIT_BUFFER(cflags);
    INIT_BUFFER(excfiles);
    INIT_BUFFER(cfiles);
    for (i = 1; i < argc; i++) {
        char *arg = argv[i];
        char *narg = argv[i+1];

        if (arg[0] == '-') {
            /* A flag. Is it an exc flag? */
            if (arg[1] == 'e') {
                /* super! Which? */
                if (arg[2] == '-') {
                    /* flag passthru */
                    WRITE_ONE_BUFFER(cflags, arg + 2);

                } else if (!strcmp(arg, "-espec") && narg) {
                    /* spec file */
                    specFile = narg;
                    i++;

                } else if (!strcmp(arg, "-eonly")) {
                    excOnly = 1;

                } else {
                    fprintf(stderr, "Unrecognized exc flag: %s\n", arg);
                    exit(1);

                }

            } else if (!strcmp(arg, "-o") && narg) {
                outFile = narg;
                i++;

            } else {
                /* nope, send it to the C compiler */
                WRITE_ONE_BUFFER(cflags, arg);

            }

        } else {
            /* got a file. What kind? */
            char *ext;
            ext = strrchr(arg, '.');
            if (!ext) {
                fprintf(stderr, "Unrecognized file type: %s\n", arg);
                exit(1);
            }
            ext++;

            if (!strcmp(ext, "exc")) {
                WRITE_ONE_BUFFER(excfiles, arg);

            } else if (!strcmp(ext, "c")) {
                WRITE_ONE_BUFFER(cfiles, arg);

            } else {
                fprintf(stderr, "Unrecognized file type: %s\n", ext);
                exit(1);

            }

        }
    }


    /* check for inconsistencies in the options */
    if (excOnly && compileOnly) {
        fprintf(stderr, "Cannot specify both -eonly and -c.\n");
        exit(1);
    }
    if (excOnly && outFile) {
        fprintf(stderr, "Cannot specify -eonly and -o.\n");
        exit(1);
    }
    if (compileOnly && outFile &&
        (excfiles.bufused + cfiles.bufused) > 1) {
        fprintf(stderr, "Cannot specify -c and -o with more than one input file.\n");
        exit(1);
    }
    if ((excfiles.bufused + cfiles.bufused) == 0) {
        fprintf(stderr, "No input files.\n");
        exit(1);
    }


    /* load the spec */
    spec = excLoadSpec(bindir, specFile);
    (void) spec;


    /* handle all the .exc files */
    for (si = 0; si < excfiles.bufused; si++) {
        TransformState state;
        char *file, *ext;

        /* remove .exc */
        SF(file, strdup, NULL, (excfiles.buf[si]));
        ext = strrchr(file, '.');
        if (ext)
            *ext = '\0';

        /* handle the file */
        state = transformFile(spec, file);

        /* unparse it */
        if (state.files.buf[0]) {
            struct Buffer_char cfname;
            FILE *f;
            struct Buffer_char unparsed = cunparse(state.files.buf[0]);
            unparsed.bufused--;

            /* get the .c file name */
            INIT_BUFFER(cfname);
            WRITE_BUFFER(cfname, file, strlen(file));
            WRITE_BUFFER(cfname, ".c", 3);

            /* and write it */
            f = fopen(cfname.buf, "w");
            if (fwrite(unparsed.buf, 1, unparsed.bufused, f) != unparsed.bufused) {
                perror(cfname.buf);
                exit(1);
            }
            FREE_BUFFER(unparsed);

            /* make sure it gets compiled (FIXME: sort of a memory leak) */
            WRITE_ONE_BUFFER(cfiles, cfname.buf);
        }

        freeTransformState(&state);
    }

    /* handle all the .c files */
    for (si = 0; si < cfiles.bufused; si++) {
        struct Buffer_char ib, ob;
        char *ofname, *ext;
        char *repNames[] = {"if", "of", NULL};
        char *repVals[] = {NULL, NULL, NULL};

        /* make the output file name */
        SF(ofname, strdup, NULL, (cfiles.buf[si]));
        ext = strrchr(ofname, '.');
        if (ext && !strcmp(ext, ".c")) {
            ext[1] = 'o';
        } else {
            fprintf(stderr, "Unrecognized file extension: %s\n", ext);
            exit(1);
        }

        /* compile */
        repVals[0] = cfiles.buf[si];
        repVals[1] = ofname;
        INIT_BUFFER(ib);
        ob = execSpec(spec->cc, repNames, repVals, ib, &tmpi);
        FREE_BUFFER(ob);
        FREE_BUFFER(ib);
        free(ofname);

        if (tmpi != 0) {
            fprintf(stderr, "Failed to compile %s\n", cfiles.buf[i]);
            exit(1);
        }
    }

    return 0;
}
