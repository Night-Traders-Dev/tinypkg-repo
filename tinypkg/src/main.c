/*
 * main.c - Entry point for tinypkg package manager
 * 
 * IMPROVEMENTS:
 * - Better usage messages
 * - Input validation at entry point
 * - Structured error reporting
 */

#include "common.h"
#include "repo.h"
#include "build.h"
#include "util.h"

void print_usage(const char *prog) {
    printf("Usage: %s [command] [args...]\n\n", prog);
    printf("Commands:\n");
    printf("  repo sync                 Synchronize package repository\n");
    printf("  repo add <url>            Add repository source\n");
    printf("  repo remove <name>        Remove repository source\n");
    printf("  search <term>             Search for packages\n");
    printf("  info <package>            Show detailed package info\n");
    printf("  list                      List all available packages\n");
    printf("  build <package>           Download and build a package\n");
    printf("  install <package>         Install a built package\n");
    printf("  remove <package>          Remove an installed package\n");
    printf("  help                      Show this help message\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s repo sync\n", prog);
    printf("  %s search sqlite\n", prog);
    printf("  %s build zlib\n", prog);
    printf("  %s install zlib\n", prog);
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char *cmd = argv[1];
    int ret = 0;
    
    /* Repository commands */
    if (strcmp(cmd, "repo") == 0) {
        if (argc < 3) {
            printf("Usage: %s repo [sync|add|remove]\n", argv[0]);
            return 1;
        }
        
        const char *subcmd = argv[2];
        
        if (strcmp(subcmd, "sync") == 0) {
            ret = repo_sync();
        } else if (strcmp(subcmd, "add") == 0) {
            if (argc < 4) {
                printf("Usage: %s repo add <url>\n", argv[0]);
                return 1;
            }
            ret = repo_add(argv[3]);
        } else if (strcmp(subcmd, "remove") == 0) {
            if (argc < 4) {
                printf("Usage: %s repo remove <name>\n", argv[0]);
                return 1;
            }
            ret = repo_remove(argv[3]);
        } else {
            printf("Unknown repo command: %s\n", subcmd);
            return 1;
        }
    }
    /* Search and info commands */
    else if (strcmp(cmd, "search") == 0) {
        if (argc < 3) {
            printf("Usage: %s search <term>\n", argv[0]);
            return 1;
        }
        ret = util_search(argv[2]);
    }
    else if (strcmp(cmd, "info") == 0) {
        if (argc < 3) {
            printf("Usage: %s info <package>\n", argv[0]);
            return 1;
        }
        
        if (!is_valid_package_name(argv[2])) {
            log_error("main", "Invalid package name");
            return 1;
        }
        
        ret = util_info(argv[2]);
    }
    else if (strcmp(cmd, "list") == 0) {
        ret = util_list();
    }
    /* Build/install commands */
    else if (strcmp(cmd, "build") == 0) {
        if (argc < 3) {
            printf("Usage: %s build <package>\n", argv[0]);
            return 1;
        }
        
        if (!is_valid_package_name(argv[2])) {
            log_error("main", "Invalid package name");
            return 1;
        }
        
        ret = build_package(argv[2]);
    }
    else if (strcmp(cmd, "install") == 0) {
        if (argc < 3) {
            printf("Usage: %s install <package>\n", argv[0]);
            return 1;
        }
        
        if (!is_valid_package_name(argv[2])) {
            log_error("main", "Invalid package name");
            return 1;
        }
        
        ret = install_package(argv[2]);
    }
    else if (strcmp(cmd, "remove") == 0) {
        if (argc < 3) {
            printf("Usage: %s remove <package>\n", argv[0]);
            return 1;
        }
        
        if (!is_valid_package_name(argv[2])) {
            log_error("main", "Invalid package name");
            return 1;
        }
        
        ret = remove_package(argv[2]);
    }
    /* Help command */
    else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "--help") == 0 ||
             strcmp(cmd, "-h") == 0) {
        print_usage(argv[0]);
        return 0;
    }
    /* Version */
    else if (strcmp(cmd, "--version") == 0 || strcmp(cmd, "-v") == 0) {
        printf("tinypkg version 0.2.0\n");
        return 0;
    }
    /* Unknown command */
    else {
        printf("Unknown command: %s\n\n", cmd);
        print_usage(argv[0]);
        return 1;
    }
    
    return ret == TINYPKG_OK ? 0 : 1;
}
