/*
 * main.c - Entry point for tinypkg
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "repo.h"
#include "build.h"
#include "util.h"

void print_usage(const char *prog) {
    printf("Usage: %s <command> [args...]\n\n", prog);
    printf("Commands:\n");
    printf("  repo sync              Synchronize repository\n");
    printf("  repo add <url>         Add repository\n");
    printf("  repo remove <name>     Remove repository\n");
    printf("  search <term>          Search for packages\n");
    printf("  info <name>            Show package info\n");
    printf("  list                   List all packages\n");
    printf("  build <name>           Build a package\n");
    printf("  install <name>         Install a package\n");
    printf("  remove <name>          Remove a package\n");
    printf("  help                   Show this help\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *cmd = argv[1];

    /* Repository commands */
    if (strcmp(cmd, "repo") == 0) {
        if (argc < 3) {
            printf("Usage: %s repo <sync|add|remove> [args...]\n", argv[0]);
            return 1;
        }
        const char *subcmd = argv[2];
        
        if (strcmp(subcmd, "sync") == 0) {
            return repo_sync();
        } else if (strcmp(subcmd, "add") == 0) {
            if (argc < 4) {
                printf("Usage: %s repo add <url>\n", argv[0]);
                return 1;
            }
            return repo_add(argv[3]);
        } else if (strcmp(subcmd, "remove") == 0) {
            if (argc < 4) {
                printf("Usage: %s repo remove <name>\n", argv[0]);
                return 1;
            }
            return repo_remove(argv[3]);
        }
    }

    /* Search and info commands */
    else if (strcmp(cmd, "search") == 0) {
        if (argc < 3) {
            printf("Usage: %s search <term>\n", argv[0]);
            return 1;
        }
        return util_search(argv[2]);
    } else if (strcmp(cmd, "info") == 0) {
        if (argc < 3) {
            printf("Usage: %s info <name>\n", argv[0]);
            return 1;
        }
        return util_info(argv[2]);
    } else if (strcmp(cmd, "list") == 0) {
        return util_list();
    }

    /* Build/install commands */
    else if (strcmp(cmd, "build") == 0) {
        if (argc < 3) {
            printf("Usage: %s build <name>\n", argv[0]);
            return 1;
        }
        return build_package(argv[2]);
    } else if (strcmp(cmd, "install") == 0) {
        if (argc < 3) {
            printf("Usage: %s install <name>\n", argv[0]);
            return 1;
        }
        return install_package(argv[2]);
    } else if (strcmp(cmd, "remove") == 0) {
        if (argc < 3) {
            printf("Usage: %s remove <name>\n", argv[0]);
            return 1;
        }
        return remove_package(argv[2]);
    }

    /* Help command */
    else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    /* Unknown command */
    else {
        printf("Unknown command: %s\n", cmd);
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}
