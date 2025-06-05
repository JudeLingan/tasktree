#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sqlite3.h>

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
	}
	else {
		printf("SQL \"%s\" SUCCESSFUL\n", sql_code);
	}

	free(sql_error);
}
