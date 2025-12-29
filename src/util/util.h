#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef UTIL_H
#define UTIL_H

#define UNUSED(x) (void)(x)
#define STR(x) #x
#define CONCAT(x, y) x##y

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

bool string_is_empty(const char *str);
char *malloc_sprintf(const char *format, ...);
bool is_pure_num(const char *str);
void handle_error(char *err);
char *get_input();
stringlist stringlist_input();
bool sqlite3_has_table(sqlite3 *database, const char *table);
bool sqlite3_table_has_column(sqlite3 *database, const char *table, const char *column);
int sqlite3_exec_by_format(sqlite3 *database, const char *format, int (*callback)(void *, int, char **, char **), void *var, ...);
int append_string(char *s, char c);
stringlist split_by_char(const char *str, char ch);
stringlist new_stringlist();
bool stringlist_append(stringlist *sl, char *str);
void stringlist_free_elements(stringlist sl);
void stringlist_free(stringlist *sl);

#endif
