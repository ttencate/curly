#include "parser.h"

#include <check.h>
#include <stdlib.h>
#include <string.h>

static Request *request;
static Parser *parser;

static void setup() {
	request = malloc(sizeof(Request));
	init_request(request);

	parser = malloc(sizeof(Parser));
	init_parser(parser, request);
}

static void teardown() {
	free(parser);
	free_request(request);
	free(request);
}

static void parse_request_with_nulls(char *request, int count) {
	memcpy(parser_get_write_ptr(parser), request, count);
	parser_parse_bytes(parser, count);
}

static void parse_request(char *request) {
	int count = strlen(request);
	parse_request_with_nulls(request, count);
}

START_TEST(test_creation_destruction)
{
	ck_assert(parser_get_write_ptr(parser) != NULL);
	ck_assert(parser_get_write_size(parser) > 0);
}
END_TEST

START_TEST(test_valid_request_line)
{
	parse_request("GET / HTTP/1.1\r\n\r\n");
	ck_assert_str_eq(request->method, "GET");
	ck_assert_str_eq(request->uri, "/");
	ck_assert_int_eq(request->http_major, 1);
	ck_assert_int_eq(request->http_minor, 1);
}
END_TEST

Suite *parser_suite() {
	Suite *s = suite_create("Parser");

	TCase *tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, test_creation_destruction);
	tcase_add_test(tc_core, test_valid_request_line);
	suite_add_tcase(s, tc_core);

	return s;
}
