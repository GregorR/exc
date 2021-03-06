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

#ifndef BUFFER_H
#define BUFFER_H

#if defined(BUFFER_GGGGC)

#elif defined(BUFFER_GC)
#define _BUFFER_MALLOC GC_malloc
#define _BUFFER_REALLOC GC_realloc
#define _BUFFER_FREE GC_free

#else
#define _BUFFER_MALLOC malloc
#define _BUFFER_REALLOC realloc
#define _BUFFER_FREE free

#endif

#include "helpers.h"

#define BUFFER_DEFAULT_SIZE 8

/* auto-expanding buffer */
#if defined(BUFFER_GGGGC)
#define PTR_BUFFER(name, type) \
struct Buffer_ ## name { \
    size_t bufsz, bufused; \
    type ## Array buf; \
}

#define DATA_BUFFER(name, type) \
struct Buffer_ ## name { \
    size_t bufsz, bufused; \
    type *buf; \
}

DATA_BUFFER(char, char);
DATA_BUFFER(int, int);

#else
#define BUFFER(name, type) \
struct Buffer_ ## name { \
    size_t bufsz, bufused; \
    type *buf; \
}

BUFFER(char, char);
BUFFER(charp, char *);
BUFFER(int, int);
#endif

/* initialize a buffer */
#if defined(BUFFER_GGGGC)
#define INIT_PTR_BUFFER(buffer) \
{ \
    (buffer).bufsz = BUFFER_DEFAULT_SIZE; \
    (buffer).bufused = 0; \
    (buffer).buf = GGC_NEW_PTR_ARRAY_VOIDP(BUFFER_DEFAULT_SIZE); \
}
#define INIT_DATA_BUFFER(buffer) \
{ \
    (buffer).bufsz = BUFFER_DEFAULT_SIZE; \
    (buffer).bufused = 0; \
    (buffer).buf = GGC_NEW_DATA_ARRAY_VOIDP(*(buffer).buf, BUFFER_DEFAULT_SIZE); \
}

#else
#define INIT_BUFFER(buffer) \
{ \
    (buffer).bufsz = BUFFER_DEFAULT_SIZE; \
    (buffer).bufused = 0; \
    SF((buffer).buf, _BUFFER_MALLOC, NULL, (BUFFER_DEFAULT_SIZE * sizeof(*(buffer).buf))); \
}

#endif

/* free a buffer (not for use with GGGGC) */
#define FREE_BUFFER(buffer) \
{ \
    if ((buffer).buf) _BUFFER_FREE((buffer).buf); \
    (buffer).buf = NULL; \
}

/* the address of the free space in the buffer */
#if defined(BUFFER_GGGGC)
#define PTR_BUFFER_END(buffer) ((buffer).buf->d + (buffer).bufused)
#endif
#define BUFFER_END(buffer) ((buffer).buf + (buffer).bufused)

/* mark new bytes in a buffer */
#define STEP_BUFFER(buffer, by) ((buffer).bufused += by)

/* the amount of space left in the buffer */
#define BUFFER_SPACE(buffer) ((buffer).bufsz - (buffer).bufused)


/* expand a buffer */
#if defined(BUFFER_GGGGC)
#define EXPAND_PTR_BUFFER(buffer) \
{ \
    (buffer).bufsz *= 2; \
    (buffer).buf = GGC_REALLOC_PTR_ARRAY_VOIDP((buffer).buf, (buffer).bufsz); \
}
#define EXPAND_BUFFER(buffer) \
{ \
    (buffer).bufsz *= 2; \
    (buffer).buf = GGC_REALLOC_DATA_ARRAY_VOIDP(*(buffer).buf, (buffer).buf, (buffer).bufsz); \
}

#else
#define EXPAND_BUFFER(buffer) \
{ \
    (buffer).bufsz *= 2; \
    SF((buffer).buf, _BUFFER_REALLOC, NULL, ((buffer).buf, (buffer).bufsz * sizeof(*(buffer).buf))); \
}

#endif

/* write a string to a buffer */
#define WRITE_BUFFER(buffer, string, len) \
{ \
    size_t _len = (len); \
    while (BUFFER_SPACE(buffer) < _len) { \
        EXPAND_BUFFER(buffer); \
    } \
    memcpy(BUFFER_END(buffer), string, _len * sizeof(*(buffer).buf)); \
    STEP_BUFFER(buffer, _len); \
}

/* write a single element into a buffer */
#define WRITE_ONE_BUFFER(buffer, elem) \
{ \
    if ((buffer).bufused == (buffer).bufsz) { \
        EXPAND_BUFFER(buffer); \
    } \
    *BUFFER_END(buffer) = (elem); \
    STEP_BUFFER(buffer, 1); \
}

/* read a file into a buffer */
#define READ_FILE_BUFFER(buffer, fh) \
{ \
    size_t _rd; \
    while ((_rd = fread(BUFFER_END(buffer), 1, BUFFER_SPACE(buffer), (fh))) > 0) { \
        STEP_BUFFER(buffer, _rd); \
        if (BUFFER_SPACE(buffer) <= 0) { \
            EXPAND_BUFFER(buffer); \
        } \
    } \
}

#endif
