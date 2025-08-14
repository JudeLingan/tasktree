#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>
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
	bool completed;
	char *name;
	char *details;
	tasklist tl;
	tasklist *parent;
};

//char *malloc_sprintf(const char* format, ...);
//int append_string(char *s, char c);
task new_task(const char *name, const char *data, int64_t id);
void task_free_elements(task tsk);
int task_free(task *tsk);
int task_add_task(task* branch, task tsk);
void task_toggle_complete(task *tsk);
int print_task(task tsk);
char *get_input();
stringlist stringlist_input();
task *tasktree_get_task_by_id(int64_t id);
void tasktree_load(const char *path);
void tasktree_unload();
void tasktree_print();
void tasktree_add_task(task *tsk, const int64_t *path);
task *tasktree_get_task(const int64_t *path);
void tasktree_remove_task_by_path(const int64_t *path);

#endif
