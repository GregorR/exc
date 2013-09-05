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
#define _XOPEN_SOURCE 700
#include "stdio.h"

#include "stdlib.h"

#include "string.h"

#include "sys/types.h"

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
    struct Buffer_charp cflags,
        excfiles, excfileso, excfilesoh,
        cfiles, cfileso,
        ofiles;
    /* flag options */
    int excOnly = 0,
        compileOnly = 0;
    char *specFile = NULL;
    char *outFile = NULL;
    char *cPrefix = "";
    char *oPrefix = "";
    if (!whereAmI(argv[0], &bindir, &binfil)) {
        fprintf(stderr, "Failed to find binary location, assuming .!\n");
        bindir = ".";
    }
    /* first handle the arguments */
    INIT_BUFFER(cflags);
    INIT_BUFFER(excfiles);
    INIT_BUFFER(cfiles);
    INIT_BUFFER(ofiles);
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
                } else if (!strcmp(arg, "-ec-prefix") && narg) {
                    char *ipath;
                    cPrefix = narg;
                    i++;
                    /* this also needs to be an -I path */
                    SF(ipath, malloc, NULL, (strlen(cPrefix) + 4));
                    sprintf(ipath, "-I%s", cPrefix);
                    WRITE_ONE_BUFFER(cflags, ipath);
                } else if (!strcmp(arg, "-eo-prefix") && narg) {
                    oPrefix = narg;
                    i++;
                } else {
                    fprintf(stderr, "Unrecognized exc flag: %s\n", arg);
                    exit(1);
                }
            } else if (!strcmp(arg, "-c")) {
                compileOnly = 1;
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
    WRITE_ONE_BUFFER(cflags, NULL);
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
    /* handle the output names */
    INIT_BUFFER(cfileso);
    for (si = 0; si < cfiles.bufused; si++) {
        char *cfile, *ofile, *ext;
        cfile = cfiles.buf[si];
        SF(ofile, malloc, NULL, (strlen(oPrefix) + strlen(cfile) + 1));
        sprintf(ofile, "%s%s", oPrefix, cfile);
        ext = strrchr(ofile, '.');
        if (ext && !strcmp(ext, ".c"))
            ext[1] = 'o';
        WRITE_ONE_BUFFER(cfileso, ofile);
        WRITE_ONE_BUFFER(ofiles, ofile);
    }
    INIT_BUFFER(excfileso);
    INIT_BUFFER(excfilesoh);
    for (si = 0; si < excfiles.bufused; si++) {
        char *excfile, *cfile, *hfile, *ofile, *ext;
        excfile = excfiles.buf[si];
        SF(cfile, malloc, NULL, (strlen(cPrefix) + strlen(excfile) + 1));
        sprintf(cfile, "%s%s", cPrefix, excfile);
        ext = strrchr(cfile, '.');
        if (ext && !strcmp(ext, ".exc"))
            strcpy(ext, ".c");
        WRITE_ONE_BUFFER(excfileso, cfile);
        WRITE_ONE_BUFFER(cfiles, cfile);
        SF(hfile, malloc, NULL, (strlen(cPrefix + strlen(excfile) + 1)));
        sprintf(hfile, "%s%s", cPrefix, excfile);
        ext = strrchr(hfile, '.');
        if (ext && !strcmp(ext, ".exc"))
            strcpy(ext, ".h");
        WRITE_ONE_BUFFER(excfilesoh, hfile);
        SF(ofile, malloc, NULL, (strlen(oPrefix) + strlen(excfile) + 1));
        sprintf(ofile, "%s%s", oPrefix, excfile);
        ext = strrchr(ofile, '.');
        if (ext && !strcmp(ext, ".exc"))
            strcpy(ext, ".o");
        WRITE_ONE_BUFFER(cfileso, ofile);
        WRITE_ONE_BUFFER(ofiles, ofile);
    }
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
        state = transformFile(spec, cflags.buf, file);
        /* unparse it */
        if (state.files.buf[0]) {
            FILE *f;
            struct Buffer_char unparsed = cunparse(state.files.buf[0]);
            unparsed.bufused--;
            /* write it */
            SFE(f, fopen, NULL, excfileso.buf[si], (excfileso.buf[si], "w"));
            if (fwrite(unparsed.buf, 1, unparsed.bufused, f) != unparsed.bufused) {
                perror(excfileso.buf[si]);
                exit(1);
            }
            fclose(f);
            FREE_BUFFER(unparsed);
        }
        /* unparse the .h file */
        if (state.header) {
            FILE *f;
            struct Buffer_char unparsed = cunparse(state.header);
            unparsed.bufused--;
            /* write it */
            SFE(f, fopen, NULL, excfilesoh.buf[si], (excfilesoh.buf[si], "w"));
            if (fwrite(unparsed.buf, 1, unparsed.bufused, f) != unparsed.bufused) {
                perror(excfilesoh.buf[si]);
                exit(1);
            }
            fclose(f);
            FREE_BUFFER(unparsed);
        }
        freeTransformState(&state);
    }
    if (excOnly) return 0;
    /* handle all the .c files */
    for (si = 0; si < cfiles.bufused; si++) {
        struct Buffer_char ib, ob;
        char *ofname;
        char *repNames[] = {"if", "of", NULL};
        char *repVals[] = {NULL, NULL, NULL};
        /* make the output file name */
        if (compileOnly && outFile) {
            ofname = outFile;
        } else {
            ofname = cfileso.buf[si];
        }
        /* compile */
        repVals[0] = cfiles.buf[si];
        repVals[1] = ofname;
        INIT_BUFFER(ib);
        ob = execSpec(spec->cc, cflags.buf, repNames, repVals, ib, &tmpi);
        FREE_BUFFER(ob);
        FREE_BUFFER(ib);
        if (tmpi != 0) {
            fprintf(stderr, "Failed to compile %s\n", cfiles.buf[i]);
            exit(1);
        }
    }
    if (compileOnly) return 0;
    /* and finally, link (FIXME: putting this in cflags is wrong wrong wrong) */
    cflags.bufused--;
    for (si = 0; si < ofiles.bufused; si++)
        WRITE_ONE_BUFFER(cflags, ofiles.buf[si]);
    WRITE_ONE_BUFFER(cflags, NULL);
    if (!outFile) outFile = "a.out";
    {
        struct Buffer_char ib, ob;
        char *repNames[] = {"of", NULL};
        char *repVals[] = {NULL, NULL};
        /* and link */
        repVals[0] = outFile;
        INIT_BUFFER(ib);
        ob = execSpec(spec->ld, cflags.buf, repNames, repVals, ib, &tmpi);
        FREE_BUFFER(ob);
        FREE_BUFFER(ib);
        if (tmpi != 0) {
            fprintf(stderr, "Failed to link!\n");
            exit(1);
        }
    }
    return 0;
}
