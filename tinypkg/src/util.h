/*
 * util.h - Package utilities (search, display, info)
 */

#ifndef UTIL_H
#define UTIL_H

/* Utility functions */
int util_search(const char *term);    /* Search for packages */
int util_info(const char *name);      /* Display package info */
int util_list(void);                  /* List all packages */

#endif
