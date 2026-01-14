#include "tinypkg.h"
#include "config.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Simple manifest parser used for the package manifest format in the repo.
 * This avoids relying on a streaming YAML parser and supports the keys
 * we need: name, version, description, source, build, install, checksum.
 */
int load_manifest(const char *pkg, Package *out) {
    char path[PATH_MAX];
    FILE *f;
    char line[4096];

    memset(out, 0, sizeof(Package));

    if (path_join4(path, sizeof(path), CACHE_DIR, "repo", "packages", pkg)) return 1;
    if (path_join(path, sizeof(path), path, "manifest.yaml")) return 1;

    f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "could not open manifest path: %s\n", path);
        return 1;
    }

    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0' || *p == '#') continue;

        /* Look for top-level key: value */
        char *colon = strchr(p, ':');
        if (!colon) continue;
        /* Extract key */
        size_t keylen = colon - p;
        char key[128] = {0};
        if (keylen >= sizeof(key)) keylen = sizeof(key)-1;
        strncpy(key, p, keylen);
        key[keylen] = '\0';
        /* Move past colon and optional space */
        char *val = colon + 1;
        while (isspace((unsigned char)*val)) val++;

        if (strcmp(key, "build") == 0 && *val == '|') {
            /* Read following indented lines until next top-level key */
            char accum[2048] = {0};
            while (fgets(line, sizeof(line), f)) {
                /* stop when a non-indented line or EOF */
                if (line[0] != ' ' && line[0] != '\t' && !(line[0] == '\n')) {
                    /* push file pointer back so outer loop will handle this line */
                    fseek(f, -((long)strlen(line)), SEEK_CUR);
                    break;
                }
                /* strip leading indent */
                char *q = line;
                while (*q == ' ' || *q == '\t') q++;
                strncat(accum, q, sizeof(accum)-strlen(accum)-1);
            }
            strncpy(out->build, accum, sizeof(out->build)-1);
        } else if (strcmp(key, "install") == 0 && *val == '|') {
            char accum[2048] = {0};
            while (fgets(line, sizeof(line), f)) {
                if (line[0] != ' ' && line[0] != '\t' && !(line[0] == '\n')) {
                    fseek(f, -((long)strlen(line)), SEEK_CUR);
                    break;
                }
                char *q = line;
                while (*q == ' ' || *q == '\t') q++;
                strncat(accum, q, sizeof(accum)-strlen(accum)-1);
            }
            strncpy(out->install, accum, sizeof(out->install)-1);
        } else {
            /* single-line value */
            /* trim trailing newline */
            char *nl = strchr(val, '\n');
            if (nl) *nl = '\0';
            if (strcmp(key, "name") == 0) strncpy(out->name, val, sizeof(out->name)-1);
            else if (strcmp(key, "version") == 0) strncpy(out->version, val, sizeof(out->version)-1);
            else if (strcmp(key, "description") == 0) strncpy(out->description, val, sizeof(out->description)-1);
            else if (strcmp(key, "source") == 0) strncpy(out->source, val, sizeof(out->source)-1);
            else if (strcmp(key, "checksum") == 0) strncpy(out->checksum, val, sizeof(out->checksum)-1);
        }
    }

    fclose(f);
    return 0;
}
