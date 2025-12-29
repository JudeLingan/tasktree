#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
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
	struct time *deadline;
};

enum Column {
	ID,
	PARENT,
	NAME,
	DETAILS,
	DEADLINE,
	COMPLETED
};


task new_task(const char *name, const char *data, int64_t id);
void task_free_elements(task tsk);
int task_free(task *tsk);
int task_add_task(task* branch, task tsk);
void task_toggle_complete(task *tsk);
int task_set_column(task *tsk, enum Column column, char *value);
int print_task(task tsk);
stringlist stringlist_input();
task *tasktree_get_task_by_id(int64_t id);
void tasktree_load(const char *path);
void tasktree_unload();
void tasktree_print();
void tasktree_add_task(task tsk, task *parent);
void tasktree_remove_task(task *tsk);

#endif
