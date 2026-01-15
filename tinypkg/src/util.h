/*
 * util.h - Utilities for searching and displaying package information
 */

#ifndef UTIL_H
#define UTIL_H

/* Search and info utilities */
int util_search(const char *term);    /* Search for packages */
int util_info(const char *name);      /* Display package details */
int util_list(void);                  /* List all packages */

#endif
