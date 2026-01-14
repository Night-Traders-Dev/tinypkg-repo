#ifndef TINYPKG_H
#define TINYPKG_H

#include <stdbool.h>

#define MAX_PATH 4096

extern char TINYPKG_DIR[MAX_PATH];
extern char CACHE_DIR[MAX_PATH];
extern char BUILD_DIR[MAX_PATH];
extern char INSTALLED_DIR[MAX_PATH];
extern char PREFIX[MAX_PATH];

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
void update_repo();
int load_manifest(const char *pkg, Package *p);

void download(const char *url, const char *out);
void run(const char *cmd, const char *dir);

void build_package(const char *pkg);
void install_package(const char *pkg);

#endif
