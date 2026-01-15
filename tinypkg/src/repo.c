/*
 * repo.c - Repository management (downloading and storing)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "repo.h"

int repo_sync(void) {
    printf("repo_sync: synchronizing repository...\n");
    return 0;
}

int repo_add(const char *url) {
    if (!url)
        return -1;
    printf("repo_add: adding repository from %s\n", url);
    return 0;
}

int repo_remove(const char *name) {
    if (!name)
        return -1;
    printf("repo_remove: removing repository %s\n", name);
    return 0;
}
