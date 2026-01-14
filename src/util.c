#include "util.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

int run_cmd(const char *fmt, ...) {
    char cmd[8192];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(cmd, sizeof(cmd), fmt, ap);
    va_end(ap);

    printf(">> %s\n", cmd);
    int rc = system(cmd);
    if (rc != 0) {
        fprintf(stderr, "Command failed: %s\n", cmd);
    }
    return rc;
}


int ensure_dir(const char *path) {
    if (mkdir(path, 0755) == 0) return 0;
    if (errno == EEXIST) return 0;
    perror(path);
    return 1;
}


int path_join(char *out, size_t max, const char *a, const char *b) {
    return snprintf(out, max, "%s/%s", a, b) >= (int)max;
}

int path_join3(char *out, size_t max, const char *a, const char *b, const char *c) {
    return snprintf(out, max, "%s/%s/%s", a, b, c) >= (int)max;
}

int path_join4(char *out, size_t max, const char *a, const char *b, const char *c, const char *d) {
    return snprintf(out, max, "%s/%s/%s/%s", a, b, c, d) >= (int)max;
}
