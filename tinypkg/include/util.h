/*
 * util.h - Utilities for searching and displaying package information
 */

#ifndef UTIL_H
#define UTIL_H

/* Search and info utilities */
int util_search(const char *term);
int util_info(const char *name);
int util_list(void);

#endif
