/*
 * repo.h - Repository management (downloading and storing)
 */

#ifndef REPO_H
#define REPO_H

/* Repository operations */
int repo_sync(void);
int repo_add(const char *url);
int repo_remove(const char *name);

/* Internal helper functions */
int repo_clone_or_pull(void);
int repo_parse_index(void);
int repo_cache_packages(void);

#endif
