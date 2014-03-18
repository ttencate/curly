#include <check.h>
#include <stdlib.h>

Suite *hashtable_suite();
Suite *parser_suite();

int main(int argc, char **argv) {
	Suite *s = suite_create("Curly");
	SRunner *sr = srunner_create(s);
	srunner_add_suite(sr, hashtable_suite());
	srunner_add_suite(sr, parser_suite());

	srunner_run_all(sr, CK_NORMAL);

	int number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return number_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
