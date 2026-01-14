// src/build.c
#include "build.h"
#include "config.h"
#include "util.h"
#include "yaml.h"
#include "tinypkg.h"

int build_package(const char *pkg)
{
    Package p;
    char dir[4096];
    char tarball[4096];

    /* Load manifest.yaml into struct */
    if (load_manifest(pkg, &p) != 0) {
        fprintf(stderr, "failed to load manifest for %s\n", pkg);
        return -1;
    }

    (void)p; /* keep variable used in release builds */

    /* Create build dir */
    if (path_join(dir, sizeof(dir), BUILD_DIR, pkg) != 0) return -1;
    if (ensure_dir(dir) != 0) return -1;

    /* Path to downloaded tarball */
    if (path_join(tarball, sizeof(tarball), dir, "src.tar.gz") != 0) return -1;

    /* Download source */
    if (run_cmd("curl -L '%s' -o '%s'", p.source, tarball) != 0)
        return -1;

    /* Extract */
    {
        char escdir[PATH_MAX*2];
        shell_escape(dir, escdir, sizeof(escdir));
        if (run_cmd("cd %s && tar xf src.tar.gz", escdir) != 0)
            return -1;
    }

    /* Find extracted subdirectory and run build script from manifest */
    {
        char child[PATH_MAX];
        if (find_first_subdir(dir, child, sizeof(child)) != 0) return -1;
        char escchild[PATH_MAX*2];
        shell_escape(child, escchild, sizeof(escchild));
        if (run_cmd("cd %s && %s", escchild, p.build) != 0)
            return -1;
    }

    return 0;
}
