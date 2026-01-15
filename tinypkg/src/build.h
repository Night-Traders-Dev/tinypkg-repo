/*
 * build.h - Building and installing packages
 */

#ifndef BUILD_H
#define BUILD_H

/* Build operations */
int build_package(const char *name);   /* Build a package */
int install_package(const char *name); /* Install a package */
int remove_package(const char *name);  /* Remove a package */

#endif
