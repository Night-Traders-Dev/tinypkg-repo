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

    path_join(TINYPKG_DIR, PATH_MAX, home, ".tinypkg");
    path_join(CACHE_DIR, PATH_MAX, TINYPKG_DIR, "cache");
    path_join(BUILD_DIR, PATH_MAX, TINYPKG_DIR, "build");
    path_join(INSTALLED_DIR, PATH_MAX, TINYPKG_DIR, "installed");
    path_join(PREFIX_DIR, PATH_MAX, TINYPKG_DIR, "opt");

    ensure_dir(TINYPKG_DIR);
    ensure_dir(CACHE_DIR);
    ensure_dir(BUILD_DIR);
    ensure_dir(INSTALLED_DIR);
    ensure_dir(PREFIX_DIR);
    return 0;
}
