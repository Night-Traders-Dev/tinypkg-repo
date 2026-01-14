#include "install.h"
#include "config.h"
#include "util.h"

int install_package(const char *pkg) {
    char dir[4096];
    path_join(dir, sizeof(dir), BUILD_DIR, pkg);
    return run_cmd("cd %s/* && make install", dir);
}

int remove_package(const char *pkg) {
    return run_cmd("rm -rf %s/%s", PREFIX_DIR, pkg);
}

int list_installed(void) {
    return run_cmd("ls %s", PREFIX_DIR);
}
