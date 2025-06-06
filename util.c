#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sqlite3.h>
#include "util.h"

void handle_error(char *err);

/*GENERAL FUNCTIONS*/
void sqlite3_exec_by_format(sqlite3 *database,  int (*callback)(void *, int, char **, char **), void *var, char *format, ...) {
	//copies formatted text to sql_code variable
	va_list args;

	va_start(args, format);
	size_t len = vsnprintf(NULL, 0, format, args) + 1;
	va_end(args);

	char sql_code[len];

	printf("len: %lu\nformat: %s\n", len, format);

	va_start(args, format);
	vsnprintf(sql_code, len, format, args);
	va_end(args);

	char* sql_error = NULL;

	//uses passed database to run sql_code, running callback on each item and generating sql_error
	sqlite3_exec(
		database,
		sql_code,
		callback,
		var,
		&sql_error
	);

	//handle sql_error
	if (sql_error != NULL) {
		printf("SQL ERROR: %s\n", sql_error);
		sqlite3_free(sql_error);
		sqlite3_close(database);
		exit(1);
	}
	else {
		printf("SQL \"%s\" SUCCESSFUL\n", sql_code);
	}

	free(sql_error);
}

int callback_set_true(void *setme, int ncolumns, char **vals, char **columns) {
	bool *boolset = (bool*)setme;
	*boolset = true;
	return 0;
}

bool sqlite3_has_table(sqlite3 *database, char *table) {
	bool* has_table = malloc(sizeof(bool));
	*has_table = false;
	sqlite3_exec_by_format(
		database,
		callback_set_true,
		has_table,
		"SELECT name FROM sqlite_master WHERE type='table' AND name='%s';",
		table
	);
	return has_table;
}

char* malloc_sprintf(const char* restrict format, ...) {
	va_list args;

	va_start(args, format);
	size_t len = vsnprintf(NULL, 0, format, args) + 1;
	va_end(args);

	char dest[len];

	printf("len: %lu\nformat: %s\n", len, format);

	va_start(args, format);
	vsnprintf(dest, len, format, args);
	va_end(args);

	return strdup(dest);
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
		handle_error("ERROR: realloc failed: null pointer exception\n");
		sl->items = NULL;
		sl->length = 0;
		return true;
	}

	sl->items = new_items;
	sl->items[sl->length - 1] = strdup(str);

	return false;
}

/*STRINGLIST FUNCTIONS*/
stringlist split_by_char(char *str, char ch) {
	int buffer_length = strlen(str) + 1;
	char buffer[buffer_length];
	strncpy(buffer, "", buffer_length);

	stringlist result;
	result.items = (char**)malloc(sizeof(char*));
	result.length = 0;

	if (str == NULL || buffer_length == 0) {
		handle_error("ERROR: null string\n");
		result.items = NULL;
		result.length = 0;
		return result;
	}

	for (long unsigned int i = 0; i <= strlen(str); ++i) {
		if (str[i] == ch || str[i] == '\0') {

			if (stringlist_append(&result, buffer)) break;

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

void handle_error(char *err) {
	printf(err);
}
