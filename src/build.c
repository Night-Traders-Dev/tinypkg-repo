#include "build.h"
#include "config.h"
#include "util.h"

int build_package(const char *pkg) {
    char dir[4096];
    path_join(dir, sizeof(dir), BUILD_DIR, pkg);
    ensure_dir(dir);

    return run_cmd(
        "cd %s && "
        "curl -L $(grep url %s/repo/packages/%s/manifest.yaml | awk '{print $2}') -o src.tar.gz && "
        "tar xf src.tar.gz && "
        "cd * && ./configure --prefix=%s && make",
        dir, CACHE_DIR, pkg, PREFIX_DIR
    );
}
