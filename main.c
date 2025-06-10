#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "tasktree.h"

#define LOCALDBPATH "/.local/share/tasktree.db"

char *get_db_path() {
	struct passwd *pw = getpwuid(getuid());

	char *homedir = pw->pw_dir;
	char *dbpath = (char*)calloc(strlen(LOCALDBPATH) + strlen(homedir) + 1, sizeof(char));
	strcpy(dbpath, homedir);
	strcat(dbpath, LOCALDBPATH);
	return dbpath;
}

int main() {
	char *dbpath = get_db_path();

	printf("%s\n", dbpath);

	tasktree_load(dbpath);

	free(dbpath);

	while(true) {
		//print text input
		printf("> ");

		stringlist input = stringlist_input();

		if (input.items == NULL || input.items[0] == 0) {
			printf("ERROR: null input\n");
			continue;
		}
		else if (!strcmp(input.items[0], "new")) {
			printf("Enter name of task: ");
			char *taskname = get_input();
			printf("Enter any details: ");
			char *tasktext = get_input();

			char *path = NULL;

			if (input.length == 2) {
				printf("%s\n", input.items[1]);
				path = input.items[1];
			}

			task tsk = new_task(taskname, tasktext, -1);

			tasktree_add_task(&tsk, path);

			task_free_elements(tsk);
			free(taskname);
			free(tasktext);
		}
		else if (!strcmp(input.items[0], "list")) {
			tasktree_print();
		}
		else if (!strcmp(input.items[0], "get")) {
			if (input.length < 2) {
				printf("'get' command requires a task path\n");
			}
			else {
				task *tsk = tasktree_get_task(input.items[1]);
				if (tsk == NULL) {
					printf("task %s not found\n", input.items[1]);
				}
				else {
					print_task(*tsk);
				}
			}
		}
		else if (!strcmp(input.items[0], "remove")) {
			if (input.length < 2) {
				printf("'remove' command requires a task path\n");
			}
			else {
				tasktree_remove_task_by_path(input.items[1]);
			}
		}
		else if (!strcmp(input.items[0], "done")) {

			stringlist_free_elements(input);

			tasktree_unload();

			break;
		}
		else {
			printf("command %s not found\n", input.items[0]);
		}

		stringlist_free_elements(input);
	}

	return 0;
}
