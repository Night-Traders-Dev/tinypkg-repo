#pragma once
#include <limits.h>

extern char TINYPKG_DIR[PATH_MAX];
extern char CACHE_DIR[PATH_MAX];
extern char BUILD_DIR[PATH_MAX];
extern char INSTALLED_DIR[PATH_MAX];
extern char PREFIX_DIR[PATH_MAX];

int load_config(void);
