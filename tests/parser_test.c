#include "parser.h"

#include <check.h>
#include <stdlib.h>
#include <string.h>

static Request *request;
static Parser *parser;

static void setup() {
	request = malloc(sizeof(Request));
	request_init(request);

	parser = malloc(sizeof(Parser));
	parser_init(parser, request);
}

static void teardown() {
	free(parser);
	request_destroy(request);
	free(request);
}

static bool parse_request_with_nulls(char *request, int count) {
	memcpy(parser_get_write_ptr(parser), request, count);
	return parser_parse_bytes(parser, count);
}

static bool parse_request(char *request) {
	int count = strlen(request);
	return parse_request_with_nulls(request, count);
}

START_TEST(creation_destruction)
{
	ck_assert(parser_get_write_ptr(parser) != NULL);
	ck_assert(parser_get_write_size(parser) > 0);
}
END_TEST

START_TEST(valid_request_line)
{
	parse_request("GET / HTTP/1.1\r\n\r\n");
	ck_assert_str_eq(request->method, "GET");
	ck_assert_str_eq(request->uri, "/");
	ck_assert_int_eq(request->http_major, 1);
	ck_assert_int_eq(request->http_minor, 1);
}
END_TEST

START_TEST(empty_request_line)
{
	ck_assert(!parse_request("\r\n"));
}
END_TEST

START_TEST(trailing_garbage_in_request_line)
{
	ck_assert(!parse_request("GET / HTTP/1.1x\r\n\r\n"));
}
END_TEST

START_TEST(null_byte_in_request_line)
{
	ck_assert(!parse_request_with_nulls("GET / HTTP/1.1\0\r\n\r\n", 19));
}
END_TEST

START_TEST(null_byte_in_blank_line)
{
	ck_assert(!parse_request_with_nulls("GET / HTTP/1.1\r\n\0\r\n", 19));
}
END_TEST

START_TEST(missing_http_version)
{
	ck_assert(!parse_request("GET /\r\n\r\n"));
}
END_TEST

START_TEST(weird_http_version)
{
	parse_request("GET / HTTP/24601.8472\r\n\r\n");
	ck_assert_int_eq(request->http_major, 24601);
	ck_assert_int_eq(request->http_minor, 8472);
}
END_TEST

START_TEST(incomplete_http)
{
	ck_assert(!parse_request("GET / HT\r\n\r\n"));
}
END_TEST

START_TEST(incomplete_http_version)
{
	ck_assert(!parse_request("GET / HTTP/1.\r\n\r\n"));
}
END_TEST

START_TEST(invalid_http_version)
{
	ck_assert(!parse_request("GET / HTTP/1.x\r\n\r\n"));
}
END_TEST

START_TEST(encoded_uppercase_uri)
{
	ck_assert(parse_request("GET %2F HTTP/1.1\r\n\r\n"));
	ck_assert_str_eq(request->uri, "/");
}
END_TEST

START_TEST(encoded_lowercase_uri)
{
	ck_assert(parse_request("GET %2f HTTP/1.1\r\n\r\n"));
	ck_assert_str_eq(request->uri, "/");
}
END_TEST

START_TEST(encoded_mixed_uri)
{
	ck_assert(parse_request("GET /%7Ezaph%6fd/.%2e/foo.html HTTP/1.1\r\n\r\n"));
	ck_assert_str_eq(request->uri, "/~zaphod/../foo.html");
}
END_TEST

START_TEST(encoded_iso8859_uri)
{
	ck_assert(parse_request("GET /M%fcnchen.html HTTP/1.1\r\n\r\n"));
	ck_assert_str_eq(request->uri, "/M\xfcnchen.html");
}
END_TEST

START_TEST(encoded_uri_non_hex_char)
{
	ck_assert(!parse_request("GET /%3. HTTP/1.1\r\n\r\n"));
}
END_TEST

START_TEST(encoded_null_uri)
{
	ck_assert(!parse_request("GET /%00 HTTP/1.1\r\n\r\n"));
}
END_TEST

START_TEST(encoded_uri_missing_digit)
{
	ck_assert(!parse_request("GET /%2 HTTP/1.1\r\n\r\n"));
}
END_TEST

START_TEST(encoded_uri_missing_digits)
{
	ck_assert(!parse_request("GET /% HTTP/1.1\r\n\r\n"));
}
END_TEST

Suite *parser_suite() {
	Suite *s = suite_create("Parser");

	TCase *tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, creation_destruction);
	suite_add_tcase(s, tc_core);

	TCase *tc_request_line = tcase_create("Request line");
	tcase_add_checked_fixture(tc_request_line, setup, teardown);
	tcase_add_test(tc_request_line, valid_request_line);
	tcase_add_test(tc_request_line, empty_request_line);
	tcase_add_test(tc_request_line, trailing_garbage_in_request_line);
	tcase_add_test(tc_request_line, null_byte_in_request_line);
	tcase_add_test(tc_request_line, null_byte_in_blank_line);
	suite_add_tcase(s, tc_request_line);

	TCase *tc_http_version = tcase_create("HTTP version");
	tcase_add_checked_fixture(tc_http_version, setup, teardown);
	tcase_add_test(tc_http_version, missing_http_version);
	tcase_add_test(tc_http_version, weird_http_version);
	tcase_add_test(tc_http_version, incomplete_http);
	tcase_add_test(tc_http_version, incomplete_http_version);
	tcase_add_test(tc_http_version, invalid_http_version);
	suite_add_tcase(s, tc_http_version);

	TCase *tc_request_uri = tcase_create("Request URI");
	tcase_add_checked_fixture(tc_request_uri, setup, teardown);
	tcase_add_test(tc_request_uri, encoded_uppercase_uri);
	tcase_add_test(tc_request_uri, encoded_lowercase_uri);
	tcase_add_test(tc_request_uri, encoded_mixed_uri);
	tcase_add_test(tc_request_uri, encoded_iso8859_uri);
	tcase_add_test(tc_request_uri, encoded_uri_non_hex_char);
	tcase_add_test(tc_request_uri, encoded_null_uri);
	tcase_add_test(tc_request_uri, encoded_uri_missing_digit);
	tcase_add_test(tc_request_uri, encoded_uri_missing_digits);
	suite_add_tcase(s, tc_request_uri);

	return s;
}
