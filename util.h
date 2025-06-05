#include <sqlite3.h>

#ifndef UTIL_H
#define UTIL_H

void sqlite3_exec_by_format(sqlite3 *database,  int (*callback)(void *, int, char **, char **), void *var, char *format, ...);

#endif
