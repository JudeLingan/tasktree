#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "tasktree.h"

#if defined(_WIN32)
#define LOCALDBPATH "\\AppData\\Local\\tasktree.db"

char *get_db_path() {
	char *homedir = getenv("USERPROFILE");
	char *dbpath = (char*)calloc(strlen(LOCALDBPATH) + strlen(homedir) + 1, sizeof(char));
	strcpy(dbpath, homedir);
	strcat(dbpath, LOCALDBPATH);
	return dbpath;
}

#elif defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#define LOCALDBPATH "/.local/share/tasktree.db"

char *get_db_path() {
	struct passwd *pw = getpwuid(getuid());

	char *homedir = pw->pw_dir;
	char *dbpath = (char*)calloc(strlen(LOCALDBPATH) + strlen(homedir) + 1, sizeof(char));
	strcpy(dbpath, homedir);
	strcat(dbpath, LOCALDBPATH);
	return dbpath;
}

#endif

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

			int64_t *taskpath = string_to_id_path(input.items[1]);
			tasktree_add_task(&tsk, taskpath);
			free(taskpath);

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
				int64_t *taskpath = string_to_id_path(input.items[1]);
				task *tsk = tasktree_get_task(taskpath);
				free(taskpath);
				if (tsk == NULL) {
					printf("task %s not found\n", input.items[1]);
				}
				else {
					print_task(*tsk);
				}
			}
		}
		else if (!strcmp(input.items[0], "complete")) {
			if (input.length < 2) {
				printf("'get' command requires a task path\n");
			}
			else {
				int64_t *taskpath = string_to_id_path(input.items[1]);
				task *tsk = tasktree_get_task(taskpath);
				free(taskpath);
				if (tsk == NULL) {
					printf("task %s not found\n", input.items[1]);
				}
				else {
					task_toggle_complete(tsk);
				}
			}
		}
		else if (!strcmp(input.items[0], "remove")) {
			if (input.length < 2) {
				printf("'remove' command requires a task path\n");
			}
			else {
				int64_t *taskpath = string_to_id_path(input.items[1]);
				tasktree_remove_task_by_path(taskpath);
				free(taskpath);
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
