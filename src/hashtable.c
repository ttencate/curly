#include "hashtable.h"

#include <stdlib.h>
#include <string.h>

void init_hashtable(Hashtable *ht) {
	ht->bucket_count = 16;
	ht->buckets = malloc(ht->bucket_count * sizeof(HashNode*));
	memset(ht->buckets, 0, ht->bucket_count * sizeof(HashNode*));
	ht->item_count = 0;
}

void free_hashtable(Hashtable *ht) {
	for (int i = 0; i < ht->bucket_count; i++) {
		HashNode *n = ht->buckets[i];
		while (n) {
			HashNode *m = n->next;
			free(n);
			n = m;
		}
	}
}

static int hash(const char *key) {
	(void) key; /* Unused */
	/* TODO implement a better hash function */
	return 42;
}

static int bucket_index(Hashtable *ht, const char *key) {
	int h = hash(key);
	return ((h % ht->bucket_count) + ht->bucket_count) % ht->bucket_count;
}

static HashNode *find_node(Hashtable *ht, const char *key) {
	int i = bucket_index(ht, key);
	for (HashNode *n = ht->buckets[i]; n; n = n->next) {
		if (!strcmp(key, n->key)) {
			return n;
		}
	}
	return NULL;
}

void *hashtable_get(Hashtable *ht, const char *key) {
	HashNode *n = find_node(ht, key);
	if (!n) {
		return NULL;
	}
	return n->value;
}

void hashtable_put(Hashtable *ht, const char *key, void *value) {
	HashNode *n = find_node(ht, key);
	if (!n) {
		/* TODO maybe resize */
		int i = bucket_index(ht, key);
		n = malloc(sizeof(HashNode));
		n->next = ht->buckets[i];
		ht->buckets[i] = n;
		ht->item_count++;
	}
	n->key = key;
	n->value = value;
}

bool hashtable_contains(Hashtable *ht, const char *key) {
	return find_node(ht, key) != NULL;
}

int hashtable_size(Hashtable *ht) {
	return ht->item_count;
}
