#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "transform.h"
#include "unparse.h"
#if 0
#include "parse.h"
#include "unparse.h"
#endif

int main(int argc, char **argv)
{
    int i;
    size_t si;
    for (i = 1; i < argc; i++) {
        TransformState state;
        struct Buffer_char unparsed;

        /* remove .exc */
        char *file = argv[i];
        for (si = 0; file[si]; si++) {
            if (!strcmp(file + si, ".exc")) {
                file[si] = '\0';
                break;
            }
        }

        /* handle the file */
        state = transformFile(file);

        /* unparse it */
        if (state.files.buf[0]) {
            unparsed = cunparse(state.files.buf[0]);
            fprintf(stderr, "%s\n", unparsed.buf);
            FREE_BUFFER(unparsed);
        }
#if 0
        ScanState state;
        Node *node;
        struct Buffer_char unparsed;
        char *error;

        FILE *f;
        f = fopen(argv[i], "r");
        if (!f) {
            perror(argv[i]);
            continue;
        }

        state = newScanState(i);
        INIT_BUFFER(state.buf);
        READ_FILE_BUFFER(state.buf, f);
        WRITE_ONE_BUFFER(state.buf, '\0');

        error = NULL;
        node = cparse(&state, &error);
        FREE_BUFFER(state.buf);

        if (node) {
            unparsed = cunparse(node);
            fprintf(stderr, "%s\n", unparsed.buf);
            FREE_BUFFER(unparsed);

            unparsed = cunparseJSON(node);
            printf("%s\n", unparsed.buf);
            FREE_BUFFER(unparsed);

            freeNode(node);

        } else if (error) {
            fprintf(stderr, "%s\n", error);
            free(error);

        } else {
            fprintf(stderr, "%s: Failed to parse.\n", argv[i]);

        }
#endif

    }

    return 0;
}
