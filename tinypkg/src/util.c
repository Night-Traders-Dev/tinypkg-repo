/*
 * util.c - Package utilities (search, display, info)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

int util_search(const char *term) {
    if (!term)
        return -1;
    printf("util_search: searching for '%s'...\n", term);
    return 0;
}

int util_info(const char *name) {
    if (!name)
        return -1;
    printf("util_info: displaying info for %s...\n", name);
    return 0;
}

int util_list(void) {
    printf("util_list: listing all packages...\n");
    return 0;
}
