#include <sqlite3.h>
#include "util.h"

#ifndef TASKTREE_H
#define TASKTREE_H

typedef struct task task;

typedef struct tasklist {
	task* tasks;
	int ntasks;
} tasklist;

struct task {
	int id;
	char *name;
	char *details;
	tasklist tl;
	tasklist *parent;
};

/*
 * The structure the program is named after, used to make, manage, and find tasks in a tree structure.
 * Use new_tasktree() as a constructor, and free_tasktree(tasktree) as a destructor.
 * It is better to not modify any elements of the structure directly, and instead use functions.
 * In case modifying directly is necessary, these are the elements
 * 
 * sqlite3* database;
 * tasklist tl;
*/
typedef struct tasktree {
	sqlite3 *db;
	tasklist tl;
} tasktree;

//char *malloc_sprintf(const char* format, ...);
//int append_string(char *s, char c);
task new_task(tasklist *parent, char *name, char *data, long long id);
void task_free_elements(task tsk);
int task_free(task *tsk);
int task_add_task(task* branch, task tsk);
int print_task(task tsk);
char *get_input();
void tasktree_load(tasktree *tree, char *path);
void tasktree_unload(tasktree *tree);
void tasktree_add_task(tasktree *tree, task tsk, char* path);
task *tasktree_get_task(tasktree tree, char *path);
void tasktree_remove_task_by_path(tasktree *tree, char *path);

#endif
