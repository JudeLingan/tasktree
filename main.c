#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "tasktree.h"

tasktree tree;

int main() {
	tasktree_load(&tree, "/home/jude/.local/share/tasktree/tasks.db");

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

			task tsk = new_task(&tree.tl, taskname, tasktext, -1);
			char *path = NULL;

			if (input.length == 2) {
				printf("%s\n", input.items[1]);
				path = input.items[1];
			}

			tasktree_add_task(&tree, tsk, path);
		}
		else if (!strcmp(input.items[0], "list")) {
			for(int i = 0; i < tree.tl.ntasks; ++i) {
				print_task(tree.tl.tasks[i]); }
		}
		else if (!strcmp(input.items[0], "get")) {
			if (input.length < 2) {
				printf("ERROR: 'get' command requires a task path\n");
			}
			else {
				task *tsk = tasktree_get_task(tree, input.items[1]);
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
				printf("ERROR: 'get' command requires a task path\n");
			}
			else {
				tasktree_remove_task_by_path(&tree, input.items[1]);
			}
		}
		else if (!strcmp(input.items[0], "done")) {

			stringlist_free_elements(input);

			tasktree_unload(&tree);

			break;
		}
		else {
			printf("command %s not found\n", input.items[0]);
		}

		stringlist_free_elements(input);
	}

	return 0;
}
