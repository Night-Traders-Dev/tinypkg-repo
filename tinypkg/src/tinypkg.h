#ifndef TINYPKG_H
#define TINYPKG_H

#include <stdbool.h>
#include <limits.h>

/* Use PATH_MAX for buffer sizes */
extern char TINYPKG_DIR[PATH_MAX];
extern char CACHE_DIR[PATH_MAX];
extern char BUILD_DIR[PATH_MAX];
extern char INSTALLED_DIR[PATH_MAX];
extern char PREFIX_DIR[PATH_MAX];

typedef struct {
    char name[64];
    char version[32];
    char description[256];
    char source[512];
    char build[1024];
    char install[1024];
    char checksum[128];
} Package;

int load_config(void);
void init_tinypkg();
int repo_update(void);
int load_manifest(const char *pkg, Package *p);

void download(const char *url, const char *out);
void run(const char *cmd, const char *dir);

int build_package(const char *pkg);
int install_package(const char *pkg);

#endif
