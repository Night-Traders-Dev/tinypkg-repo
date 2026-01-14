#include "git.h"
#include "config.h"
#include "util.h"
#include <stdio.h>
#include <unistd.h>

int repo_update(void) {
    char repo[4096];

    if (path_join(repo, sizeof(repo), CACHE_DIR, "repo") != 0) {
        fprintf(stderr, "Path too long\n");
        return 1;
    }

    if (access(repo, F_OK) != 0) {
        return run_cmd("git clone https://github.com/Night-Traders-Dev/tinypkg-repo %s", repo);
    } else {
        return run_cmd("cd %s && git pull", repo);
    }
}
