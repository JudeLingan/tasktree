#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "util.h"
#include "tasktree.h"

/* GLOBAL MACROS */
#define STR(x) #x
#define CONCAT(x, y) x##y

static int total_tests = 0;
static int passed_tests = 0;

void run_test(char* (*test)(void*), void* val) {
	total_tests += 1;
	printf("Test #%d: ", total_tests);
	char *result = test(val);
	if (result == NULL) {
		printf(BLUE("PASSED\n"));
		++passed_tests;
	}
	else {
		printf(RED("FAILED") "%s\n", result);
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

void test_is_pure_num() {
	CU_ASSERT_TRUE(is_pure_num("0123456789"));
	CU_ASSERT_FALSE(is_pure_num("a0123456789"));
	CU_ASSERT_FALSE(is_pure_num("0123456789a"));
	CU_ASSERT_FALSE(is_pure_num("012abc3456789"));
	CU_ASSERT_FALSE(is_pure_num(""));
	CU_ASSERT_FALSE(is_pure_num(NULL));
}

int main() {
#define ADD_TEST(s, t) CU_add_test((s), STR(t), CONCAT(test_, t))

	if (CU_initialize_registry() != CUE_SUCCESS) {
		goto out;
	}

	CU_pSuite util_suite = CU_add_suite("util", NULL, NULL);
	if (util_suite == NULL) {
		CU_cleanup_registry();
		goto out;
	}

	ADD_TEST(util_suite, is_pure_num);

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_suite(util_suite);
	printf("%d\n", CU_get_number_of_suites_run());

	CU_cleanup_registry();

out:
	printf("testing complete");
	return CU_get_error();
}