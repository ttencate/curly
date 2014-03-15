#include "constants.h"

#include <check.h>
#include <string.h>

void setup() {
}

void teardown() {
}

START_TEST(test_creation_destruction)
{
	/*
	ck_assert(line_buffer_write_ptr(line_buffer) != NULL);
	ck_assert_int_eq(line_buffer_write_size(line_buffer), LINE_BUFFER_SIZE);
	*/
}
END_TEST

Suite *line_buffer_suite() {
	Suite *s = suite_create("LineBuffer");

	TCase *tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, test_creation_destruction);
	suite_add_tcase(s, tc_core);

	return s;
}
