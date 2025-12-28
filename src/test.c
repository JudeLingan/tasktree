#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "util.h"
#include "tasktree.h"

/*TESTS*/
void test_malloc_snprintf() {
	char *testvar = NULL;
	testvar = malloc_sprintf("Number: %d String: \"%s\"\n", 5, "Hello World!");

	CU_ASSERT_PTR_NOT_NULL(testvar);
	CU_ASSERT_STRING_EQUAL(testvar, "Number: 5 String: \"Hello World!\"\n");

	if (testvar != NULL) free(testvar);
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
	ADD_TEST(util_suite, malloc_snprintf);

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	printf("%d\n", CU_get_number_of_suites_run());

	CU_cleanup_registry();

out:
	printf("testing complete");
	return CU_get_error();
}
