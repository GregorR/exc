#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "parse.h"
#include "unparse.h"

int main(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc; i++) {
        ScanState state;
        Node *node;
        struct Buffer_char unparsed;

        FILE *f;
        f = fopen(argv[i], "r");
        if (!f) {
            perror(argv[i]);
            continue;
        }
        
        INIT_BUFFER(state.buf);
        READ_FILE_BUFFER(state.buf, f);
        WRITE_ONE_BUFFER(state.buf, '\0');
        state.f = i;
        state.idx = 0;
        state.l = state.c = 1;

        node = cparse(&state);
        FREE_BUFFER(state.buf);

        if (node) {
            unparsed = cunparse(node);
            fprintf(stderr, "%s\n", unparsed.buf);
            FREE_BUFFER(unparsed);

            unparsed = cunparseJSON(node);
            fprintf(stderr, "%s\n", unparsed.buf);
            FREE_BUFFER(unparsed);

            freeNode(node);

        } else {
            fprintf(stderr, "%s: Failed to parse.\n", argv[i]);

        }

    }

    return 0;
}
