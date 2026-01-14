#include "config.h"
#include "git.h"
#include "repo.h"
#include "build.h"
#include "install.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    load_config();

    if (argc < 2) {
        printf("tinypkg: init, update, install, remove, list, search, info\n");
        return 1;
    }

    if (!strcmp(argv[1], "init")) return 0;
    if (!strcmp(argv[1], "update")) return repo_update();
    if (!strcmp(argv[1], "list")) return list_installed();
    if (!strcmp(argv[1], "search")) {
    if (argc < 3) {
        printf("usage: tinypkg search <term>\n");
        return 1;
    }
    return repo_search(argv[2]);
}
    if (!strcmp(argv[1], "info")) {
        if (argc < 3) {
            printf("usage: tinypkg info <pkg>\n");
            return 1;
        }
        return repo_info(argv[2]);
    }

    if (!strcmp(argv[1], "install")) {
        if (argc < 3) {
            printf("usage: tinypkg install <pkg>\n");
            return 1;
        }
        if (repo_update() != 0) return 1;
        if (build_package(argv[2]) != 0) return 1;
        return install_package(argv[2]);
    }

    if (!strcmp(argv[1], "remove")) {
        if (argc < 3) {
            printf("usage: tinypkg remove <pkg>\n");
            return 1;
        }
        return remove_package(argv[2]);
    }

    return 0;
}
