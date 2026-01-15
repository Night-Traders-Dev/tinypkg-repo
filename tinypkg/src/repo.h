/*
 * repo.h - Repository management (downloading and storing)
 */

#ifndef REPO_H
#define REPO_H

/* Repository operations */
int repo_sync(void);              /* Download and sync repository */
int repo_add(const char *url);    /* Add repository source */
int repo_remove(const char *name);/* Remove repository source */

/* Internal helper functions */
int repo_clone_or_pull(void);     /* Clone or update git repo */
int repo_parse_index(void);       /* Parse packages/index.yaml */
int repo_cache_packages(void);    /* Cache package data locally */

#endif
