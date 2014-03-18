#include "hashtable.h"

#include <check.h>

static Hashtable ht;

static const char *k = "foo", *l = "bar", *m = "biz";
static const char *v = "Raphael", *w = "Leonardo", *x = "Donatello";

static void setup() {
	init_hashtable(&ht);
}

static void teardown() {
	free_hashtable(&ht);
}

START_TEST(empty)
{
	ck_assert(hashtable_size(&ht) == 0);
	ck_assert(!hashtable_contains(&ht, k));
	ck_assert(!hashtable_contains(&ht, l));
	ck_assert(!hashtable_contains(&ht, m));
	ck_assert(hashtable_get(&ht, k) == NULL);
	ck_assert(hashtable_get(&ht, l) == NULL);
	ck_assert(hashtable_get(&ht, m) == NULL);
}
END_TEST

START_TEST(put_then_get)
{
	hashtable_put(&ht, k, (void*) v);
	ck_assert(hashtable_size(&ht) == 1);
	ck_assert(hashtable_contains(&ht, k));
	ck_assert(!hashtable_contains(&ht, l));
	ck_assert_str_eq(hashtable_get(&ht, k), v);
}
END_TEST

START_TEST(put_multiple)
{
	hashtable_put(&ht, k, (void*) v);
	hashtable_put(&ht, l, (void*) w);
	ck_assert(hashtable_size(&ht) == 2);
	ck_assert(hashtable_contains(&ht, k));
	ck_assert(hashtable_contains(&ht, l));
	ck_assert(!hashtable_contains(&ht, m));
	ck_assert_str_eq(hashtable_get(&ht, k), v);
	ck_assert_str_eq(hashtable_get(&ht, l), w);
}
END_TEST

Suite *hashtable_suite() {
	Suite *s = suite_create("Hashtable");

	TCase *tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, empty);
	tcase_add_test(tc_core, put_then_get);
	tcase_add_test(tc_core, put_multiple);
	suite_add_tcase(s, tc_core);

	return s;
}
