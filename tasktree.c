#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sqlite3.h>
#include "tasktree.h"
#include "util.h"

// ALTER TABLE [table] ADD [column] datatype

static const char COLUMNS[5][2][64] = {
	{"id",         "INTEGER PRIMARY KEY"},
	{"parent",     "INTEGER NOT NULL DEFAULT 0"},
	{"name",       "TEXT"},
	{"details",    "TEXT"},
	{"completed",  "INTEGER NOT NULL DEFAULT 0"},
};

char *get_input() {
	size_t buffer_length = 0;
	char *buffer = (char*)malloc(sizeof(char*));
	*buffer = '\0';

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
	
	char *str = (char*)malloc((strlen(buffer) + 1)*sizeof(char));
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

sqlite3 *db = NULL;
tasklist rootlist = {.tasks = NULL, .ntasks = 0};

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
	new_tasks = (task*)realloc(tl->tasks, (tl->ntasks + 1)*sizeof(task));

	if (new_tasks == NULL) {
		handle_error("ERROR: task reallocation failed\n");
		return 1;
	}

	tl->tasks = new_tasks;
	++tl->ntasks;

	return 0;
}

bool tasklist_has_task(tasklist tl, const char *name) {
	if (name == NULL) return false;

	for (int i = 0; i < tl.ntasks; ++i) {
		if (strcmp(tl.tasks[i].name, name) == 0) {
			return true;
		}
	}

	return false;
}

task *tasklist_get_task(tasklist tl, int64_t id) {
	//get task
	for (int i = 0; i < tl.ntasks; ++i) {
		if(tl.tasks[i].id == id) {
			return &tl.tasks[i];
		}
	}

	//return null when no task is found
	return NULL;
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

void tasklist_remove_task(tasklist *tl, int64_t id) {
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

task *tasklist_get_task_by_id(tasklist tl, int64_t id) {
	for (int i = 0; i < tl.ntasks; ++i) {
		task *in = tasklist_get_task_by_id(tl.tasks[i].tl, id);
		if (in != NULL) {
			return in;
		}

		if (tl.tasks[i].id == id) {
			return &tl.tasks[i];
		}
	}

	return NULL;
}

/*TASK FUNCTIONS*/

task new_task(const char *name, const char *details, int64_t id) {
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
	int addfailure = tasklist_add_task(root == NULL ? &rootlist : &root->tl, branch);
	if (addfailure == 1) {
		return 1;
	}
	
	return 0;
}

int print_task(task tsk) {
	static int indent = -1;
	
	++indent;

	for (int i = 0; i < indent; ++i) printf("\t");

	printf(tsk.completed ? GREEN("#%d: %s\n") : RED("#%d: %s\n"), tsk.id, tsk.name);

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

void task_toggle_complete(task *tsk) {
	if (tsk == NULL) {
		handle_error("CANNOT COMPLETE NULL TASK");
		return;
	}

	if (db == NULL) {
		handle_error("CANNOT COMPLETE TASK, NO DATABASE FOUND");
		return;
	}

	//update database
	char *str_completed = malloc_sprintf("%d", !tsk->completed);
	char *str_id = malloc_sprintf(PRId64, tsk->id);
	int rc = sqlite3_exec_by_format(
		db,
		"UPDATE tasks SET completed = ? WHERE id = ?",
		NULL,
		NULL,
		str_completed,
		str_id
	);
	free(str_completed);
	free(str_id);

	if (rc) {
		handle_error("SQLITE FAILED");
		return;
	}

	tsk->completed = !tsk->completed;
}

/*TASKTREE FUNCTIONS*/
task *tasktree_get_task_by_id(int64_t id) {
	return tasklist_get_task_by_id(rootlist, id);
}
static int callback_load_child_tasks_from_db(void *in, int ncolumns, char **values, char **columns) {
	char *name = NULL;
	char *details = NULL;
	int64_t id = -1;
	bool completed = false;

	//load columns into variables
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

	task *parent = (task*)in;
	task tsk = new_task(name, details, id);
	tsk.completed = completed;

	task_add_task(parent, tsk);

	free(name);
	free(details);
	id = -1;
	return 0;
}

void load_child_tasks_from_db(task *parent) {
	int64_t parentid = 0;
	if (parent != NULL) {
		parentid = parent->id;
	}

	char *str_parentid = malloc_sprintf("%lld", parentid);
	sqlite3_exec_by_format(
		db,
		"SELECT * FROM tasks WHERE parent = ?",
		callback_load_child_tasks_from_db,
		parent,
		str_parentid
	);
	free(str_parentid);

	tasklist parentlist = parent == NULL ? rootlist : parent->tl;
	for(int i = 0; i < parentlist.ntasks; ++i) {
		load_child_tasks_from_db(&parentlist.tasks[i]);
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
		exit(1);
	}

	//generate array of the sql for all columns
	char *sql_generate_table = malloc_sprintf("CREATE TABLE tasks (");
	size_t sql_len = strlen(sql_generate_table);
	stringlist cols = new_stringlist();
	int COLUMNS_length = sizeof(COLUMNS)/sizeof(COLUMNS[0]);
	for (int i = 0; i < COLUMNS_length; ++i) {
		char *col_i = malloc_sprintf("%s %s, ", COLUMNS[i][0], COLUMNS[i][1]);
		stringlist_append(
			&cols,
			col_i
		);
		free(col_i);
		col_i = NULL;

		sql_len += strlen(cols.items[i]);
		sql_generate_table = (char*)realloc(sql_generate_table, sizeof(char)*(sql_len + 1));
		sql_generate_table = strcat(sql_generate_table, cols.items[i]);
	}
	sql_generate_table[sql_len - 2] = ')';
	sql_generate_table[sql_len - 1] = ';';
	
	//add table if it doesn't exist
	if (!sqlite3_has_table(db, "tasks")) {
		handle_error("table tasks not found\n");
		sqlite3_exec_by_format(
			db,
			sql_generate_table,
			NULL,
			NULL
		);
	}

	free(sql_generate_table);

	//add columns if they do not exist
	for (int i = 0; i < COLUMNS_length; ++i) {
		if (!sqlite3_table_has_column(db, "tasks", COLUMNS[i][0])) {
			sqlite3_exec_by_format(
				db,
				"ALTER TABLE tasks ADD %s",
				NULL,
				NULL,
				cols.items[i]
			);
		}
	}

	stringlist_free_elements(cols);
	load_tasks_from_db();
}

void tasktree_unload() {
	sqlite3_close(db);
	tasklist_free_elements(&rootlist);
}

void tasktree_print() {
	for(int i = 0; i < rootlist.ntasks; ++i) {
		print_task(rootlist.tasks[i]); 
	}
}

//TODO: make this neater
static void tasktree_remove_task_from_db(int64_t id);

static int callback_remove_task_from_db(void *val, int ncolumns, char **values, char **columns) {
	UNUSED(val);

	int64_t id = 0;

	for (int i = 0; i < ncolumns; ++i) {
		if (!strcmp(columns[i], "id")) {
			id = atoi(values[i]);
			break;
		}
	}

	tasktree_remove_task_from_db(id);
	return 0;
}

static void tasktree_remove_task_from_db(int64_t id) {
	char *str_id = malloc_sprintf("%d", id);
	sqlite3_exec_by_format(
		db,
		"DELETE FROM tasks WHERE id = ?;",
		callback_remove_task_from_db,
		NULL,
		str_id
	);
	free(str_id);
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

void tasktree_add_task(task tsk, task *parent) {
	printf("adding task\n");

	tasklist *parentlist = parent == NULL ? &rootlist : &parent->tl;                        //parent tasklist

	//create new task in memory
	tasklist_add_task(parentlist, new_task(tsk.name, tsk.details, tsk.id));

	//bypass database entry if database not found or task is already inserted
	if (db == NULL || tsk.id > 0) {
		printf("task already in db\n");
		return;
	}

	//enter new task into database
	int64_t parentid = 0;

	if (parent != NULL) {
		parentid = parent->id;
	}
	
	//execute sqlite code
	char *str_parentid = malloc_sprintf("%d", parentid);
	char sql_format[] = "INSERT INTO tasks (parent, name, details) VALUES (?, ?, ?);";

	sqlite3_exec_by_format(
		db,
		sql_format,
		NULL,
		NULL,
		str_parentid,
		tsk.name,
		tsk.details
	);

	free(str_parentid);

	//set id of task to the one given by the database
	parentlist->tasks[parentlist->ntasks - 1].id = sqlite3_last_insert_rowid(db);
}

