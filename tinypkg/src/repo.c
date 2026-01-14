#include "repo.h"
#include "config.h"
#include "util.h"
#include <stdio.h>

int repo_list(void) {
    return run_cmd("ls %s/repo/packages", CACHE_DIR);
}

int repo_search(const char *term) {
    return run_cmd("ls %s/repo/packages | grep -i %s", CACHE_DIR, term);
}

int repo_info(const char *pkg) {
    return run_cmd("cat %s/repo/packages/%s/manifest.yaml", CACHE_DIR, pkg);
}
