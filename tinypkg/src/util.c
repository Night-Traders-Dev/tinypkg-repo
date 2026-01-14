#include "util.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

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
    char tmp[PATH_MAX];
    int n = snprintf(tmp, sizeof(tmp), "%s/%s", a, b);
    if (n < 0 || (size_t)n >= max) return 1;
    strncpy(out, tmp, max);
    out[max-1] = '\0';
    return 0;
}

int path_join3(char *out, size_t max, const char *a, const char *b, const char *c) {
    char tmp[PATH_MAX];
    int n = snprintf(tmp, sizeof(tmp), "%s/%s/%s", a, b, c);
    if (n < 0 || (size_t)n >= max) return 1;
    strncpy(out, tmp, max);
    out[max-1] = '\0';
    return 0;
}

int path_join4(char *out, size_t max, const char *a, const char *b, const char *c, const char *d) {
    char tmp[PATH_MAX];
    int n = snprintf(tmp, sizeof(tmp), "%s/%s/%s/%s", a, b, c, d);
    if (n < 0 || (size_t)n >= max) return 1;
    strncpy(out, tmp, max);
    out[max-1] = '\0';
    return 0;
}

/* Produce a single-quoted shell-escaped string in out. */
void shell_escape(const char *in, char *out, size_t max) {
    size_t w = 0;
    if (w + 1 < max) out[w++] = '\'';
    for (size_t i = 0; in[i] != '\0' && w + 4 < max; ++i) {
        if (in[i] == '\'') {
            /* close quote, add escaped single quote, reopen */
            if (w + 4 >= max) break;
            out[w++] = '\'';
            out[w++] = '\\';
            out[w++] = '\'';
            out[w++] = '\'';
        } else {
            out[w++] = in[i];
        }
    }
    if (w + 1 < max) out[w++] = '\'';
    out[w < max ? w : max-1] = '\0';
}

/* Find the first non-dot subdirectory inside `dir` and write its path to out. */
int find_first_subdir(const char *dir, char *out, size_t max) {
    DIR *d = opendir(dir);
    if (!d) return -1;
    struct dirent *ent;
    char candidate[PATH_MAX];
    int found = -1;
    while ((ent = readdir(d)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        if (snprintf(candidate, sizeof(candidate), "%s/%s", dir, ent->d_name) >= (int)sizeof(candidate)) continue;
        struct stat st;
        if (stat(candidate, &st) == 0 && S_ISDIR(st.st_mode)) {
            if (snprintf(out, max, "%s", candidate) >= (int)max) { found = -1; break; }
            found = 0;
            break;
        }
    }
    closedir(d);
    return found;
}

/* Write `block` to a temporary script, set PREFIX env var inside it, run it from
 * `workdir`, then remove the temp file. Returns script exit status (0 on success).
 */
int run_script_block(const char *block, const char *workdir, const char *prefix) {
    char tmpl[] = "/tmp/tinypkg-script-XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd < 0) return 1;
    FILE *f = fdopen(fd, "w");
    if (!f) { close(fd); unlink(tmpl); return 1; }

    char esc_prefix[PATH_MAX*2];
    shell_escape(prefix ? prefix : "", esc_prefix, sizeof(esc_prefix));

    fprintf(f, "#!/bin/sh\nset -e\nexport PREFIX=%s\n", esc_prefix);
    fprintf(f, "%s\n", block);
    fflush(f);
    fclose(f);
    if (chmod(tmpl, 0700) != 0) { unlink(tmpl); return 1; }

    char esc_work[PATH_MAX*2];
    char esc_script[PATH_MAX*2];
    shell_escape(workdir ? workdir : ".", esc_work, sizeof(esc_work));
    shell_escape(tmpl, esc_script, sizeof(esc_script));

    char logpath[PATH_MAX];
    if (path_join(logpath, sizeof(logpath), workdir, "tinypkg.log") != 0) {
        /* fallback: run without log */
        int rc = run_cmd("cd %s && sh %s", esc_work, esc_script);
        unlink(tmpl);
        return rc;
    }
    char esc_log[PATH_MAX*2];
    shell_escape(logpath, esc_log, sizeof(esc_log));

    int rc = run_cmd("cd %s && sh %s > %s 2>&1", esc_work, esc_script, esc_log);
    if (rc != 0) {
        run_cmd("echo '--- last 50 lines of %s ---' && tail -n 50 %s", esc_log, esc_log);
    }
    unlink(tmpl);
    return rc;
}
