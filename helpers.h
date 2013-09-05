/*
 * Written in 2009, 2010 by Gregor Richards
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty. 
 *
 * You should have received a copy of the CC0 Public Domain Dedication along
 * with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>. 
 */

#ifndef HELPERS_H
#define HELPERS_H

/* SFC: safely use functions, given provided code in case of an error */
#define SFC(into, func, bad, args) \
    (into) = func args; \
    if ((into) == (bad))

/* SFE: SFC with perror */
#define SFE(into, func, bad, error, args) \
    SFC(into, func, bad, args) { \
        perror(error); \
        exit(1); \
    }

/* SF: safely use functions that fail with errno without pulling your hair out */
#define SF(into, func, bad, args) \
    SFE(into, func, bad, #func, args);

#endif
