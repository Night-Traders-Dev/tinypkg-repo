/*
 * build.c - Building and installing packages
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "build.h"

int build_package(const char *name) {
    if (!name)
        return -1;
    printf("build_package: building %s...\n", name);
    return 0;
}

int install_package(const char *name) {
    if (!name)
        return -1;
    printf("install_package: installing %s...\n", name);
    return 0;
}

int remove_package(const char *name) {
    if (!name)
        return -1;
    printf("remove_package: removing %s...\n", name);
    return 0;
}
