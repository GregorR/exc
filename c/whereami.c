/*
 * Written in 2005, 2006, 2013 by Gregor Richards
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty. 
 *
 * You should have received a copy of the CC0 Public Domain Dedication along
 * with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>. 
 */ 

#line 13 "src/whereami.exc"
#define _XOPEN_SOURCE 700
#include "whereami.h"


#include "string.h"

#include "stdio.h"

#include "stdlib.h"

#include "sys/types.h"

#include "sys/stat.h"

#include "unistd.h"


/* Separate a full path into a directory and file component (internal)
 * full: Full path
 * dir: Where to put the dir component
 * fil: Where to put the file component */
#line 27 "src/whereami.exc"
static void dirAndFil(const char *full, char **dir, char **fil)
{
    *dir = strdup(full);
    *fil = strrchr(*dir, '/');
    if (*fil) {
        **fil = '\0';
        (*fil)++;
        *fil = strdup(*fil);
    } else {
        *fil = (char *) malloc(1);
        if (!(*fil)) {
            perror("malloc");
            exit(1);
        }
        **fil = '\0';
    }
}

/* Figure out where a binary is installed
 * argvz: argv[0]
 * dir: Where to put the directory component
 * fil: Where to put the filename component
 * returns a pointer to dir or NULL for failure */

#line 50 "src/whereami.exc"
 char *whereAmI(const char *argvz, char **dir, char **fil)
{
    char *workd, *path, *retname;
    char *pathelem[1024];
    int i, j, osl;
    struct stat sbuf;
    char *argvzd = strdup(argvz);
    if (!argvzd) { perror("strdup"); exit(1); }
    /* 1: full path, yippee! */
    if (argvzd[0] == '/') {
        dirAndFil(argvzd, dir, fil);
        return argvzd;
    }
    /* 2: relative path */
    if (strchr(argvzd, '/')) {
        workd = (char *) malloc(1024 * sizeof(char));
        if (!workd) { perror("malloc"); exit(1); }

        if (getcwd(workd, 1024)) {
            retname = (char *) malloc((strlen(workd) + strlen(argvzd) + 2) * sizeof(char));
            if (!retname) { perror("malloc"); exit(1); }

            sprintf(retname, "%s/%s", workd, argvzd);
            free(workd);

            dirAndFil(retname, dir, fil);
            free(argvzd);
            return retname;
        }
    }

    /* 3: worst case: find in PATH */
    path = getenv("PATH");
    if (path == NULL) {
        return NULL;
    }
    path = strdup(path);

    /* tokenize by : */
    memset(pathelem, 0, 1024 * sizeof(char *));
    pathelem[0] = path;
    i = 1;
    osl = strlen(path);
    for (j = 0; j < osl; j++) {
        for (; path[j] != '\0' && path[j] != ':'; j++);

        if (path[j] == ':') {
            path[j] = '\0';

            j++;
            pathelem[i++] = path + j;
        }
    }

    /* go through every pathelem */
    for (i = 0; pathelem[i]; i++) {
        retname = (char *) malloc((strlen(pathelem[i]) + strlen(argvzd) + 2) * sizeof(char));
        if (!retname) { perror("malloc"); exit(1); }

        sprintf(retname, "%s/%s", pathelem[i], argvzd);

        if (stat(retname, &sbuf) == -1) {
            free(retname);
            continue;
        }

        if (sbuf.st_mode & S_IXUSR) {
            dirAndFil(retname, dir, fil);
            free(argvzd);
            return retname;
        }

        free(retname);
    }

    /* 4: can't find it */
    dir = NULL;
    fil = NULL;
    free(argvzd);
    return NULL;
}
#line 1 "<stdin>"
