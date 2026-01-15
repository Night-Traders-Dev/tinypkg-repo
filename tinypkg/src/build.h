/*
 * build.h - Building and installing packages
 */

#ifndef BUILD_H
#define BUILD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Package manifest structure */
struct manifest {
    char name[128];
    char version[64];
    char source[512];           /* Download URL */
    char build_script[4096];    /* Build commands */
    char install_script[4096];  /* Install commands */
};

/* Main build operations */
int build_package(const char *name);       /* Download + compile */
int install_package(const char *name);     /* Install compiled binary */
int remove_package(const char *name);      /* Uninstall package */

/* Helper functions */
int parse_manifest(const char *name, struct manifest *m);
int download_source(const char *name, const char *url);
int extract_tarball(const char *name);
int execute_build(const char *name, struct manifest *m);
int execute_install(const char *name);
int track_installation(const char *name, const char *version);
int is_installed(const char *name);

#endif
