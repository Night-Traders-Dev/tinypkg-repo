#include "config.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

char TINYPKG_DIR[PATH_MAX];
char CACHE_DIR[PATH_MAX];
char BUILD_DIR[PATH_MAX];
char INSTALLED_DIR[PATH_MAX];
char PREFIX_DIR[PATH_MAX];

int load_config(void) {
    const char *home = getenv("HOME");
    if (!home) return -1;

    if (path_join(TINYPKG_DIR, PATH_MAX, home, ".tinypkg") != 0) return -1;
    if (path_join(CACHE_DIR, PATH_MAX, TINYPKG_DIR, "cache") != 0) return -1;
    if (path_join(BUILD_DIR, PATH_MAX, TINYPKG_DIR, "build") != 0) return -1;
    if (path_join(INSTALLED_DIR, PATH_MAX, TINYPKG_DIR, "installed") != 0) return -1;
    if (path_join(PREFIX_DIR, PATH_MAX, TINYPKG_DIR, "opt") != 0) return -1;

    if (ensure_dir(TINYPKG_DIR) != 0) return -1;
    if (ensure_dir(CACHE_DIR) != 0) return -1;
    if (ensure_dir(BUILD_DIR) != 0) return -1;
    if (ensure_dir(INSTALLED_DIR) != 0) return -1;
    if (ensure_dir(PREFIX_DIR) != 0) return -1;
    return 0;
}
