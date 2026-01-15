/*
 * repo.c - Repository management (downloading and storing)
 * 
 * Syncs packages from https://github.com/Night-Traders-Dev/tinypkg-repo
 * Caches them locally in ~/.cache/tinypkg/
 * 
 * SECURITY IMPROVEMENTS:
 * - Uses safe_execute() instead of system() for git commands
 * - Validates repository paths
 * - Proper error handling with context
 */

#include "common.h"
#include "repo.h"
#include <yaml.h>

/* Create directory if it doesn't exist */
static int ensure_dir(const char *path) {
    struct stat st;
    
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return TINYPKG_OK;
        }
        log_error("ensure_dir", "Path exists but is not a directory");
        return TINYPKG_ERR;
    }
    
    return mkdir_p(path);
}

/* Clone or update the git repository using safe_execute */
int repo_clone_or_pull(void) {
    char *cache = get_cache_path();
    char repo_path[PATH_MAX_LEN];
    struct stat st;
    
    if (!cache) {
        log_error("repo_clone_or_pull", "Failed to get cache path");
        return TINYPKG_ERR;
    }
    
    snprintf(repo_path, PATH_MAX_LEN, "%s/repo", cache);
    
    printf("Repository cache: %s\n", repo_path);
    
    /* Check if repo already exists */
    if (stat(repo_path, &st) == 0 && S_ISDIR(st.st_mode)) {
        /* Repository exists, update it */
        printf("Updating existing repository...\n");
        
        char *argv[] = { "git", "-C", repo_path, "pull", NULL };
        int ret = safe_execute(argv);
        if (ret != TINYPKG_OK) {
            log_error("repo_clone_or_pull", "git pull failed");
            return TINYPKG_ERR;
        }
    } else {
        /* Repository doesn't exist, clone it */
        printf("Cloning repository from %s...\n", REPO_URL);
        
        if (ensure_dir(cache) != TINYPKG_OK) {
            log_error("repo_clone_or_pull", "Failed to create cache directory");
            return TINYPKG_ERR;
        }
        
        char *argv[] = { "git", "clone", REPO_URL, repo_path, NULL };
        int ret = safe_execute(argv);
        if (ret != TINYPKG_OK) {
            log_error("repo_clone_or_pull", "git clone failed");
            return TINYPKG_ERR;
        }
    }
    
    printf("✓ Repository synced successfully\n");
    return TINYPKG_OK;
}

/* Parse packages/index.yaml and display available packages */
int repo_parse_index(void) {
    char *cache = get_cache_path();
    char index_path[PATH_MAX_LEN];
    FILE *f;
    char line[LINE_MAX_LEN];
    int in_packages = 0;
    
    if (!cache) {
        log_error("repo_parse_index", "Failed to get cache path");
        return TINYPKG_ERR;
    }
    
    snprintf(index_path, PATH_MAX_LEN, "%s/repo/packages/index.yaml", cache);
    
    f = fopen(index_path, "r");
    if (!f) {
        log_error("repo_parse_index", "Could not open index.yaml");
        return TINYPKG_ERR;
    }
    
    printf("\nAvailable packages:\n");
    printf("-------------------\n");
    
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        
        /* Skip leading whitespace */
        while (*p && (*p == ' ' || *p == '\t')) p++;
        
        /* Look for "packages:" section */
        if (!in_packages) {
            if (strncmp(p, "packages:", 9) == 0) {
                in_packages = 1;
            }
            continue;
        }
        
        /* Parse package entries (two-space indented, name followed by colon) */
        if (line[0] == ' ' && line[1] == ' ' && line[2] != ' ' && line[2] != '\0') {
            char name[128] = {0};
            char version[64] = {0};
            char desc[256] = {0};
            
            /* Extract package name */
            if (sscanf(line + 2, "%127[^:]:", name) == 1) {
                /* Try to find version and description on following lines */
                char next_line[LINE_MAX_LEN];
                
                while (fgets(next_line, sizeof(next_line), f)) {
                    if (next_line[0] != ' ' || next_line[1] != ' ' || next_line[2] == ' ') {
                        break; /* End of this package entry */
                    }
                    
                    if (strstr(next_line, "version:")) {
                        sscanf(next_line, "%*[^:]:%*[ ]%63s", version);
                    }
                    
                    if (strstr(next_line, "description:")) {
                        sscanf(next_line, "%*[^:]:%*[ ]%255[^\n]", desc);
                    }
                }
                
                /* Print package info */
                printf(" %s (%s)\n", name, version[0] ? version : "unknown");
                if (desc[0]) {
                    printf(" %s\n", desc);
                }
            }
        }
    }
    
    fclose(f);
    printf("-------------------\n\n");
    
    return TINYPKG_OK;
}

/* Cache downloaded packages locally */
int repo_cache_packages(void) {
    char *cache = get_cache_path();
    char src[PATH_MAX_LEN];
    
    if (!cache) {
        log_error("repo_cache_packages", "Failed to get cache path");
        return TINYPKG_ERR;
    }
    
    snprintf(src, PATH_MAX_LEN, "%s/repo/packages", cache);
    
    printf("Caching package data...\n");
    
    /* Copy index.yaml to cache root */
    char *argv[] = { "cp", "index.yaml", cache, NULL };
    int ret = safe_execute_in_dir(src, argv);
    
    if (ret != TINYPKG_OK) {
        log_warn("Could not cache index.yaml");
    }
    
    printf("Package data cached at %s/\n", cache);
    return TINYPKG_OK;
}

/* Main sync function */
int repo_sync(void) {
    printf("=== Synchronizing tinypkg repository ===\n\n");
    
    /* Step 1: Clone or pull repository */
    if (repo_clone_or_pull() != TINYPKG_OK) {
        return TINYPKG_ERR;
    }
    
    printf("\n");
    
    /* Step 2: Parse index and list packages */
    if (repo_parse_index() != TINYPKG_OK) {
        return TINYPKG_ERR;
    }
    
    /* Step 3: Cache package data locally */
    if (repo_cache_packages() != TINYPKG_OK) {
        return TINYPKG_ERR;
    }
    
    printf("\n✓ Repository sync complete!\n");
    return TINYPKG_OK;
}

/* Add a repository source */
int repo_add(const char *url) {
    if (!url || !url[0]) {
        log_error("repo_add", "URL required");
        return TINYPKG_ERR;
    }
    
    /* Validate URL basic format */
    if (strncmp(url, "http://", 7) != 0 && strncmp(url, "https://", 8) != 0) {
        log_error("repo_add", "URL must start with http:// or https://");
        return TINYPKG_ERR;
    }
    
    printf("repo_add: adding repository from %s\n", url);
    printf("(Not yet implemented - will support multiple repositories soon)\n");
    return TINYPKG_OK;
}

/* Remove a repository source */
int repo_remove(const char *name) {
    if (!name || !name[0]) {
        log_error("repo_remove", "Repository name required");
        return TINYPKG_ERR;
    }
    
    printf("repo_remove: removing repository %s\n", name);
    printf("(Not yet implemented)\n");
    return TINYPKG_OK;
}
