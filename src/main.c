#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "tasktree.h"
#include "util.h"

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

		switch (input.length) {
			case 0:
				printf("ERROR: null input\n");
				break;
			case 1:
				if (!strcmp(input.items[0], "new")) {
					printf("Enter id of parent task or \"0\" for root: ");
					int64_t id = atoi(get_input());
					printf("Enter name of task: ");
					char *taskname = get_input();
					printf("Enter any details: ");
					char *tasktext = get_input();

					task *parent = tasktree_get_task_by_id(id);

					task tsk = new_task(taskname, tasktext, -1);

					tasktree_add_task(tsk, parent);

					task_free_elements(tsk);
					free(taskname);
					free(tasktext);
				}
				else if (!strcmp(input.items[0], "list")) {
					tasktree_print();
				}
				else if (!strcmp(input.items[0], "done")) {
					stringlist_free_elements(input);
					tasktree_unload();

					return 0;
				}
				else {
					printf("command not found or wrong number of arguments");
				}
				break;
			case 2:
				if (!strcmp(input.items[0], "get")) {
					int64_t id = atoi(input.items[1]);
					task *tsk = tasktree_get_task_by_id(id);
					if (tsk == NULL) {
						printf("task %s not found\n", input.items[1]);
					}
					else {
						print_task(*tsk);
					}
				}
				else if (!strcmp(input.items[0], "complete")) {
					int64_t id = atoi(input.items[1]);
					task *tsk = tasktree_get_task_by_id(id);
					if (tsk == NULL) {
						printf("task %s not found\n", input.items[1]);
					}
					else {
						task_toggle_complete(tsk);
					}
				}
				else if (!strcmp(input.items[0], "modify")) {

				}
				else if (!strcmp(input.items[0], "remove")) {
					tasktree_remove_task(tasktree_get_task_by_id(atoi(input.items[1])));
				}
				else if (!strcmp(input.items[0], "deadline")) {
					task_set_column(tasktree_get_task_by_id(atoi(input.items[1])), DEADLINE, input.items[0]);
				}
				else {
					printf("command not found or wrong number of arguments");
				}
		}

		stringlist_free_elements(input);
	}

	return 0;
}
