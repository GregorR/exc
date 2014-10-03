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

#line 13 "src/exc-namespace.exc"
#define _XOPEN_SOURCE 700

#include "stdio.h"

#include "stdlib.h"

#include "string.h"


#include "parse.h"

#include "transform.h"

#include "unparse.h"


#line 23 "src/exc-namespace.exc"
static void globalAdd(Node *pnode, Node *cnode)
{
    /* look for the translation unit */
    while (pnode->parent && pnode->parent->type != NODE_TRANSLATION_UNIT)
        pnode = pnode->parent;

    /* then insert it */
    trInsertBefore(pnode, cnode);
}

static Node *excNamespaceF(TransformState *state, Node *node, int *then, void *arg)
{
    char **namespaceP = arg;
    char *namespace = *namespaceP;
    Node *cnode;

    /* if this is an ID... */
    if ((node->type == NODE_ID || node->type == NODE_DIRECT_DECLARATOR) &&
        node->tok && node->tok->type == TOK_ID) {
        /* rename it */
        if (namespace[0]) {
            struct Buffer_char buf;
            char *origname = node->tok->tok;
            (void) buf;
            (void) origname;

            /* first do the conditional rename */
            INIT_BUFFER(buf);
            WRITE_BUFFER(buf, "\n@public @raw(#ifdef EXC_EXT_NAMESPACE_USING_", 45);
            WRITE_BUFFER(buf, namespace, strlen(namespace));
            WRITE_BUFFER(buf, ");", 2);
            cnode = cquickparse(&buf, parseDeclaration);
            globalAdd(node, cnode);
            buf.bufused = 0;
            WRITE_BUFFER(buf, "\n@public @raw(#define ", 22);
            WRITE_BUFFER(buf, origname, strlen(origname));
            WRITE_ONE_BUFFER(buf, ' ');
            WRITE_BUFFER(buf, namespace, strlen(namespace));
            WRITE_BUFFER(buf, origname, strlen(origname));
            WRITE_BUFFER(buf, ");", 2);
            cnode = cquickparse(&buf, parseDeclaration);
            globalAdd(node, cnode);
            buf.bufused = 0;
            WRITE_BUFFER(buf, "\n@public @raw(#endif);", 22);
            cnode = cquickparse(&buf, parseDeclaration);
            globalAdd(node, cnode);

            /* then do the actual rename */
            buf.bufused = 0;
            WRITE_BUFFER(buf, namespace, strlen(namespace));
            WRITE_BUFFER(buf, origname, strlen(origname) + 1);
            free(origname);
            node->tok->tok = buf.buf;
        }

    } else if (node->type == NODE_DECLARATION_DECORATOR &&
               node->parent->parent->type == NODE_DECORATION_DECLARATION &&
               node->children[1]->type == NODE_DECORATION_OPEN) {
        Node *repl;
        char *decoration = node->children[0]->tok->tok;
        struct Buffer_char def, ns;

        /* get the namespace */
        ns = cunparse(NULL, node->children[1]->children[0]);
        ns.bufused--;

        if (!strcmp(decoration, "namespace")) {
            /* set our current namespace */
            free(namespace);
            SF(namespace, strdup, NULL, (ns.buf));
            *namespaceP = namespace;
        }

        /* then make the #define */
        INIT_BUFFER(def);
        WRITE_BUFFER(def, "\n@raw(#ifndef EXC_EXT_NAMESPACE_USING_", 38);
        WRITE_BUFFER(def, ns.buf, ns.bufused);
        WRITE_BUFFER(def, ");", 2);
        cnode = cquickparse(&def, parseDeclaration);
        globalAdd(node, cnode);
        def.bufused = 0;
        WRITE_BUFFER(def, "\n@raw(#define EXC_EXT_NAMESPACE_USING_", 38);
        WRITE_BUFFER(def, ns.buf, ns.bufused);
        WRITE_BUFFER(def, " 1);", 4);
        cnode = cquickparse(&def, parseDeclaration);
        globalAdd(node, cnode);
        def.bufused = 0;
        WRITE_BUFFER(def, "\n@raw(#endif);", 14);
        cnode = cquickparse(&def, parseDeclaration);
        globalAdd(node, cnode);

        FREE_BUFFER(ns);

        /* and get rid of it */
        def.bufused = 0;
        WRITE_BUFFER(def, "@raw();", 8);
        repl = cquickparse(&def, parseDeclarationDecorator);
        FREE_BUFFER(def);
        trReplace(node, repl, 1);
        free(node);
        return repl;
    }

    return node;
}

static Node *excNamespace(TransformState *state, Node *node, int isprimary)
{
    TrFind find;
    char *namespace;

    if (!isprimary) return node;

    memset(&find, 0, sizeof(find));

    /* look for @namespace or @using */
    find.matchDecoration[0] = "namespace";
    find.matchDecoration[1] = "use";

    /* and for IDs */
    find.matchNode[0] = NODE_ID;
    find.matchNode[1] = NODE_DIRECT_DECLARATOR;

    /* but don't look in blocks or parameter lists */
    find.notInNode[0] = NODE_STRUCT_DECLARATION_LIST;
    find.notInNode[1] = NODE_STRUCT_OR_UNION_SPECIFIER;
    find.notInNode[2] = NODE_ENUMERATOR_LIST;
    find.notInNode[3] = NODE_PARAMETER_LIST;
    find.notInNode[4] = NODE_TYPE_SPECIFIER;
    find.notInNode[5] = NODE_BLOCK;

    /* start with the blank namespace */
    SF(namespace, strdup, NULL, (""));
    transform(state, node, &find, excNamespaceF, (void *) &namespace);
    free(namespace);

    return node;
}

void exctension(TransformState *state)
{
    trAddStage(state, "namespace", excNamespace);
}
#line 1 "<stdin>"
