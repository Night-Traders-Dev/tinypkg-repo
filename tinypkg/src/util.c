/*
 * util.c - Utilities for searching and displaying package information
 * 
 * Reads from cached repository data (~/.cache/tinypkg/)
 * Performs searches and displays package metadata
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include "util.h"

#define CACHE_DIR ".cache/tinypkg"

/* Get cache path */
static char* get_cache_path(void) {
    static char path[512];
    const char *home = getenv("HOME");
    if (!home)
        home = ".";
    snprintf(path, sizeof(path), "%s/%s", home, CACHE_DIR);
    return path;
}

/* Convert string to lowercase for case-insensitive search */
static char* strlower(const char *str) {
    static char buf[256];
    int i = 0;
    while (str[i] && i < 255) {
        buf[i] = tolower(str[i]);
        i++;
    }
    buf[i] = '\0';
    return buf;
}

/* Search for packages matching term (case-insensitive) */
int util_search(const char *term) {
    char *cache = get_cache_path();
    char index_path[512];
    FILE *f;
    char line[256];
    char term_lower[128];
    int found = 0;
    
    if (!term || !term[0]) {
        printf("Error: search term required\n");
        return -1;
    }
    
    snprintf(index_path, sizeof(index_path), "%s/index.yaml", cache);
    
    /* Check if cache exists */
    struct stat st;
    if (stat(index_path, &st) != 0) {
        printf("Error: Repository not synced. Run 'tinypkg repo sync' first\n");
        return -1;
    }
    
    f = fopen(index_path, "r");
    if (!f) {
        printf("Error: Could not open index at %s\n", index_path);
        return -1;
    }
    
    strncpy(term_lower, strlower(term), sizeof(term_lower) - 1);
    
    printf("Searching for '%s'...\n\n", term);
    
    int in_packages = 0;
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        
        /* Skip leading whitespace */
        while (*p && (*p == ' ' || *p == '\t'))
            p++;
        
        if (!in_packages && strncmp(p, "packages:", 9) == 0) {
            in_packages = 1;
            continue;
        }
        
        if (!in_packages)
            continue;
        
        /* Check for package name (2-space indent, followed by colon) */
        if (line[0] == ' ' && line[1] == ' ' && line[2] != ' ') {
            char name[128] = {0};
            char version[64] = {0};
            char desc[256] = {0};
            
            if (sscanf(line + 2, "%127[^:]:", name) == 1) {
                char name_lower[128];
                strncpy(name_lower, strlower(name), sizeof(name_lower) - 1);
                
                /* Read following lines for version and description */
                char next_line[256];
                while (fgets(next_line, sizeof(next_line), f)) {
                    if (next_line[0] != ' ' || next_line[1] != ' ' || next_line[2] == ' ')
                        break;
                    
                    if (strstr(next_line, "version:")) {
                        sscanf(next_line, "%*[^:]:%*[ ]%63s", version);
                    }
                    if (strstr(next_line, "description:")) {
                        sscanf(next_line, "%*[^:]:%*[ ]%255[^\n]", desc);
                    }
                }
                
                /* Check if name or description matches search term */
                if (strstr(name_lower, term_lower) || strstr(desc, term)) {
                    printf("  %s (%s)\n", name, version[0] ? version : "unknown");
                    if (desc[0])
                        printf("    %s\n", desc);
                    printf("\n");
                    found++;
                }
            }
        }
    }
    
    fclose(f);
    
    if (found == 0)
        printf("No packages found matching '%s'\n", term);
    else
        printf("Found %d package(s)\n", found);
    
    return found > 0 ? 0 : -1;
}

/* Display detailed information about a package */
int util_info(const char *name) {
    char *cache = get_cache_path();
    char manifest_path[512];
    FILE *f;
    char line[512];
    
    if (!name || !name[0]) {
        printf("Error: package name required\n");
        return -1;
    }
    
    snprintf(manifest_path, sizeof(manifest_path), "%s/repo/packages/%s/manifest.yaml", cache, name);
    
    f = fopen(manifest_path, "r");
    if (!f) {
        printf("Error: Package '%s' not found\n", name);
        printf("(Looked in %s)\n", manifest_path);
        return -1;
    }
    
    printf("\n=== Package Information: %s ===\n\n", name);
    
    while (fgets(line, sizeof(line), f)) {
        /* Remove trailing newline */
        line[strcspn(line, "\n")] = 0;
        
        /* Skip empty lines and comment lines */
        if (line[0] == '\0' || line[0] == '#')
            continue;
        
        /* Format output */
        if (strstr(line, ":")) {
            printf("%s\n", line);
        } else if (line[0] == ' ') {
            printf("%s\n", line);
        }
    }
    
    fclose(f);
    printf("\n");
    
    return 0;
}

/* List all available packages */
int util_list(void) {
    char *cache = get_cache_path();
    char index_path[512];
    FILE *f;
    char line[256];
    
    snprintf(index_path, sizeof(index_path), "%s/index.yaml", cache);
    
    /* Check if cache exists */
    struct stat st;
    if (stat(index_path, &st) != 0) {
        printf("Error: Repository not synced. Run 'tinypkg repo sync' first\n");
        return -1;
    }
    
    f = fopen(index_path, "r");
    if (!f) {
        printf("Error: Could not open index\n");
        return -1;
    }
    
    printf("\nAvailable Packages:\n");
    printf("===================\n\n");
    
    int in_packages = 0;
    int count = 0;
    
    while (fgets(line, sizeof(line), f)) {
        if (!in_packages && strstr(line, "packages:")) {
            in_packages = 1;
            continue;
        }
        
        if (!in_packages)
            continue;
        
        /* Package entries: 2-space indent, name followed by colon */
        if (line[0] == ' ' && line[1] == ' ' && line[2] != ' ') {
            char name[128] = {0};
            char version[64] = {0};
            
            if (sscanf(line + 2, "%127[^:]:", name) == 1) {
                /* Try to get version from next lines */
                char next_line[256];
                while (fgets(next_line, sizeof(next_line), f)) {
                    if (next_line[0] != ' ' || next_line[1] != ' ' || next_line[2] == ' ')
                        break;
                    if (strstr(next_line, "version:")) {
                        sscanf(next_line, "%*[^:]:%*[ ]%63s", version);
                        break;
                    }
                }
                
                printf("  %-20s %s\n", name, version[0] ? version : "unknown");
                count++;
            }
        }
    }
    
    fclose(f);
    
    printf("\n===================\n");
    printf("Total: %d packages\n\n", count);
    
    return 0;
}
