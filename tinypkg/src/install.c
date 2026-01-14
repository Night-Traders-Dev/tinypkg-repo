// src/install.c
#include "install.h"
#include "config.h"
#include "util.h"
#include "yaml.h"
#include "tinypkg.h"

int install_package(const char *pkg)
{
    Package p;
    char dir[4096];

    if (load_manifest(pkg, &p) != 0) {
        return -1;
    }

    if (path_join(dir, sizeof(dir), BUILD_DIR, pkg) != 0) return -1;

    char child[PATH_MAX];
    if (find_first_subdir(dir, child, sizeof(child)) != 0) return -1;
    char escchild[PATH_MAX*2];
    shell_escape(child, escchild, sizeof(escchild));
    return run_cmd("cd %s && %s", escchild, p.install);
}

int remove_package(const char *pkg) {
    return run_cmd("rm -rf %s/%s", PREFIX_DIR, pkg);
}

int list_installed(void) {
    return run_cmd("ls %s", PREFIX_DIR);
}
