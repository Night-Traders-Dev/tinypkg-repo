#include "tinypkg.h"
#include "config.h"
#include "util.h"
#include <yaml.h>
#include <stdio.h>
#include <string.h>

int load_manifest(const char *pkg, Package *out) {
    char path[4096];
    FILE *f;
    yaml_parser_t parser;
    yaml_event_t event;

    memset(out, 0, sizeof(Package));

    if (path_join4(path, sizeof(path), CACHE_DIR, "repo", "packages", pkg)) return 1;
    if (path_join(path, sizeof(path), path, "manifest.yaml")) return 1;

    f = fopen(path, "r");
    if (!f) return 1;

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, f);

    char key[128] = "";

    while (1) {
        yaml_parser_parse(&parser, &event);
        if (event.type == YAML_STREAM_END_EVENT) break;

        if (event.type == YAML_SCALAR_EVENT) {
            const char *val = (const char *)event.data.scalar.value;

            if (!*key) {
                strncpy(key, val, sizeof(key)-1);
            } else {
                if (!strcmp(key, "name")) strncpy(out->name, val, sizeof(out->name)-1);
                else if (!strcmp(key, "version")) strncpy(out->version, val, sizeof(out->version)-1);
                else if (!strcmp(key, "description")) strncpy(out->description, val, sizeof(out->description)-1);
                else if (!strcmp(key, "source")) strncpy(out->source, val, sizeof(out->source)-1);
                else if (!strcmp(key, "build")) strncpy(out->build, val, sizeof(out->build)-1);
                else if (!strcmp(key, "install")) strncpy(out->install, val, sizeof(out->install)-1);
                else if (!strcmp(key, "checksum")) strncpy(out->checksum, val, sizeof(out->checksum)-1);
                key[0] = 0;
            }
        }

        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);
    fclose(f);
    return 0;
}
