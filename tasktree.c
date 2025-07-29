#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sqlite3.h>
#include "tasktree.h"
#include "util.h"

char *get_input() {
	size_t buffer_length = 0;
	char *buffer = NULL;

	//read input until newline
	char c;
	int i = 0;
	c = fgetc(stdin);
	while (c != EOF && c != '\n') {
		buffer = realloc(buffer, sizeof(char)*(i + 2));
		buffer[i] = c;
		buffer[i + 1] = '\0';
		++i;
		c = fgetc(stdin);
	}
	
	char *str = (char*)malloc(strlen(buffer) + 1);
	strncpy(str, buffer, strlen(buffer) + 1);

	free(buffer);

	return str;
}

//gets a line of input and returns it as a stringlist
stringlist stringlist_input() {
	char *raw = get_input();
	stringlist output = split_by_char(raw, ' ');
	free(raw);
	return output;
}

tasklist new_tasklist();

tasklist tl = {.tasks = NULL, .ntasks = 0};
sqlite3 *db = NULL;

/*TASKLIST FUNCTIONS*/

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
		new_tasks = (task*)realloc(tl->tasks, (tl->ntasks + 1)*sizeof(task));
	}
	else {
		new_tasks = (task*)malloc((tl->ntasks + 1)*sizeof(task));
	}

	if (new_tasks == NULL) {
		handle_error("ERROR: task reallocation failed\n");
		return 1;
	}

	tl->tasks = new_tasks;
	++tl->ntasks;

	return 0;
}

/*
 * Get first task with name in tasklist.
 * breaks with multiple tasks that have the same name.
 * will most likely be replaced with ids once a GUI is made.
*/
task *tasklist_get_task_by_name(tasklist tl, const char *name) {
	//prevents NULL errors
	if (name == NULL || tl.tasks == NULL || tl.ntasks <= 0) {
		return NULL;
	}

	//get task
	for (int i = 0; i < tl.ntasks; ++i) {
	  if (!strcmp(tl.tasks[i].name, name)) {
			return &tl.tasks[i];
	  }
	}

	//return NULL if no task is found with given name
	return NULL;
}

task *tasklist_get_task_by_path(tasklist tl, const char *path) {
	//prevent errors from null
	if (path == NULL) {
		return NULL;
	}

	stringlist pathlist = split_by_char(path, '/');

	//remove last element of sl if it is empty, allowing for trailing slash
	if (!strcmp(pathlist.items[pathlist.length - 1], "")) {
		free(pathlist.items[pathlist.length - 1]);
		pathlist.items[pathlist.length - 1] = NULL;
		--pathlist.length;
	}

	task *tsk = NULL;

	for (int i = 0; i < pathlist.length; ++i) {
		tsk = tasklist_get_task_by_name(tl, pathlist.items[i]);

		if (tsk == NULL) {
			stringlist_free_elements(pathlist);
			return NULL;
		}
		else {
			tl = tsk->tl;
		}
	}

	stringlist_free_elements(pathlist);
	return tsk;
}

int tasklist_add_task(tasklist *tl, task t) {
	if (tasklist_increment_tasks(tl)) {
		handle_error("tasklist_increment_tasks failed\n");
		return 1;
	}

	t.parent = tl;
	tl->tasks[tl->ntasks - 1] = t;
	return 0;
}

void tasklist_remove_task(tasklist *tl, long long id) {
	//prevent crashes
	if (tl == NULL) {
		handle_error("TASK HAS NULL PARENT");
		return;
	}

	//set index to index of task with said id in array
	tasklist new_tl = new_tasklist();
	for (int i = 0; i < tl->ntasks; ++i) {
		if (tl->tasks[i].id == id) {
			task_free_elements(tl->tasks[i]);
		}
		else {
			tasklist_add_task(&new_tl, tl->tasks[i]);
			new_tl.tasks[new_tl.ntasks - 1].parent = tl;
		}
	}

	//replace tasklist
	free(tl->tasks);
	*tl = new_tl;
}

void tasklist_free_elements(tasklist *tl) {
	for (int i = 0; i < tl->ntasks; ++i) {
		task_free_elements(tl->tasks[i]);
	}

	free(tl->tasks);

	tl->ntasks = 0;
	tl->tasks = NULL;
}

/*TASK FUNCTIONS*/

task new_task(char *name, char *details, long long id) {
	task tsk = {
		.id = id,
		.name = strdup(name),
		.details = strdup(details),
		.completed = false,
		.tl = new_tasklist()
	};

	return tsk;
}

void task_free_elements(task tsk) {
	//NOTE: this does make a recursive function as tasklist_free_elements calls task_free_elements
	tasklist_free_elements(&tsk.tl);

	free(tsk.name);
	free(tsk.details);
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
	static int indent = -1;
	
	++indent;

	for (int i = 0; i < indent; ++i) printf("\t");

	printf(tsk.completed ? GREEN("%s\n") : RED("%s\n"), tsk.name);

	if (!string_is_empty(tsk.details)) {
		for (int i = 0; i < indent; ++i) printf("\t");
		printf("%s\n", tsk.details);
	}

	for (int i = 0; i < tsk.tl.ntasks; ++i) {
		print_task(tsk.tl.tasks[i]);
	}

	--indent;

	return 0;
}

void task_complete(task *tsk) {
	if (tsk == NULL) {
		handle_error("CANNOT COMPLETE NULL TASK\n");
		return;
	}

	tsk->completed = true;

	//update database
	if (db != NULL) {
		sqlite3_exec_by_format(
			db,
			NULL,
			NULL,
			"UPDATE tasks SET completed = 1 WHERE id = %lld;",
			tsk->id
		);
	}
	else {
		handle_error("CANNOT COMPLETE TASK, NO DATABASE FOUND\n");
	}
}

/*TASKTREE FUNCTIONS*/
static int callback_load_child_tasks_from_db(void *in, int ncolumns, char **values, char **columns) {
	char *name = NULL;
	char *details = NULL;
	long long id = -1;
	bool completed = false;

	for (int i = 0; i < ncolumns; ++i) {
		char *val = values[i];
		char *column = columns[i];

		if (val == NULL) {
			continue;
		}
		else if (!strcmp(column, "name")) {
			name = strdup(val);
		}
		else if (!strcmp(column, "details")) {
			details = val == NULL ? strdup("") : strdup(val);
		}
		else if (!strcmp(column, "id")) {
			id = atoi(val);
		}
		else if (!strcmp(column, "completed")) {
			completed = atoi(val);
		}
	}

	tasklist *parentlist = (tasklist*)in;
	task tsk = new_task(name, details, id);
	tsk.completed = completed;

	tasklist_add_task(parentlist, tsk);

	free(name);
	free(details);
	id = -1;
	return 0;
}

void load_child_tasks_from_db(task *parent) {
	int parentid = 0;
	if (parent != NULL) {
		parentid = parent->id;
	}

	tasklist *parenttl = parent == NULL ? &tl : &parent->tl;

	sqlite3_exec_by_format(
		db,
		callback_load_child_tasks_from_db,
		parenttl,
		"SELECT * FROM tasks WHERE parent = %d",
		parentid
	);

	for(int i = 0; i < parenttl->ntasks; ++i) {
		load_child_tasks_from_db(&parenttl->tasks[i]);
	}
}

void load_tasks_from_db() {
	printf("loading tasks part 1\n");
	load_child_tasks_from_db(NULL);
}

void tasktree_load(const char *path) {
	int rc = 1;

	if (path != NULL) {
		printf("OPENING DATABASE\n");
		rc = sqlite3_open(path, &db);
	}

	if (rc) {
		handle_error("SQL CONNECTION FAILED\n");
	}
	
	if (!sqlite3_has_table(db, "tasks")) {
		handle_error("table tasks not found\n");
		sqlite3_exec(
			db,
			"CREATE TABLE tasks (id INTEGER PRIMARY KEY, parent INTEGER, name TEXT, details TEXT, completed INTEGER);",
			NULL,
			NULL,
			NULL
		);
	}

	load_tasks_from_db();
}

void tasktree_unload() {
	sqlite3_close(db);
	tasklist_free_elements(&tl);
}

void tasktree_print() {
	for(int i = 0; i < tl.ntasks; ++i) {
		print_task(tl.tasks[i]); 
	}
}

task *tasktree_get_task(const char *path) {
	return tasklist_get_task_by_path(tl, path);
}

//TODO: make this neater
static void tasktree_remove_task_from_db(long long id);

static int callback_remove_task_from_db(void *val, int ncolumns, char **values, char **columns) {
	UNUSED(val);

	long long id = 0;

	for (int i = 0; i < ncolumns; ++i) {
		if (!strcmp(columns[i], "id")) {
			id = atoi(values[i]);
			break;
		}
	}

	tasktree_remove_task_from_db(id);
	return 0;
}

static void tasktree_remove_task_from_db(long long id) {
	sqlite3_exec_by_format(
		db,
		callback_remove_task_from_db,
		NULL,
		"DELETE FROM tasks WHERE id=%lld;",
		id
	);
}

void tasktree_remove_task(task *tsk) {
	//prevents crashes
	if (tsk == NULL) {
		handle_error("CANNOT REMOVE NULL TASK\n");
		return;
	}

	//delete task from database
	tasktree_remove_task_from_db(tsk->id);

	//remove task from memory
	tasklist_remove_task(tsk->parent, tsk->id);
}

void tasktree_remove_task_by_path(const char *path) {
	tasktree_remove_task(tasklist_get_task_by_path(tl, path));
}

/*
 * adds to a number after the name until the name is unique.
 * this prevents duplicate names.
 * returns a character pointer with a unique name.
 */
char *tasklist_get_next_available_name(tasklist tl, char* name) {
	//prevents NULL errors
	if (name == NULL) {
		return NULL;
	}

	char *numberless = NULL;
	char *strnum = &name[0];
	int num = 0;

	//sets i to the first character that only has numbers after it
	int i = 0;
	for (i = 0; is_pure_num(strnum) == false && strnum[0] != '\0'; ++i) {
		strnum = &strnum[1];
	}

	//sets numberless to everything before the number at the end
	numberless = (char*)malloc(sizeof(char*)*(i+1));
	strncpy(numberless, name, i);
	numberless[i] = '\0';

	//increases number until name is available, then assigns it to "result"
	num = atoi(strnum);
	char *result = NULL;

	do {
		free(result);
		if (num == 0) {
			result = malloc_sprintf("%s", numberless);
		}
		else {
			result = malloc_sprintf("%s%d", numberless, num);
		}
		num += 1;
	} while (tasklist_get_task_by_name(tl, result) != NULL);

	free(numberless);

	return result;
}

void tasktree_add_task(task *tsk, const char *path) {
	printf("adding task\n");

	//variable declarations
	task *parent = tasklist_get_task_by_path(tl, path);                             //parent task
	tasklist *parentlist = path == NULL ? &tl : &parent->tl;                        //parent tasklist
	char *taskname = tasklist_get_next_available_name(*parentlist, tsk->name);      //name of task

	//create new task in memory
	tasklist_add_task(parentlist, new_task(taskname, tsk->details, tsk->id));

	//bypass database entry if database not found or task is already inserted
	if (db == NULL || tsk->id > 0) {
		printf("task already in db\n");
		return;
	}

	//enter new task into database
	long long parentid = 0;

	if (parent != NULL) {
		parentid = parent->id;
	}
	//execute sqlite code
	if (tsk->details != NULL) {
		char sql_format[] = "INSERT INTO tasks (parent, name, details) VALUES (%lld, '%s', '%s');";
		sqlite3_exec_by_format(db, NULL, NULL, sql_format, parentid, taskname, tsk->details);
	}
	else {
		char sql_format[] = "INSERT INTO tasks (parent, name) VALUES ('%s', %lld);";
		sqlite3_exec_by_format(db, NULL, NULL, sql_format, parentid, taskname);
	}

	//free local variables
	free(taskname);

	//set id of task to the one given by the database
	parentlist->tasks[parentlist->ntasks - 1].id = sqlite3_last_insert_rowid(db);
}
