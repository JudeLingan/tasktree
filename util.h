#include <sqlite3.h>

#ifndef UTIL_H
#define UTIL_H

typedef struct stringlist {
	char **items;
	int length;
} stringlist;

bool sqlite3_has_table(sqlite3 *database, char *table);
void sqlite3_exec_by_format(sqlite3 *database,  int (*callback)(void *, int, char **, char **), void *var, char *format, ...);
int append_string(char *s, char c);
stringlist split_by_char(char *str, char ch);
stringlist stringlist_input();
void stringlist_free_elements(stringlist sl);
void stringlist_free(stringlist *sl);

#endif
