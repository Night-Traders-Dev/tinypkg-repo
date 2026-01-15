/*
 * build.c - Building and installing packages
 *
 * Manages the complete build pipeline:
 * 1. Parse manifest (extract build/install scripts)
 * 2. Download source tarball
 * 3. Extract tarball
 * 4. Execute build commands
 * 5. Install binaries to ~/.local/bin/
 * 6. Track installation in database
 * 7. Remove/uninstall packages
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "build.h"
#include "util.h"
#include "repo.h"

/* Forward declarations for util.c functions we'll use */
extern char* get_cache_path(void);
#define BUILD_DIR ".cache/tinypkg/build"
#define LOCAL_BIN_DIR ".local/bin"
#define TINYPKG_DIR ".cache/tinypkg"

/* ============================================================================
 * Helper Functions - Path Management
 * ============================================================================
 */

/* Get home directory */
static char* get_home_dir(void) {
    static char home[512];
    const char *env_home = getenv("HOME");
    if (!env_home) {
        fprintf(stderr, "Error: HOME environment variable not set\n");
        return NULL;
    }
    strncpy(home, env_home, sizeof(home) - 1);
    home[sizeof(home) - 1] = '\0';
    return home;
}

/* Get build directory (~/tinypkg-build) */
static char* get_build_dir(void) {
    static char path[1024];
    const char *home = get_home_dir();
    if (!home) return NULL;
    snprintf(path, sizeof(path), "%s/%s", home, BUILD_DIR);
    return path;
}

/* Get local bin directory (~/.local/bin) */
static char* get_local_bin(void) {
    static char path[1024];
    const char *home = get_home_dir();
    if (!home) return NULL;
    snprintf(path, sizeof(path), "%s/%s", home , LOCAL_BIN_DIR);
    return path;
}

/* Get tinypkg metadata directory (~/.tinypkg) */
static char* get_tinypkg_dir(void) {
    static char path[1024];
    const char *home = get_home_dir();
    if (!home) return NULL;
    snprintf(path, sizeof(path), "%s/%s", home, TINYPKG_DIR);
    return path;
}

/* Create directory and parents (like mkdir -p) */
static int mkdir_p(const char *path) {
    char tmp[1024];
    char *p = NULL;
    
    strncpy(tmp, path, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                perror("mkdir");
                return -1;
            }
            *p = '/';
        }
    }
    
    /* Create final directory */
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        perror("mkdir");
        return -1;
    }
    
    return 0;
}

/* ============================================================================
 * Phase 1: Parse Manifest
 * ============================================================================
 */

int parse_manifest(const char *name, struct manifest *m) {
    char *cache = get_cache_path();
    char manifest_path[2048];
    FILE *f;
    char line[512];

    if (!name || !m) return -1;

    snprintf(manifest_path, sizeof(manifest_path),
             "%s/repo/packages/%s/manifest.yaml", cache, name);

    f = fopen(manifest_path, "r");
    if (!f) {
        fprintf(stderr, "Error: Package '%s' manifest not found\n", name);
        return -1;
    }

    memset(m, 0, sizeof(*m));
    strncpy(m->name, name, sizeof(m->name) - 1);
    m->name[sizeof(m->name) - 1] = '\0';

    /* Parse YAML manifest */
    while (fgets(line, sizeof(line), f)) {
        /* Extract version */
        if (strstr(line, "version:")) {
            char *start = strchr(line, ':') + 1;
            while (*start == ' ') start++;
            sscanf(start, "%63s", m->version);
        }

        /* Extract source URL */
        if (strstr(line, "source:")) {
            char *start = strchr(line, ':') + 1;
            while (*start == ' ') start++;
            sscanf(start, "%511s", m->source);
        }

        /* Extract build script (multi-line) */
        if (strstr(line, "build:") && line[0] != ' ') {
            size_t offset = 0;
            while (fgets(line, sizeof(line), f)) {
                /* Stop at next top-level key or end of file */
                if (line[0] != ' ' || (line[0] == ' ' && line[1] != ' ')) {
                    break;
                }
                if (offset + strlen(line) < sizeof(m->build_script) - 1) {
                    strcat(m->build_script, line);
                    offset += strlen(line);
                }
            }
            continue;
        }

        /* Extract install script (multi-line) */
        if (strstr(line, "install:") && line[0] != ' ') {
            size_t offset = 0;
            while (fgets(line, sizeof(line), f)) {
                /* Stop at next top-level key or end of file */
                if (line[0] != ' ' || (line[0] == ' ' && line[1] != ' ')) {
                    break;
                }
                if (offset + strlen(line) < sizeof(m->install_script) - 1) {
                    strcat(m->install_script, line);
                    offset += strlen(line);
                }
            }
            break;
        }
    }

    fclose(f);

    if (m->source[0] == 0) {
        fprintf(stderr, "Error: No source URL found in manifest\n");
        return -1;
    }

    return 0;
}

/* ============================================================================
 * Phase 2: Download Source
 * ============================================================================
 */

int download_source(const char *name, const char *url) {
    char *build_base = get_build_dir();
    char pkg_dir[1024];
    char cmd[4096];
    int ret;

    if (!build_base) return -1;

    snprintf(pkg_dir, sizeof(pkg_dir), "%s/%s", build_base, name);

    /* Create package directory with parent directories */
    if (mkdir_p(pkg_dir) != 0) {
        fprintf(stderr, "Error: Failed to create build directory\n");
        return -1;
    }

    printf("Downloading %s from %s...\n", name, url);

    /* Download with wget or curl */
    snprintf(cmd, sizeof(cmd),
             "cd %s && wget -q --show-progress %s -O source.tar.gz 2>&1",
             pkg_dir, url);

    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Error: Failed to download source\n");
        return -1;
    }

    printf("✓ Downloaded to %s/source.tar.gz\n", pkg_dir);
    return 0;
}

/* ============================================================================
 * Phase 3: Extract Tarball
 * ============================================================================
 */

int extract_tarball(const char *name) {
    char *build_base = get_build_dir();
    char pkg_dir[1024];
    char cmd[4096];
    int ret;

    if (!build_base) return -1;

    snprintf(pkg_dir, sizeof(pkg_dir), "%s/%s", build_base, name);

    printf("Extracting source...\n");

    snprintf(cmd, sizeof(cmd), "cd %s && tar -xzf source.tar.gz", pkg_dir);

    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Error: Failed to extract tarball\n");
        return -1;
    }

    printf("✓ Extracted to %s/\n", pkg_dir);
    return 0;
}

/* ============================================================================
 * Phase 4: Execute Build
 * ============================================================================
 */

int execute_build(const char *name, struct manifest *m) {
    char *build_base = get_build_dir();
    char *home = get_home_dir();
    char pkg_dir[1024];
    char prefix[1024];
    char cmd[8192];
    int ret;

    if (!build_base || !home) return -1;

    snprintf(pkg_dir, sizeof(pkg_dir), "%s/%s", build_base, name);
    snprintf(prefix, sizeof(prefix), "%s/tinypkg-build/%s/PKG", home, name);

    /* Create PKG (installation prefix) directory with parents */
    if (mkdir_p(prefix) != 0) {
        fprintf(stderr, "Error: Failed to create prefix directory\n");
        return -1;
    }

    printf("Building %s...\n", name);

    /* Execute build script with PREFIX set */
    snprintf(cmd, sizeof(cmd),
             "cd %s && PREFIX=%s /bin/bash -c '%s' 2>&1",
             pkg_dir, prefix, m->build_script);

    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Error: Build failed\n");
        return -1;
    }

    printf("✓ Build complete: %s/bin/%s\n", prefix, name);
    return 0;
}

/* ============================================================================
 * Phase 5: Execute Install
 * ============================================================================
 */

int execute_install(const char *name) {
    char *build_base = get_build_dir();
    char *home = get_home_dir();
    char *local_bin = get_local_bin();
    char pkg_bin[1024];
    char install_bin[1024];
    char cmd[4096];
    struct stat st;

    if (!build_base || !home || !local_bin) return -1;

    snprintf(pkg_bin, sizeof(pkg_bin),
             "%s/%s/PKG/bin/%s", build_base, name, name);
    snprintf(install_bin, sizeof(install_bin), "%s/%s", local_bin, name);

    /* Check if binary was built */
    if (stat(pkg_bin, &st) != 0) {
        fprintf(stderr, "Error: Package not built. Run 'tinypkg build %s' first\n", name);
        return -1;
    }

    /* Create ~/.local/bin if it doesn't exist */
    if (mkdir_p(local_bin) != 0) {
        fprintf(stderr, "Error: Failed to create local bin directory\n");
        return -1;
    }

    printf("Installing %s to %s...\n", name, install_bin);

    /* Copy binary */
    snprintf(cmd, sizeof(cmd), "cp %s %s", pkg_bin, install_bin);
    if (system(cmd) != 0) {
        fprintf(stderr, "Error: Failed to copy binary\n");
        return -1;
    }

    /* Make executable */
    chmod(install_bin, 0755);

    /* Track installation */
    if (track_installation(name, "") != 0) {
        fprintf(stderr, "Warning: Could not track installation\n");
    }

    printf("✓ Installed to %s\n", install_bin);
    return 0;
}

/* ============================================================================
 * Phase 6: Track Installation
 * ============================================================================
 */

int track_installation(const char *name, const char *version) {
    char *tinypkg_dir = get_tinypkg_dir();
    char db_path[1024];
    FILE *f;

    if (!tinypkg_dir) return -1;

    /* Create ~/.tinypkg if needed */
    if (mkdir_p(tinypkg_dir) != 0) {
        fprintf(stderr, "Error: Failed to create tinypkg directory\n");
        return -1;
    }

    snprintf(db_path, sizeof(db_path), "%s/installed.db", tinypkg_dir);

    /* Append to installed.db */
    f = fopen(db_path, "a");
    if (!f) {
        perror("fopen");
        return -1;
    }

    fprintf(f, "%s %s\n", name, version ? version : "unknown");
    fclose(f);

    return 0;
}

/* ============================================================================
 * Check if Package is Installed
 * ============================================================================
 */

int is_installed(const char *name) {
    char *tinypkg_dir = get_tinypkg_dir();
    char db_path[1024];
    FILE *f;
    char line[256];

    if (!tinypkg_dir) return 0;

    snprintf(db_path, sizeof(db_path), "%s/installed.db", tinypkg_dir);

    f = fopen(db_path, "r");
    if (!f) return 0;  /* Not installed */

    while (fgets(line, sizeof(line), f)) {
        char pkg[128] = {0};
        sscanf(line, "%127s", pkg);
        if (strcmp(pkg, name) == 0) {
            fclose(f);
            return 1;  /* Installed */
        }
    }

    fclose(f);
    return 0;
}

/* ============================================================================
 * Phase 7: Remove Package
 * ============================================================================
 */

int remove_package_impl(const char *name) {
    char *local_bin = get_local_bin();
    char *tinypkg_dir = get_tinypkg_dir();
    char binary_path[1024];
    char db_path[1024];
    char tmp_path[1024];
    FILE *in, *out;
    char line[256];
    int found = 0;

    if (!local_bin || !tinypkg_dir) return -1;

    snprintf(binary_path, sizeof(binary_path), "%s/%s", local_bin, name);
    snprintf(db_path, sizeof(db_path), "%s/installed.db", tinypkg_dir);
    snprintf(tmp_path, sizeof(tmp_path), "%s/temp.db", tinypkg_dir);

    /* Remove binary */
    if (unlink(binary_path) == 0) {
        printf("✓ Removed %s\n", binary_path);
    } else {
        fprintf(stderr, "Warning: Could not remove %s\n", binary_path);
    }

    /* Update database (remove entry) */
    in = fopen(db_path, "r");
    out = fopen(tmp_path, "w");

    if (in && out) {
        while (fgets(line, sizeof(line), in)) {
            char pkg[128] = {0};
            sscanf(line, "%127s", pkg);
            if (strcmp(pkg, name) != 0) {
                fputs(line, out);
            } else {
                found = 1;
            }
        }
        fclose(in);
        fclose(out);

        if (found) {
            rename(tmp_path, db_path);
        } else {
            unlink(tmp_path);
        }
    }

    printf("✓ %s uninstalled\n", name);
    return 0;
}

/* ============================================================================
 * Main Entry Points
 * ============================================================================
 */

int build_package(const char *name) {
    struct manifest m;

    if (!name) {
        fprintf(stderr, "Error: package name required\n");
        return -1;
    }

    printf("=== Building %s ===\n\n", name);

    /* Step 1: Parse manifest */
    if (parse_manifest(name, &m) != 0) {
        return -1;
    }

    printf("Version: %s\n", m.version);
    printf("Source: %s\n\n", m.source);

    /* Step 2: Download source */
    if (download_source(name, m.source) != 0) {
        return -1;
    }

    /* Step 3: Extract */
    if (extract_tarball(name) != 0) {
        return -1;
    }

    /* Step 4: Build */
    if (execute_build(name, &m) != 0) {
        return -1;
    }

    printf("\n✓ Build complete!\n");
    printf("Next: tinypkg install %s\n", name);
    return 0;
}

int install_package(const char *name) {
    if (!name) {
        fprintf(stderr, "Error: package name required\n");
        return -1;
    }

    printf("=== Installing %s ===\n\n", name);

    if (execute_install(name) != 0) {
        return -1;
    }

    printf("\n✓ Installation complete!\n");
    return 0;
}

int remove_package(const char *name) {
    if (!name) {
        fprintf(stderr, "Error: package name required\n");
        return -1;
    }

    printf("=== Removing %s ===\n\n", name);

    if (remove_package_impl(name) != 0) {
        return -1;
    }

    return 0;
}
