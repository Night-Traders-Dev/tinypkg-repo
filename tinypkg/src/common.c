#define _POSIX_C_SOURCE 200809L

#include "common.h"

static __thread char home_dir[PATH_MAX_LEN];
static __thread char cache_path[PATH_MAX_LEN];
static __thread char build_dir[PATH_MAX_LEN];
static __thread char local_bin[PATH_MAX_LEN];
static __thread char tinypkg_dir[PATH_MAX_LEN];
static __thread int paths_initialized = 0;

/* dst = a + "/" + b */
static void build_path(char *dst, size_t dst_len, const char *a, const char *b)
{
    size_t a_len = strnlen(a, dst_len);
    size_t b_len = strlen(b);

    if (a_len + 1 + b_len + 1 > dst_len) {
        log_error("build_path", "Path too long");
        exit(1);
    }

    memcpy(dst, a, a_len);
    dst[a_len] = '/';
    memcpy(dst + a_len + 1, b, b_len);
    dst[a_len + 1 + b_len] = '\0';
}

static void init_paths(void)
{
    if (paths_initialized)
        return;

    const char *env_home = getenv("HOME");
    if (!env_home || !*env_home)
        env_home = ".";

    size_t home_len = strnlen(env_home, PATH_MAX_LEN);
    if (home_len + 1 >= PATH_MAX_LEN) {
        log_error("init_paths", "HOME too long");
        exit(1);
    }

    memcpy(home_dir, env_home, home_len);
    home_dir[home_len] = '\0';

    build_path(cache_path,  PATH_MAX_LEN, home_dir, CACHE_DIR);
    build_path(build_dir,   PATH_MAX_LEN, home_dir, BUILD_DIR);
    build_path(local_bin,   PATH_MAX_LEN, home_dir, LOCAL_BIN_DIR);
    build_path(tinypkg_dir, PATH_MAX_LEN, home_dir, TINYPKG_DIR);

    paths_initialized = 1;
}

/* Accessors */

char* get_home_dir(void)
{
    init_paths();
    return home_dir;
}

char* get_cache_path(void)
{
    init_paths();
    return cache_path;
}

char* get_build_dir(void)
{
    init_paths();
    return build_dir;
}

char* get_local_bin(void)
{
    init_paths();
    return local_bin;
}

char* get_tinypkg_dir(void)
{
    init_paths();
    return tinypkg_dir;
}

/* mkdir -p */
int mkdir_p(const char *path)
{
    if (!path || !path[0])
        return TINYPKG_ERR;

    char tmp[PATH_MAX_LEN];
    size_t len = strnlen(path, PATH_MAX_LEN);

    if (len >= PATH_MAX_LEN)
        return TINYPKG_ERR;

    memcpy(tmp, path, len + 1);

    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, 0755) && errno != EEXIST) {
                log_error("mkdir_p", strerror(errno));
                return TINYPKG_ERR;
            }
            *p = '/';
        }
    }

    if (mkdir(tmp, 0755) && errno != EEXIST) {
        log_error("mkdir_p", strerror(errno));
        return TINYPKG_ERR;
    }

    return TINYPKG_OK;
}

/* Package name validation */
int is_valid_package_name(const char *name)
{
    if (!name || !*name)
        return 0;

    if (strlen(name) > 128)
        return 0;

    for (size_t i = 0; name[i]; i++) {
        unsigned char c = (unsigned char)name[i];
        if (!isalnum(c) && c != '-' && c != '_')
            return 0;
    }

    return 1;
}

/* Process execution */
int safe_execute(char *const argv[])
{
    if (!argv || !argv[0]) {
        log_error("safe_execute", "Invalid argv");
        return TINYPKG_ERR;
    }

    pid_t pid = fork();
    if (pid < 0) {
        log_error("fork", strerror(errno));
        return TINYPKG_ERR;
    }

    if (pid == 0) {
        execvp(argv[0], argv);
        perror(argv[0]);
        _exit(127);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0)
        return TINYPKG_ERR;

    return (WIFEXITED(status) && WEXITSTATUS(status) == 0)
           ? TINYPKG_OK
           : TINYPKG_ERR;
}

int safe_execute_in_dir(const char *dir, char *const argv[])
{
    if (!dir || !argv || !argv[0])
        return TINYPKG_ERR;

    pid_t pid = fork();
    if (pid < 0)
        return TINYPKG_ERR;

    if (pid == 0) {
        if (chdir(dir) != 0) {
            perror("chdir");
            _exit(127);
        }
        execvp(argv[0], argv);
        perror(argv[0]);
        _exit(127);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0)
        return TINYPKG_ERR;

    return (WIFEXITED(status) && WEXITSTATUS(status) == 0)
           ? TINYPKG_OK
           : TINYPKG_ERR;
}

/* Logging */
void log_error(const char *func, const char *msg)
{
    fprintf(stderr, "[ERROR] %s: %s\n", func, msg);
}

void log_info(const char *msg)
{
    fprintf(stdout, "[INFO] %s\n", msg);
}

void log_warn(const char *msg)
{
    fprintf(stdout, "[WARN] %s\n", msg);
}
