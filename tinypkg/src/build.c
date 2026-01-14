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
        return -1;
    }

    /* Create build dir */
    path_join(dir, sizeof(dir), BUILD_DIR, pkg);
    ensure_dir(dir);

    /* Path to downloaded tarball */
    path_join(tarball, sizeof(tarball), dir, "src.tar.gz");

    /* Download source */
    if (run_cmd("curl -L '%s' -o '%s'", p.source, tarball) != 0)
        return -1;

    /* Extract */
    if (run_cmd("cd '%s' && tar xf src.tar.gz", dir) != 0)
        return -1;

    /* Run build script from manifest */
    if (run_cmd("cd '%s'/* && %s", dir, p.build) != 0)
        return -1;

    return 0;
}
