#include "hashtable.h"

#include <stdint.h>
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

/* Murmur3_32 hash; see http://en.wikipedia.org/wiki/MurmurHash.
 * Non-static for testing only. */
uint32_t hash(const char *key) {
	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;
	const uint32_t r1 = 15;
	const uint32_t r2 = 13;
	const uint32_t m = 5;
	const uint32_t n = 0xe6546b64;

	uint32_t hash = 0;

	int len = strlen(key);
	const uint32_t *chunks = (const uint32_t*) key;
	int chunk_count = len / 4;
	for (int i = 0; i < chunk_count; i++) {
		uint32_t k = chunks[i];
		k *= c1;
		k = (k << r1) | (k >> (32 - r1));
		k *= c2;
		hash |= k;
		hash = (hash << r2) | (hash >> (32 - r2));
		hash = hash * m + n;
	}

	uint32_t remaining_bytes = 0;
	const char *remaining = key + chunk_count * 4;
	switch (len % 4) {
		case 3:
			remaining_bytes |= remaining[2] << 16;
		case 2:
			remaining_bytes |= remaining[1] << 8;
		case 1:
			remaining_bytes |= remaining[0];

			remaining_bytes *= c1;
			remaining_bytes = (remaining_bytes << r1) | (remaining_bytes >> (32 - r1));
			remaining_bytes *= c2;
			hash ^= remaining_bytes;
	}

	hash ^= (uint32_t) len;
	hash ^= hash >> 16;
	hash *= 0x85ebca6b;
	hash ^= hash >> 13;
	hash *= 0xc2b2ae35;
	hash ^= hash >> 16;

	return hash;
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
