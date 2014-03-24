#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <stdbool.h>

/* A simple hashtable implementation mapping null-terminated strings to void
 * pointers. Each bucket is a linked list of HashNodes. The table does not own
 * its keys or values; the caller is responsible for freeing them.
 */

struct HashNode_;

typedef struct HashNode_ {
	const char *key;
	void *value;
	struct HashNode_ *next;
} HashNode;

typedef struct {
	HashNode **buckets;
	int bucket_count;
	int item_count;
} Hashtable;

void hashtable_init(Hashtable *ht);
void hashtable_destroy(Hashtable *ht);

void *hashtable_get(Hashtable *ht, const char *key);
void hashtable_put(Hashtable *ht, const char *key, void *value);
bool hashtable_contains(Hashtable *ht, const char *key);
int hashtable_size(Hashtable *ht);

#endif
