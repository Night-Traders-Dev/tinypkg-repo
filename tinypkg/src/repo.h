/*
 * repo.h - Repository management (downloading and storing)
 */

#ifndef REPO_H
#define REPO_H

/* Repository operations */
int repo_sync(void);              /* Download and sync repository */
int repo_add(const char *url);    /* Add repository source */
int repo_remove(const char *name);/* Remove repository source */

#endif
