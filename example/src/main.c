#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "hash_table.h"

static void hash_table_example() {
    HashTable hash_table = {0};
    initialize_hash_table(&hash_table);

    uint64_t k = 12;
    uint32_t v = 22222;

    hash_table_insert(&hash_table, k, v);
    uint32_t* result = hash_table_get_entry(&hash_table, k);
    assert(result != NULL);
    assert(*result == v);

    printf("K: %zu, V: %d", k, *result);
    for (int i = 0; i < 200; i++) {
        uint32_t value = (uint32_t)(i ^ (2 << i));
        uint64_t key = (i + 1);
        hash_table_insert(&hash_table, key, value);
    }

    for (int i = 0; i < 200; i++) {
        uint32_t value = (uint32_t)(i ^ (2 << i));
        uint64_t key = (i + 1);
        assert(hash_table_contains_key(&hash_table, key));
        result = hash_table_get_entry(&hash_table, key);
        assert(*result == value);
    }

    for (int i = 0; i < 200; i++) {
        uint64_t key = (i + 1);
        assert(hash_table_delete_entry(&hash_table, key));
    }
    destroy_hash_table(&hash_table);
}

int main() {
    hash_table_example();
    return 0;
}