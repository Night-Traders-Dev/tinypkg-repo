#include "repo.h"
#include "config.h"
#include "util.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

int repo_list(void) {
    char path[PATH_MAX];
    if (path_join(path, sizeof(path), CACHE_DIR, "repo/packages/index.yaml") != 0) return 1;
    FILE *f = fopen(path, "r");
    if (!f) return run_cmd("ls %s/repo/packages", CACHE_DIR);
    char line[1024];
    int in_packages = 0;
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (*p && isspace((unsigned char)*p)) p++;
        if (!in_packages) {
            if (strncmp(p, "packages:", 9) == 0) in_packages = 1;
            continue;
        }
        /* look for package keys: two-space indent then name: */
        if (line[0] == ' ' && line[1] == ' ' && !isspace((unsigned char)line[2])) {
            char name[256];
            if (sscanf(line+2, "%255[^:] :", name) == 1) {
                printf("%s\n", name);
            } else {
                /* try without space before colon */
                if (sscanf(line+2, "%255[^:]%*1[:]", name) == 1) printf("%s\n", name);
            }
        }
    }
    fclose(f);
    return 0;
}

int repo_search(const char *term) {
    char path[PATH_MAX];
    if (path_join(path, sizeof(path), CACHE_DIR, "repo/packages/index.yaml") != 0) return 1;
    FILE *f = fopen(path, "r");
    if (!f) return 1;
    char line[1024];
    int in_packages = 0;
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (*p && isspace((unsigned char)*p)) p++;
        if (!in_packages) {
            if (strncmp(p, "packages:", 9) == 0) in_packages = 1;
            continue;
        }
        if (line[0] == ' ' && line[1] == ' ' && !isspace((unsigned char)line[2])) {
            char name[256];
            if (sscanf(line+2, "%255[^:] :", name) == 1 || sscanf(line+2, "%255[^:]%*1[:]", name) == 1) {
                if (strcasestr(name, term)) printf("%s\n", name);
            }
        } else if (in_packages && isspace((unsigned char)line[0])) {
            /* description or version lines; check if term in them and print last seen package */
            /* Not implemented: keep simple name-only search */
        }
    }
    fclose(f);
    return 0;
}

int repo_info(const char *pkg) {
    char path[PATH_MAX];
    char esc[PATH_MAX * 2];
    if (path_join(path, sizeof(path), CACHE_DIR, "repo/packages") != 0) return 1;
    if (path_join(path, sizeof(path), path, pkg) != 0) return 1;
    if (path_join(path, sizeof(path), path, "manifest.yaml") != 0) return 1;
    shell_escape(path, esc, sizeof(esc));
    return run_cmd("cat %s", esc);
}
