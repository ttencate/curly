#include "hashtable.h"

#include <check.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static Hashtable ht;

static const char *k = "foo", *l = "bar", *m = "biz";
static const char *v = "Raphael", *w = "Leonardo";

static void setup() {
	init_hashtable(&ht);
}

static void teardown() {
	free_hashtable(&ht);
}

uint32_t hash(const char *key);

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

START_TEST(resizes)
{
	char keys[1000][4];
	for (int i = 0; i < 1000; i++) {
		sprintf(keys[i], "%d", i);
		hashtable_put(&ht, keys[i], (void*) (i % 2 == 0 ? v : w));
	}
	ck_assert_msg(ht.bucket_count == 2048, "bucket count is %d", ht.bucket_count);
	for (int i = 0; i < 1000; i++) {
		ck_assert_msg(hashtable_contains(&ht, keys[i]), "element %d not found", i);
	}
}
END_TEST

START_TEST(hash_distributes)
{
	int counts[10];
	memset(counts, 0, sizeof(counts));
	for (int i = 0; i < 1000; i++) {
		char key[4];
		sprintf(key, "%d", i);
		int index = ((hash(key) % 10) + 10) % 10;
		counts[index]++;
	}
	for (int i = 0; i < 10; i++) {
		ck_assert_msg(counts[i] > 50, "bucket %d received %d elements", i, counts[i]);
		ck_assert_msg(counts[i] < 150, "bucket %d received %d elements", i, counts[i]);
	}
}
END_TEST

Suite *hashtable_suite() {
	Suite *s = suite_create("Hashtable");

	TCase *tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, empty);
	tcase_add_test(tc_core, put_then_get);
	tcase_add_test(tc_core, put_multiple);
	tcase_add_test(tc_core, hash_distributes);
	tcase_add_test(tc_core, resizes);
	suite_add_tcase(s, tc_core);

	return s;
}
