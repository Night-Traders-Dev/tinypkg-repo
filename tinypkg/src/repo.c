/*
 * repo.c - Repository management (downloading and storing)
 * 
 * Syncs packages from https://github.com/Night-Traders-Dev/tinypkg-repo
 * Caches them locally in ~/.cache/tinypkg/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define REPO_URL "https://github.com/Night-Traders-Dev/tinypkg-repo.git"
#define CACHE_DIR ".cache/tinypkg"


/* Get home directory and build cache path */
char* get_cache_path(void) {
    static char path[512];
    const char *home = getenv("HOME");
    if (!home)
        home = ".";
    snprintf(path, sizeof(path), "%s/%s", home, CACHE_DIR);
    return path;
}

/* Create directory if it doesn't exist */
static int ensure_dir(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0)
        return 0;  /* Directory exists */
    
    if (mkdir(path, 0755) == -1)
        return -1;
    return 0;
}

/* Clone or update the git repository */
int repo_clone_or_pull(void) {
    char *cache = get_cache_path();
    char repo_path[512];
    char cmd[1024];
    
    snprintf(repo_path, sizeof(repo_path), "%s/repo", cache);
    
    printf("Repository cache: %s\n", repo_path);
    
    /* Check if repo already exists */
    struct stat st;
    if (stat(repo_path, &st) == 0) {
        /* Repository exists, update it */
        printf("Updating existing repository...\n");
        snprintf(cmd, sizeof(cmd), "cd %s && git pull", repo_path);
    } else {
        /* Repository doesn't exist, clone it */
        printf("Cloning repository from %s...\n", REPO_URL);
        ensure_dir(cache);
        snprintf(cmd, sizeof(cmd), "git clone %s %s", REPO_URL, repo_path);
    }
    
    int ret = system(cmd);
    if (ret != 0) {
        printf("Failed to clone/pull repository\n");
        return -1;
    }
    
    printf("Repository synced successfully\n");
    return 0;
}

/* Parse packages/index.yaml and display available packages */
int repo_parse_index(void) {
    char *cache = get_cache_path();
    char index_path[512];
    FILE *f;
    char line[256];
    int in_packages = 0;
    
    snprintf(index_path, sizeof(index_path), "%s/repo/packages/index.yaml", cache);
    
    f = fopen(index_path, "r");
    if (!f) {
        printf("Error: Could not open index at %s\n", index_path);
        return -1;
    }
    
    printf("\nAvailable packages:\n");
    printf("-------------------\n");
    
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        
        /* Skip leading whitespace */
        while (*p && (*p == ' ' || *p == '\t'))
            p++;
        
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
                char next_line[256];
                while (fgets(next_line, sizeof(next_line), f)) {
                    if (next_line[0] != ' ' || next_line[1] != ' ' || next_line[2] == ' ')
                        break;  /* End of this package entry */
                    
                    if (strstr(next_line, "version:")) {
                        sscanf(next_line, "%*[^:]:%*[ ]%63s", version);
                    }
                    if (strstr(next_line, "description:")) {
                        sscanf(next_line, "%*[^:]:%*[ ]%255[^\n]", desc);
                    }
                }
                
                /* Print package info */
                printf("  %s (%s)\n", name, version[0] ? version : "unknown");
                if (desc[0])
                    printf("    %s\n", desc);
            }
        }
    }
    
    fclose(f);
    printf("-------------------\n\n");
    return 0;
}

/* Cache downloaded packages locally */
int repo_cache_packages(void) {
    char *cache = get_cache_path();
    char src[512];
    char cmd[1024];
    
    snprintf(src, sizeof(src), "%s/repo/packages", cache);
    
    printf("Caching package data...\n");
    
    /* Copy index.yaml to cache root */
    snprintf(cmd, sizeof(cmd), "cp %s/index.yaml %s/", src, cache);
    if (system(cmd) != 0)
        printf("Warning: Could not cache index.yaml\n");
    
    printf("Package data cached at %s/\n", cache);
    return 0;
}

/* Main sync function */
int repo_sync(void) {
    printf("=== Synchronizing tinypkg repository ===\n\n");
    
    /* Step 1: Clone or pull repository */
    if (repo_clone_or_pull() != 0)
        return -1;
    
    printf("\n");
    
    /* Step 2: Parse index and list packages */
    if (repo_parse_index() != 0)
        return -1;
    
    /* Step 3: Cache package data locally */
    if (repo_cache_packages() != 0)
        return -1;
    
    printf("\n✓ Repository sync complete!\n");
    return 0;
}

/* Add a repository source (stub) */
int repo_add(const char *url) {
    if (!url)
        return -1;
    printf("repo_add: adding repository from %s\n", url);
    printf("(Not yet implemented)\n");
    return 0;
}

/* Remove a repository source (stub) */
int repo_remove(const char *name) {
    if (!name)
        return -1;
    printf("repo_remove: removing repository %s\n", name);
    printf("(Not yet implemented)\n");
    return 0;
}
