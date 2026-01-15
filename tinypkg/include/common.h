/*
 * common.h - Shared utilities and constants for tinypkg
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>

/* Path definitions */
#define CACHE_DIR ".cache/tinypkg"
#define BUILD_DIR ".cache/tinypkg/build"
#define LOCAL_BIN_DIR ".local/bin"
#define TINYPKG_DIR ".cache/tinypkg"
#define REPO_URL "https://github.com/Night-Traders-Dev/tinypkg-repo.git"

/* Buffer sizes */
#define PATH_MAX_LEN 1024
#define CMD_MAX_LEN 8192
#define MANIFEST_MAX_LEN 4096
#define LINE_MAX_LEN 512

/* Return codes */
#define TINYPKG_OK 0
#define TINYPKG_ERR -1
#define TINYPKG_NOT_FOUND -2

/* Function declarations */
char* get_home_dir(void);
char* get_cache_path(void);
char* get_build_dir(void);
char* get_local_bin(void);
char* get_tinypkg_dir(void);
int mkdir_p(const char *path);
int is_valid_package_name(const char *name);
int safe_execute(char *const argv[]);
int safe_execute_in_dir(const char *workdir, char *const argv[]);
void log_error(const char *func, const char *msg);
void log_info(const char *msg);
void log_warn(const char *msg);

#endif
