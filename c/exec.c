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
#include "exec.h"

#include "stdio.h"

#include "stdlib.h"

#include "sys/wait.h"

#include "unistd.h"

/* run this command with the given input (as a buffer, non-null-terminated),
 * returning output as a buffer, again non-null-terminated */
 struct Buffer_char execBuffered(
    char *const cmd[],
    struct Buffer_char input,
    int *status)
{
    int tmpi;
    int pipei[2], pipeo[2];
    pid_t pid, tmpp;
    FILE *f;
    struct Buffer_char ret;
    /* prepare to pipe it into/out of the child */
    SF(tmpi, pipe, -1, (pipei));
    SF(tmpi, pipe, -1, (pipeo));
    /* FIXME: the command should be in a spec file! */
    SF(pid, fork, -1, ());
    if (pid == 0) {
        SF(tmpi, close, -1, (pipei[1]));
        SF(tmpi, close, -1, (pipeo[0]));
        SF(tmpi, dup2, -1, (pipei[0], 0));
        SF(tmpi, dup2, -1, (pipeo[1], 1));
        SF(tmpi, close, -1, (pipei[0]));
        SF(tmpi, close, -1, (pipeo[1]));
        SF(tmpi, execvp, -1, (cmd[0], cmd));
        exit(1);
    }
    SF(tmpi, close, -1, (pipei[0]));
    SF(tmpi, close, -1, (pipeo[1]));
    /* give them the input */
    SF(tmpi, write, -1, (pipei[1], input.buf, input.bufused));
    SF(tmpi, close, -1, (pipei[1]));
    /* read in the preprocessed source */
    SF(f, fdopen, NULL, (pipeo[0], "r"));
    INIT_BUFFER(ret);
    READ_FILE_BUFFER(ret, f);
    SF(tmpi, fclose, EOF, (f));
    /* and wait for them */
    SF(tmpp, waitpid, -1, (pid, status, 0));
    return ret;
}
