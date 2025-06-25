#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
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

void sqlite3_exec_by_format(sqlite3 *database,  int (*callback)(void *, int, char **, char **), void *var, const char *format, ...) {
	//copies formatted text to sql_code variable
	va_list args;

	va_start(args, format);
	char *sql_code = malloc_vsprintf(format, args);
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

	free(sql_code);
	free(sql_error);
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

	bool result = *has_table;
	free(has_table);
	
	return result;
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
stringlist new_stringlist() {
	stringlist out;
	out.items = NULL;
	out.length = 0;

	return out;
}

stringlist split_by_char(const char *str, char ch) {
	stringlist out = new_stringlist();

	if (str == NULL) {
		handle_error("ERROR: null string\n");
		return out;
	}

	int buffer_length = strlen(str) + 1;
	char buffer[buffer_length];
	strncpy(buffer, "", buffer_length);

	for (long unsigned int i = 0; i <= strlen(str); ++i) {
		if (str[i] == ch || str[i] == '\0') {
			if (stringlist_append(&out, buffer)) break;
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

	return out;
}

void stringlist_free_elements(stringlist sl) {
	for (int i = 0; i < sl.length; ++i) {
		printf("%d\n", i);
		free(sl.items[i]);
	}
	free(sl.items);
}

void stringlist_free(stringlist *sl) {
	stringlist_free_elements(*sl);
	free(sl);
}

void handle_error(char *err) {
	printf("%s\n", err);
}
