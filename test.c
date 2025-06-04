#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "tasktree.h"

static int total_tests = 0;
static int passed_tests = 0;

void run_test(char* (*test)(void*), void* val) {
	total_tests += 1;
	printf("Test #%d: ", total_tests);
	char *result = test(val);
	if (result == NULL) {
		printf("\x1B[32mPASSED\n");
		++passed_tests;
	}
	else {
		printf("\x1B[31mFAILED: %s\n", result);
		free(result);
	}
}

/*TESTS*/
char* test_malloc_snprintf(void* val) {
	char *testvar = NULL;
	char *out = NULL;
	testvar = malloc_sprintf("Number: %d String: \"%s\"\n", 5, "Hello World!");
	if (testvar == NULL) {
		out = "MALLOC_SPRINTF FAILED";
	}
	else if (strcmp(testvar, "Number: 5 String: \"Hello World!\"\n")) {
		out = testvar;
	}
	if (testvar != NULL) free(testvar);

	return out;
}

int main() {
	run_test(test_malloc_snprintf, NULL);
	return 0;
}
