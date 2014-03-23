#include "url.h"

#include <check.h>

static char buffer[1024];

#define go(input, expected_output) \
	strcpy(buffer, input); \
	remove_dot_segments(buffer); \
	ck_assert_msg(!strcmp(buffer, expected_output), "Expected " input " to turn into " expected_output " but got %s", buffer);

START_TEST(no_dots)
{
	go("", "");
	go("abc", "abc");
	go("/abc", "/abc");
	go("abc/", "abc/");
	go("/abc/", "/abc/");
	go("abc/", "abc/");
	go("abc/def", "abc/def");
	go("/abc/def", "/abc/def");
	go("abc/def/", "abc/def/");
	go("/abc/def/", "/abc/def/");
}
END_TEST

START_TEST(single_dots)
{
	go(".", "");
	go("/.", "/");
	go("./", "");
	go("/./", "/");
	go("./abc", "abc");
	go("/./abc", "/abc");
	go("abc/.", "abc/");
	go("abc/./", "abc/");
	go("abc/./def", "abc/def");
	go("/././abc/./././def/././.", "/abc/def/");
}
END_TEST

START_TEST(double_dots)
{
	go("..", "");
	go("/..", "/");
	go("../", "");
	go("/../", "/");
	go("../abc", "abc");
	go("/../abc", "/abc");
	go("abc/..", "/");
	go("abc/../", "/");
	go("abc/../def", "/def");
	go("/abc/def/../ghi/../../jkl/", "/jkl/")
}
END_TEST

START_TEST(rfc_examples)
{
	go("/a/b/c/./../../g", "/a/g");
	go("mid/content=5/../6", "mid/6");
}
END_TEST

Suite *url_suite() {
	Suite *s = suite_create("URL");

	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, no_dots);
	tcase_add_test(tc_core, single_dots);
	tcase_add_test(tc_core, double_dots);
	tcase_add_test(tc_core, rfc_examples);
	suite_add_tcase(s, tc_core);

	return s;
}

