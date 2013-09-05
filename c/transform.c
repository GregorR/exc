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
#include "transform.h"

#include "builtin-stages.h"

#include "exec.h"

#include "parse.h"

#include "stdio.h"

#include "stdlib.h"

#include "stdarg.h"

#include "string.h"

#include "sys/types.h"

#include "unistd.h"

enum {
    MATCH_NO = 0,
    MATCH_MATCH,
    MATCH_NOTIN
};
/* parenthesize a node */
 Node *trParenthesize(Node *node)
{
    Node *ret = newNode(NULL, NODE_PAREN, newToken(TOK_LPAREN, 1, NULL, "("), 2);
    node->parent = ret;
    ret->children[0] = node;
    ret->children[1] = newNode(ret, NODE_TOK, newToken(TOK_RPAREN, 1, NULL, ")"), 0);
    return ret;
}
/* replace a node */
 void trReplace(Node *from, Node *to, int preserveWhitespace)
{
    size_t i;
    if (from->parent) {
        Node *pnode = from->parent;
        for (i = 0; pnode->children[i] && pnode->children[i] != from; i++);
        if (pnode->children[i]) pnode->children[i] = to;
        to->parent = pnode;
    }
    if (preserveWhitespace) {
        /* attempt to preserve the whitespace by drilling down until we find
         * the leftmost tok, then taking its whitespace */
        char *keepWhitespace = "";
        Node *n = from;
        while (n) {
            if (n->tok) {
                if (n->tok->pre)
                    keepWhitespace = n->tok->pre;
                break;
            }
            n = n->children[0];
        }
        if (keepWhitespace[0]) {
            n = to;
            while (n) {
                if (n->tok) {
                    char *nw;
                    if (n->tok->pre) {
                        SF(nw, malloc, NULL, (strlen(keepWhitespace) + strlen(n->tok->pre) + 1));
                        sprintf(nw, "%s%s", keepWhitespace, n->tok->pre);
                    } else {
                        SF(nw, strdup, NULL, (keepWhitespace));
                    }
                    free(n->tok->pre);
                    n->tok->pre = nw;
                    break;
                }
                n = n->children[0];
            }
        }
    }
}
/* resize a node */
 Node *trResize(Node *node, size_t to)
{
    size_t i;
    Node *nnode = newNode(NULL, node->type, node->tok, to);
    node->tok = NULL;
    for (i = 0; node->children[i] && i < to; i++) {
        nnode->children[i] = node->children[i];
        nnode->children[i]->parent = nnode;
        node->children[i] = NULL;
    }
    /* make sure we don't lose any children */
    if (node->children[i]) {
        for (; node->children[i]; i++) {
            freeNode(node->children[i]);
            node->children[i] = NULL;
        }
    }
    /* update it in the parent */
    trReplace(node, nnode, 0);
    /* free the old node */
    freeNode(node);
    return nnode;
}
/* append nodes as children of an existing node */
 Node *trAppend(Node *parent, ...)
{
    struct Buffer_Nodep buf;
    Node *child;
    va_list ap;
    size_t i, ni;
    INIT_BUFFER(buf);
    /* first collect all the nodes */
    va_start(ap, parent);
    while ((child = (va_arg(ap, Node *))))
        WRITE_ONE_BUFFER(buf, child);
    va_end(ap);
    /* now resize the parent node */
    for (i = 0; parent->children[i]; i++);
    parent = trResize(parent, i + buf.bufused);
    /* and add the new nodes */
    for (ni = 0; ni < buf.bufused; ni++, i++) {
        parent->children[i] = buf.buf[ni];
        parent->children[i]->parent = parent;
    }
    FREE_BUFFER(buf);
    return parent;
}
/* prepend a single node to an existing node, and perhaps give it the
 * successor's whitespace */
 Node *trPrepend(Node *parent, Node *child)
{
    size_t i, ni;
    /* resize the parent node */
    for (i = 0; parent->children[i]; i++);
    parent = trResize(parent, i + 1);
    /* move the children */
    for (ni = i; ni >= 1; ni--)
        parent->children[ni] = parent->children[ni-1];
    /* and add the new node */
    parent->children[0] = child;
    child->parent = parent;
    return parent;
}
/* duplicate a tree of nodes */
 Node *trDupNode(Node *node)
{
    size_t i;
    Node *ret;
    Token *tok = NULL;
    /* first duplicate the token */
    if (node->tok) {
        tok = newToken(node->tok->type, 1, node->tok->pre, node->tok->tok);
        tok->idx = node->tok->idx;
        tok->f = node->tok->f;
        tok->l = node->tok->l;
        tok->c = node->tok->c;
    }
    /* count children */
    for (i = 0; node->children[i]; i++);
    /* allocate the new node */
    ret = newNode(NULL, node->type, tok, i);
    /* and copy children */
    for (i = 0; node->children[i]; i++) {
        Node *c = trDupNode(node->children[i]);
        c->parent = ret;
        ret->children[i] = c;
    }
    return ret;
}
static int match(TransformState *state, Node *node, TrFind *find)
{
    int i;
    /* first try node type match */
    for (i = 0; i < 4; i++) {
        if (find->matchNode[i] == node->type) return MATCH_MATCH;
        if (find->notInNode[i] == node->type) return MATCH_NOTIN;
    }
    /* now if it's a decorator, try matching it */
    if (node->type == NODE_EXPRESSION_DECORATOR ||
        node->type == NODE_DECLARATION_DECORATOR) {
        /* the first child is a decoration name, check it */
        const char *dName = node->children[0]->tok->tok;
        for (i = 0; i < 4 &&
                    (find->matchDecoration[i] || find->notInDecoration[i]); i++) {
            if (find->matchDecoration[i] &&
                !strcmp(find->matchDecoration[i], dName)) return MATCH_MATCH;
            if (find->notInDecoration[i] &&
                !strcmp(find->notInDecoration[i], dName)) return MATCH_NOTIN;
        }
    }
    /* and maybe it'll provide functions */
    if (find->matchFunc && find->matchFunc(state, node)) return MATCH_MATCH;
    if (find->notInFunc && find->notInFunc(state, node)) return MATCH_NOTIN;
    return MATCH_NO;
}
/* perform the given transformation on matching nodes */
 void transform(TransformState *state, Node *node, TrFind *find, transform_func_t func, void *arg)
{
    int then;
    size_t i;
    /* find a matching node */
    while (node) {
        /* first try the node itself */
        if (match(state, node, find)) {
            Node *snode;
            then = THEN_INNER_EXCLUSIVE;
            snode = func(state, node, &then, arg);
            /* and figure out what to do next */
            if (snode) {
                node = snode;
                if (then == THEN_INNER_INCLUSIVE) continue;
                else if (then == THEN_OUTER) goto outer;
                else if (then == THEN_STOP) break;
            }
        }
        /* then children nodes */
        if (node->children[0]) {
            node = node->children[0];
            continue;
        }
        /* then siblings */
outer:
        while (node) {
            Node *pnode = node->parent;
            if (pnode) {
                for (i = 0; pnode->children[i] && pnode->children[i] != node; i++);
                if (pnode->children[i]) i++;
                if (pnode->children[i]) {
                    /* OK, have a neighbor, continue from there */
                    node = pnode->children[i];
                    break;
                }
            }
            node = pnode;
        }
    }
}
/* starting from the given file (malloc'd, now owned by TransformState), read,
 * preprocess, and transform */
 TransformState transformFile(Spec *spec, char *const cflags[], char *filename)
{
    TransformState state;
    size_t i;
    int tmpi;
    Node *node;
    INIT_BUFFER(state.transforms);
    INIT_BUFFER(state.filenames);
    INIT_BUFFER(state.files);
    state.header = NULL;
    WRITE_ONE_BUFFER(state.filenames, filename);
    /* first stage just gets all the files input */
    for (i = 0; i < state.filenames.bufused; i++) {
        filename = state.filenames.buf[i];
        /* have we read, preprocessed and parsed the file? */
        if (state.files.bufused <= i) {
            /* we need to read in the file first! */
            struct Buffer_char loader, source;
            ScanState scanState;
            char *error;
            INIT_BUFFER(loader);
            WRITE_BUFFER(loader, "#include \"", 10);
            WRITE_BUFFER(loader, filename, strlen(filename));
            WRITE_BUFFER(loader, ".exc\"\n", 6);
            source = execSpec(spec->cpp, cflags, NULL, NULL, loader, &tmpi);
            FREE_BUFFER(loader);
            WRITE_ONE_BUFFER(source, '\0');
            if (tmpi != 0) {
                fprintf(stderr, "cpp failed!\n");
                exit(1);
            }
            /* now parse it */
            scanState = newScanState(i);
            scanState.buf = source;
            error = NULL;
            node = cparse(&scanState, &error);
            if (!node) {
                fprintf(stderr, "%s: %s\n", filename, error);
                FREE_BUFFER(source);
                continue;
            }
            FREE_BUFFER(source);
            WRITE_ONE_BUFFER(state.files, node);
        }
        /* run the imports */
        state.files.buf[i] = transformImportStage(&state, state.files.buf[i], (i == 0));
    }
    /* @extension unimplemented */
    /* header stage */
    for (i = 0; i < state.files.bufused; i++)
        state.files.buf[i] = transformHeaderStage(&state, state.files.buf[i], (i == 0));
    /* finally, the @raw stage */
    for (i = 0; i < state.files.bufused; i++)
        state.files.buf[i] = transformRawStage(&state, state.files.buf[i], (i == 0));
    return state;
}
/* free a TransformState */
 void freeTransformState(TransformState *state)
{
    size_t i;
    FREE_BUFFER(state->transforms);
    for (i = 0; i < state->filenames.bufused; i++)
        free(state->filenames.buf[i]);
    FREE_BUFFER(state->filenames);
    for (i = 0; i < state->files.bufused; i++)
        if (state->files.buf[i])
            freeNode(state->files.buf[i]);
    FREE_BUFFER(state->files);
    if (state->header)
        freeNode(state->header);
}
