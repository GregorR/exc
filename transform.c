#include <string.h>

#include "parse.h"
#include "transform.h"

enum {
    MATCH_NO = 0,
    MATCH_MATCH,
    MATCH_NOTIN
};

static int match(TransformState *state, Node *node, TrFind *find)
{
    int i;

    /* first try node type match */
    for (i = 0; i < TR_FIND_MATCH_CT; i++) {
        if (find->matchNode[i] == node->type) return MATCH_MATCH;
        if (find->notInNode[i] == node->type) return MATCH_NOTIN;
    }

    /* now if it's a decorator, try matching it */
    if (node->type == NODE_EXPRESSION_DECORATOR ||
        node->type == NODE_DECLARATION_DECORATOR) {
        /* the first child is a decoration name, check it */
        const char *dName = node->children[0]->tok->tok;

        for (i = 0; i < TR_FIND_MATCH_CT &&
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

void transform(TransformState *state, Node *node, TrFind *find, transform_func_t func)
{
    int then;
    size_t i;

    /* find a matching node */
    while (node) {
        /* first try the node itself */
        if (match(state, node, find)) {
            Node *snode;

            then = THEN_INNER_EXCLUSIVE;
            snode = func(state, node, &then);

            /* and figure out what to do next */
            if (snode) {
                node = snode;
                if (then == THEN_INNER_INCLUSIVE) continue;
                else if (then == THEN_OUTER) goto outer;
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
