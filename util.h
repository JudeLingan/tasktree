#include <sqlite3.h>
#include <stdbool.h>

#ifndef UTIL_H
#define UTIL_H

#define UNUSED(x) (void)(x)

#define RED(string)     "\x1b[31m" string "\x1b[0m"
#define GREEN(string)   "\x1b[32m" string "\x1b[0m"
#define YELLOW(string)  "\x1b[33m" string "\x1b[0m"
#define BLUE(string)    "\x1b[34m" string "\x1b[0m"
#define MAGENTA(string) "\x1b[35m" string "\x1b[0m"
#define CYAN(string)    "\x1b[36m" string "\x1b[0m"

typedef struct stringlist {
	char **items;
	int length;
} stringlist;

bool string_is_empty(char *str);
char *malloc_sprintf(const char *format, ...);
bool is_pure_num(const char *str);
void handle_error(char *err);
bool sqlite3_has_table(sqlite3 *database, char *table);
void sqlite3_exec_by_format(sqlite3 *database,  int (*callback)(void *, int, char **, char **), void *var, const char *format, ...);
int append_string(char *s, char c);
stringlist split_by_char(const char *str, char ch);
stringlist stringlist_input();
void stringlist_free_elements(stringlist sl);
void stringlist_free(stringlist *sl);

#endif
