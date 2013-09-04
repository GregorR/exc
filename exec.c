#define _XOPEN_SOURCE 700
#include "stdio.h"

#include "stdlib.h"

#include "sys/types.h"

#include "sys/wait.h"

#include "unistd.h"

#include "exec.h"

/*
 * Copyright (C) 2009 Gregor Richards
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/* SFC: safely use functions, given provided code in case of an error */
/* SFE: SFC with perror */
/* SF: safely use functions that fail with errno without pulling your hair out */
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
    (tmpi) = pipe (pipei); if ((tmpi) == (-1)) { perror("pipe"); exit(1); };;
    (tmpi) = pipe (pipeo); if ((tmpi) == (-1)) { perror("pipe"); exit(1); };;
    /* FIXME: the command should be in a spec file! */
    (pid) = fork (); if ((pid) == (-1)) { perror("fork"); exit(1); };;
    if (pid == 0) {
        (tmpi) = close (pipei[1]); if ((tmpi) == (-1)) { perror("close"); exit(1); };;
        (tmpi) = close (pipeo[0]); if ((tmpi) == (-1)) { perror("close"); exit(1); };;
        (tmpi) = dup2 (pipei[0], 0); if ((tmpi) == (-1)) { perror("dup2"); exit(1); };;
        (tmpi) = dup2 (pipeo[1], 1); if ((tmpi) == (-1)) { perror("dup2"); exit(1); };;
        (tmpi) = close (pipei[0]); if ((tmpi) == (-1)) { perror("close"); exit(1); };;
        (tmpi) = close (pipeo[1]); if ((tmpi) == (-1)) { perror("close"); exit(1); };;
        (tmpi) = execvp (cmd[0], cmd); if ((tmpi) == (-1)) { perror("execvp"); exit(1); };;
        exit(1);
    }
    (tmpi) = close (pipei[0]); if ((tmpi) == (-1)) { perror("close"); exit(1); };;
    (tmpi) = close (pipeo[1]); if ((tmpi) == (-1)) { perror("close"); exit(1); };;
    /* give them the input */
    (tmpi) = write (pipei[1], input.buf, input.bufused); if ((tmpi) == (-1)) { perror("write"); exit(1); };;
    (tmpi) = close (pipei[1]); if ((tmpi) == (-1)) { perror("close"); exit(1); };;
    /* read in the preprocessed source */
    (f) = fdopen (pipeo[0], "r"); if ((f) == (NULL)) { perror("fdopen"); exit(1); };;
    INIT_BUFFER(ret);
    READ_FILE_BUFFER(ret, f);
    (tmpi) = fclose (f); if ((tmpi) == (EOF)) { perror("fclose"); exit(1); };;
    /* and wait for them */
    (tmpp) = waitpid (pid, status, 0); if ((tmpp) == (-1)) { perror("waitpid"); exit(1); };;
    return ret;
}
