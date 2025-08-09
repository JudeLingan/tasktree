#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <sqlite3.h>
#include "util.h"

/*INTERNAL FUNCTIONS*/

static int callback_set_true(void *setme, int ncolumns, char **vals, char **columns) {
	//mark unused variables
	UNUSED(ncolumns);
	UNUSED(vals);
	UNUSED(columns);

	//set setme to true
	bool *boolset = (bool*)setme;
	*boolset = true;
	return 0;
}

static char *malloc_vsprintf(const char* restrict format, va_list args) {
	va_list args2;

	va_copy(args2, args);
	size_t len = vsnprintf(NULL, 0, format, args2) + 1;
	va_end(args2);
	char *result = (char*)malloc(len*sizeof(char));

	vsnprintf(result, len, format, args);

	return result;
}

/*GENERAL FUNCTIONS*/

bool string_is_empty(char *str) {
	if (str == NULL) {
		return true;
	}
	if (!strcmp(str, "")) {
		return true;
	}
	return false;
}

bool is_pure_num(const char *str) {
	for (int i = 0; str[i] != '\0'; ++i) {
		printf("%d", i);
		if(!isdigit(str[i]))
			return false;
	}
	return true;
}

char *malloc_sprintf(const char* format, ...) {
	va_list args;
	va_start(args, format);
	char *out = malloc_vsprintf(format, args);
	va_end(args);
	return out;
}

int append_string(char *s, char c) {
	int len = strlen(s);
	s[len] = c;
	s[len + 1] = '\0';
	return 0;
}

bool stringlist_append(stringlist *sl, char *str) {
	sl->length += 1;
	char **new_items = (char**)realloc(sl->items, sizeof(char*)*(sl->length));

	if (new_items == NULL) {
		handle_error("realloc failed: null pointer exception\n");
		sl->items = NULL;
		sl->length = 0;
		return true;
	}

	sl->items = new_items;
	sl->items[sl->length - 1] = strdup(str);

	return false;
}

/*STRINGLIST FUNCTIONS*/

stringlist new_stringlist() {
	stringlist out;
	out.items = NULL;
	out.length = 0;

	return out;
}

// returns a stringlist wher char* "str" is split by character "ch" with regards to escape character "\"
stringlist split_by_char(const char *str, char ch) {
	stringlist out = new_stringlist();

	if (str == NULL) {
		handle_error("null string\n");
		return out;
	}

	int buffer_length = strlen(str) + 1;
	char buffer[buffer_length];
	strncpy(buffer, "", buffer_length);

	long unsigned int end = strlen(str);

	for (long unsigned int i = 0; i <= end; ++i) {
		//add buffer to stringlist at split character or null terminator
		if (str[i] == ch || str[i] == '\0') {
			if (stringlist_append(&out, buffer)) break;
			strncpy(buffer, "", buffer_length);
		} 
		//allows escaping characters
		else if (str[i] == '\\') {
			//prevents escaping the null terminator
			if (str[i + 1] == '\0')
				continue;

			++i;
			append_string(buffer, str[i]);
		}
		//add character to buffer when it is not the split character
		else {
			append_string(buffer, str[i]);
		}
	}

	return out;
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

void handle_error(char *err) {
	printf("ERROR: %s\n", err);
}

void sqlite3_exec_by_format(sqlite3 *database,  int (*callback)(void *, int, char **, char **), void *var, const char *format, ...) {
	//initial variable declarations
	va_list args;
	sqlite3_stmt *stmt = NULL;
	char *sql_error = NULL;

	sqlite3_prepare_v2(database, format,-1, &stmt, NULL);

	va_start(args, format);
	int num_vals = 0;
	for (int i = 0; format[i]; ++i) {
		//execute when a new parameter slot is found
		if (format[i] == '?') {
			++num_vals;
			char *param = va_arg(args, char*);
			int rc = sqlite3_bind_text(stmt, num_vals, param, -1, SQLITE_TRANSIENT);

			//return and output error if binding fails
			if (rc != SQLITE_OK) {
				char *err = malloc_sprintf("binding sqlite param failed: %s", param);
				handle_error(err);
				free(err);
				return;
			}
		}
	}
	va_end(args);

	//call statement
	int rc;
	do {
		rc = sqlite3_step(stmt);

		if (rc == SQLITE_ROW) {
			//get values and names of all columns
			char **vals = NULL;
			char **names = NULL;
			int i;
			for (i = 0; i < sqlite3_data_count(stmt); ++i) {
				//get column name
				const char *name = (const char*)sqlite3_column_name(stmt, i);
				if (name == NULL) {
					handle_error("failed to retrieve column data");
					return;
				}
				else {
					names = (char**)realloc(names, (i + 2)*sizeof(char*));
					names[i] = strdup(name);
				}

				//get column value
				const char *val = (const char*)sqlite3_column_text(stmt, i);
				if (val == NULL) {
					handle_error("failed to retrieve column data");
					return;
				}
				else {
					vals = (char**)realloc(vals, (i + 2)*sizeof(char*));
					vals[i] = strdup(val);
				}
			}
			vals[i] = NULL;
			names[i] = NULL;

			//run callback function and assign it to variable "success"
			int failure = (*callback)(var, i, vals, names);

			if (failure != 0) {
				handle_error("callback failed");
				return;
			}
		}
		else if (rc != SQLITE_DONE) {
			handle_error("sqlite3_step failed");
			return;
		}
	} while (rc != SQLITE_DONE);

	//handle sql_error
	if (sql_error != NULL) {
		printf("SQL ERROR: %s\n", sql_error);
		sqlite3_free(sql_error);
		sqlite3_close(database);
		exit(1);
	}

	sqlite3_finalize(stmt);
	sqlite3_free(sql_error);
}

bool sqlite3_has_table(sqlite3 *database, char *table) {
	bool *has_table = malloc(sizeof(bool));
	*has_table = false;
	sqlite3_exec_by_format(
		database,
		callback_set_true,
		has_table,
		"SELECT name FROM sqlite_master WHERE type='table' AND name=?;",
		table
	);

	bool result = *has_table;
	free(has_table);
	
	return result;
}
