#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

int append_string(char *s, char c) {
	int len = strlen(s);
	s[len] = c;
	s[len + 1] = '\0';
	return 0;
}

stringlist split_by_char(char *str, char ch) {
	int buffer_length = strlen(str) + 1;
	char buffer[buffer_length];
	strncpy(buffer, "", buffer_length);

	stringlist result;
	result.items = (char**)malloc(sizeof(char*));
	result.length = 0;

	if (str == NULL || buffer_length == 0) {
		printf("ERROR: null string\n");
		result.items = NULL;
		result.length = 0;
		return result;
	}

	for (long unsigned int i = 0; i <= strlen(str); ++i) {
		if (str[i] == ch || str[i] == '\0') {

			char **newitems = (char**)realloc(
				result.items,
				sizeof(char*)*(result.length + 1)
			);
			
			if (newitems == NULL) {
				printf("ERROR: realloc failed: null pointer exception\n");
				result.items = NULL;
				result.length = 0;
				return result;
			}

			result.items = newitems;
			result.items[result.length] = strdup(buffer);
			++result.length;

			strncpy(buffer, "", buffer_length);
		} 
		else if (str[i] == '\\') {
			++i;
			append_string(buffer, str[i]);
		}
		else {
			append_string(buffer, str[i]);
		}
	}

	return result;
}

void stringlist_free_elements(stringlist sl) {
	for (int i = 0; i < sl.length; ++i) {
		free(sl.items[i]);
	}
	free(sl.items);
}

void stringlist_free(stringlist *sl) {
	stringlist_free_elements(*sl);
	free(sl);
}

tasklist new_tasklist() {
	tasklist tl = {
		.tasks = NULL,
		.ntasks = 0
	};
	return tl;
}

int tasklist_increment_tasks(tasklist* tl) {
	task *new_tasks;
	if (tl->tasks != NULL) {
		printf("%p\n", tl->tasks);
		new_tasks = (task*)realloc(tl->tasks, (tl->ntasks + 1)*sizeof(task));
	}
	else {
		new_tasks = (task*)malloc((tl->ntasks + 1)*sizeof(task));
	}

	if (new_tasks == NULL) {
		printf("ERROR: task reallocation failed\n");
		return 1;
	}

	tl->tasks = new_tasks;

	return 0;
}

/*
	* Get first task with name in tasklist
*/
task *tasklist_get_task_by_name(tasklist tl, char *name) {
	for (int i = 0; i < tl.ntasks; ++i) {
	  if (!strcmp(tl.tasks[i].name, name)) {
			return &tl.tasks[i];
	  }
	}

	return NULL;
}

task *tasklist_get_task_by_path(tasklist tl, char *str) {
	stringlist sl = split_by_char(str, '/');

	//remove last element of sl if it is empty, allowing for trailing slash
	if (!strcmp(sl.items[sl.length - 1], "")) {
		free(sl.items);
		--sl.length;
	}

	task *tsk = NULL;

	for (int i = 0; i < sl.length; ++i) {
		tsk = tasklist_get_task_by_name(tl, sl.items[i]);

		if (tsk == NULL) {
			stringlist_free_elements(sl);
			return NULL;
		}
		else {
			tl = tsk->tl;
		}
	}

	stringlist_free_elements(sl);
	
	return tsk;
}

int tasklist_add_task(tasklist *tl, task t) {
	if (tasklist_increment_tasks(tl)) {
		return 1;
	}

	tl->tasks[tl->ntasks] = t;
	printf("task %d is %s\n", tl->ntasks, tl->tasks[tl->ntasks].name);
	tl->ntasks += 1;
	printf("new ntasks is %d\n", tl->ntasks);

	return 0;
}

void tasklist_free_elements(tasklist *tl) {
	for (int i = 0; i < tl->ntasks; ++i) {
		task_free_elements(tl->tasks[i]);
	}

	if (tl->tasks != NULL)
		free(tl->tasks);

	tl->ntasks = 0;
	tl->tasks = NULL;
}

task new_task(char *name, char *details) {
	task tsk;

	tsk.name = name;
	tsk.details = details;

	tsk.tl.tasks = NULL;
	tsk.tl.ntasks = 0;

	return tsk;
}

void task_free_elements(task tsk) {
	//NOTE: this does make a recursive function as tasklist_free_elements calls task_free_elements
	tasklist_free_elements(&tsk.tl);

	free(tsk.name);
	free(tsk.details);
	free(tsk.tl.tasks);
}

int task_free(task *tsk) {
	task_free_elements(*tsk);
	free(tsk);

	return 0;
}

int task_add_task(task *root, task branch) {
	int addfailure = tasklist_add_task(&root->tl, branch);
	if (addfailure == 1) {
		return 1;
	}
	
	return 0;
}

int print_task(task tsk) {
	for (int i = 0; i < tsk.tl.ntasks; ++i) {
		print_task(tsk.tl.tasks[i]);
	}

	printf("task: %s\n\tdetails: %s\n", tsk.name, tsk.details);

	return 0;
}

void tasktree_load(tasktree *tree, char *path) {
	sqlite3 *db = NULL;
	tasklist tl = new_tasklist();
	
	if (path != NULL) {
		sqlite3_open(path, &db);
	}

	tree->database = db;
	tree->tl = tl;
}

void tasktree_unload(tasktree *tree) {
	sqlite3_close(tree->database);
	tasklist_free_elements(&tree->tl);
}

task *tasktree_get_task(tasktree tree, char *path) {
	return tasklist_get_task_by_path(tree.tl, path);
}

void tasktree_add_task(tasktree *tree, task tsk, char *path) {
	if (path == NULL) {
		tasklist_add_task(&tree->tl, tsk);
	}
	else {
		//create new task in memory
		task *parent = tasklist_get_task_by_path(tree->tl, path);

		if (parent == NULL) {
			printf("task not found\n");
		}
		else {
			task_add_task(parent, tsk);
		}
	}

	if (tree->database == NULL) return;

	//enter new task into database
	char sql_format[] = "INSERT INTO tasks (name, deatails) VALUES (%s, %s)";
	int sql_length = strlen(sql_format) + strlen(tsk.name) + strlen(tsk.details) + 1;
	char sql_code[sql_length];

	snprintf(
		sql_code,
		sql_length,
		sql_format,
		tsk.name,
		tsk.details
	);

	char* sql_error = NULL;

	sqlite3_exec(
		tree->database,
		sql_code,
		NULL,
		NULL,
		&sql_error
	);
}

char *get_input() {
	size_t buffer_length = 0;
	char *buffer = NULL;
	getline(&buffer, &buffer_length, stdin);
	buffer[strcspn(buffer, "\n")] = '\0';
	
	char *str = (char*)malloc(strlen(buffer) + 1);
	strncpy(str, buffer, strlen(buffer) + 1);

	free(buffer);

	return str;
}

stringlist stringlist_input() {
	char *raw = get_input();

	stringlist output = split_by_char(raw, ' ');
	free(raw);
	return output;
}
