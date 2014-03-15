#include "linebuffer.h"

#include "constants.h"

#include <check.h>
#include <string.h>

static LineBuffer *line_buffer;
static char *lines[16];
static int line_index;

void setup() {
	line_buffer = alloc_line_buffer();
	line_index = 0;
}

void teardown() {
	free_line_buffer(line_buffer);
	line_buffer = NULL;
}

bool callback(char *line, void *callback_arg) {
	return true;
}

START_TEST(test_creation_destruction)
{
	ck_assert(line_buffer_write_ptr(line_buffer) != NULL);
	ck_assert_int_eq(line_buffer_write_size(line_buffer), LINE_BUFFER_SIZE);
}
END_TEST

START_TEST(test_append)
{
	const char *buffer = "hello\r\nworld\r\n";
	ck_assert(line_buffer_write_size(line_buffer) >= strlen(buffer) + 1);

	strcpy(line_buffer_write_ptr(line_buffer), buffer);
	line_buffer_process_appended_bytes(line_buffer, strlen(buffer), callback, NULL);

	ck_assert_int_eq(line_index, 2);
	ck_assert_str_eq(lines[0], "hello");
	ck_assert_str_eq(lines[1], "world");
}
END_TEST

Suite *line_buffer_suite() {
	Suite *s = suite_create("LineBuffer");

	TCase *tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, test_creation_destruction);
	tcase_add_test(tc_core, test_append);
	suite_add_tcase(s, tc_core);

	return s;
}
