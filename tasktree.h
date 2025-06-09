#include <sqlite3.h>
#include <stdbool.h>
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

//char *malloc_sprintf(const char* format, ...);
//int append_string(char *s, char c);
task new_task(char *name, char *data, long long id);
void task_free_elements(task tsk);
int task_free(task *tsk);
int task_add_task(task* branch, task tsk);
int print_task(task tsk);
char *get_input();
void tasktree_load(const char *path);
void tasktree_unload();
void tasktree_print();
void tasktree_add_task(const task tsk, const char* path);
task *tasktree_get_task(const char *path);
void tasktree_remove_task_by_path(const char *path);

#endif
