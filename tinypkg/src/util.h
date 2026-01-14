#pragma once
#include <stddef.h>

int path_join(char *out, size_t max, const char *a, const char *b);
int path_join3(char *out, size_t max, const char *a, const char *b, const char *c);
int path_join4(char *out, size_t max, const char *a, const char *b, const char *c, const char *d);

int ensure_dir(const char *path);
int run_cmd(const char *fmt, ...);
void shell_escape(const char *in, char *out, size_t max);
int find_first_subdir(const char *dir, char *out, size_t max);
