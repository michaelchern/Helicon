#include <stdio.h>
#include <stdlib.h>

#include "helicon.h"

int main(void) {
    if (helicon_add(2, 3) != 5) {
        fprintf(stderr, "Expected 2 + 3 to equal 5\n");
        return EXIT_FAILURE;
    }

    if (helicon_add(-2, 1) != -1) {
        fprintf(stderr, "Expected -2 + 1 to equal -1\n");
        return EXIT_FAILURE;
    }

    printf("All Helicon tests passed.\n");
    return EXIT_SUCCESS;
}
