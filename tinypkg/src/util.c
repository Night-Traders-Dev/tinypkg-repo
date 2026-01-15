/*
 * util.c - Utilities for searching and displaying package information
 * 
 * Reads from cached repository data (~/.cache/tinypkg/)
 * Performs searches and displays package metadata
 * 
 * IMPROVEMENTS:
 * - Centralized path management via common.c
 * - Input validation
 * - Better error messages
 */

#include "common.h"
#include "util.h"

/* Convert string to lowercase for case-insensitive search */
static char* strlower(char *dest, size_t dest_size, const char *str) {
    if (!dest || !str || dest_size == 0) return NULL;
    
    int i = 0;
    while (str[i] && i < (int)dest_size - 1) {
        dest[i] = tolower((unsigned char)str[i]);
        i++;
    }
    dest[i] = '\0';
    return dest;
}

/* Search for packages matching term (case-insensitive) */
int util_search(const char *term) {
    char *cache = get_cache_path();
    char index_path[PATH_MAX_LEN];
    FILE *f;
    char line[LINE_MAX_LEN];
    char term_lower[128];
    int found = 0;
    struct stat st;
    
    if (!term || !term[0]) {
        log_error("util_search", "Search term required");
        return TINYPKG_ERR;
    }
    
    if (!cache) {
        log_error("util_search", "Failed to get cache path");
        return TINYPKG_ERR;
    }
    
    snprintf(index_path, PATH_MAX_LEN, "%s/index.yaml", cache);
    
    /* Check if cache exists */
    if (stat(index_path, &st) != 0) {
        log_error("util_search", "Repository not synced - run 'tinypkg repo sync' first");
        return TINYPKG_ERR;
    }
    
    f = fopen(index_path, "r");
    if (!f) {
        log_error("util_search", "Could not open index");
        return TINYPKG_ERR;
    }
    
    if (!strlower(term_lower, sizeof(term_lower), term)) {
        log_error("util_search", "Search term too long");
        fclose(f);
        return TINYPKG_ERR;
    }
    
    printf("Searching for '%s'...\n\n", term);
    
    int in_packages = 0;
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        
        /* Skip leading whitespace */
        while (*p && (*p == ' ' || *p == '\t')) p++;
        
        if (!in_packages && strncmp(p, "packages:", 9) == 0) {
            in_packages = 1;
            continue;
        }
        
        if (!in_packages) continue;
        
        /* Check for package name (2-space indent, followed by colon) */
        if (line[0] == ' ' && line[1] == ' ' && line[2] != ' ' && line[2] != '\0') {
            char name[128] = {0};
            char version[64] = {0};
            char desc[256] = {0};
            char name_lower[128];
            
            if (sscanf(line + 2, "%127[^:]:", name) == 1) {
                if (!strlower(name_lower, sizeof(name_lower), name)) {
                    continue;
                }
                
                /* Read following lines for version and description */
                char next_line[LINE_MAX_LEN];
                while (fgets(next_line, sizeof(next_line), f)) {
                    if (next_line[0] != ' ' || next_line[1] != ' ' || next_line[2] == ' ') {
                        break;
                    }
                    
                    if (strstr(next_line, "version:")) {
                        sscanf(next_line, "%*[^:]:%*[ ]%63s", version);
                    }
                    
                    if (strstr(next_line, "description:")) {
                        sscanf(next_line, "%*[^:]:%*[ ]%255[^\n]", desc);
                    }
                }
                
                /* Check if name or description matches search term */
                if (strstr(name_lower, term_lower) || strstr(desc, term)) {
                    printf(" %s (%s)\n", name, version[0] ? version : "unknown");
                    if (desc[0]) {
                        printf(" %s\n", desc);
                    }
                    printf("\n");
                    found++;
                }
            }
        }
    }
    
    fclose(f);
    
    if (found == 0) {
        printf("No packages found matching '%s'\n", term);
        return TINYPKG_ERR;
    } else {
        printf("Found %d package(s)\n", found);
        return TINYPKG_OK;
    }
}

/* Display detailed information about a package */
int util_info(const char *name) {
    char *cache = get_cache_path();
    char manifest_path[PATH_MAX_LEN];
    FILE *f;
    char line[LINE_MAX_LEN];
    struct stat st;
    
    if (!name || !name[0]) {
        log_error("util_info", "Package name required");
        return TINYPKG_ERR;
    }
    
    if (!is_valid_package_name(name)) {
        log_error("util_info", "Invalid package name");
        return TINYPKG_ERR;
    }
    
    if (!cache) {
        log_error("util_info", "Failed to get cache path");
        return TINYPKG_ERR;
    }
    
    snprintf(manifest_path, PATH_MAX_LEN, "%s/repo/packages/%s/manifest.yaml", 
             cache, name);
    
    /* Check if file exists first to avoid race condition */
    if (stat(manifest_path, &st) != 0) {
        log_error("util_info", "Package not found");
        return TINYPKG_NOT_FOUND;
    }
    
    f = fopen(manifest_path, "r");
    if (!f) {
        log_error("util_info", "Could not open manifest");
        return TINYPKG_ERR;
    }
    
    printf("\n=== Package Information: %s ===\n\n", name);
    
    while (fgets(line, sizeof(line), f)) {
        /* Remove trailing newline */
        line[strcspn(line, "\n")] = 0;
        
        /* Skip empty lines and comment lines */
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }
        
        /* Format output */
        if (strstr(line, ":")) {
            printf("%s\n", line);
        } else if (line[0] == ' ') {
            printf("%s\n", line);
        }
    }
    
    fclose(f);
    printf("\n");
    
    return TINYPKG_OK;
}

/* List all available packages */
int util_list(void) {
    char *cache = get_cache_path();
    char index_path[PATH_MAX_LEN];
    FILE *f;
    char line[LINE_MAX_LEN];
    struct stat st;
    
    if (!cache) {
        log_error("util_list", "Failed to get cache path");
        return TINYPKG_ERR;
    }
    
    snprintf(index_path, PATH_MAX_LEN, "%s/index.yaml", cache);
    
    /* Check if cache exists */
    if (stat(index_path, &st) != 0) {
        log_error("util_list", "Repository not synced - run 'tinypkg repo sync' first");
        return TINYPKG_ERR;
    }
    
    f = fopen(index_path, "r");
    if (!f) {
        log_error("util_list", "Could not open index");
        return TINYPKG_ERR;
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
        
        if (!in_packages) continue;
        
        /* Package entries: 2-space indent, name followed by colon */
        if (line[0] == ' ' && line[1] == ' ' && line[2] != ' ' && line[2] != '\0') {
            char name[128] = {0};
            char version[64] = {0};
            
            if (sscanf(line + 2, "%127[^:]:", name) == 1) {
                /* Try to get version from next lines */
                char next_line[LINE_MAX_LEN];
                
                while (fgets(next_line, sizeof(next_line), f)) {
                    if (next_line[0] != ' ' || next_line[1] != ' ' || next_line[2] == ' ') {
                        break;
                    }
                    
                    if (strstr(next_line, "version:")) {
                        sscanf(next_line, "%*[^:]:%*[ ]%63s", version);
                        break;
                    }
                }
                
                printf(" %-20s %s\n", name, version[0] ? version : "unknown");
                count++;
            }
        }
    }
    
    fclose(f);
    
    printf("\n===================\n");
    printf("Total: %d packages\n\n", count);
    
    return TINYPKG_OK;
}
