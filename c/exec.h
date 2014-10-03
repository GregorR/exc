#ifndef EXC_src_exec
#define EXC_src_exec 1
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

#line 2 "???"
#ifdef EXC_EXT_NAMESPACE_USING_excExec_
#define execBuffered excExec_execBuffered
#endif

/* run this command with the given input (as a buffer, non-null-terminated),
 * returning output as a buffer, again non-null-terminated */

#line 30 "src/exec.exc"
 struct Buffer_char excExec_execBuffered(
    char *const cmd[],
    struct Buffer_char input,
    int *status);
#endif
