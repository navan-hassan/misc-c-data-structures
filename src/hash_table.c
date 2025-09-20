#include "hash_table.h"
#include "list_utilities.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define INITIAL_CAPACITY 1024

typedef enum NodeStatus {
    NODE_STATUS_EMPTY,
    NODE_STATUS_DELETED,
    NODE_STATUS_OCCUPIED,
} NodeStatus;

struct Node {
    NodeStatus status;
    uint64_t key;
    size_t probe_distance;
    uint32_t value;
};

static XXH64_hash_t generate_random_seed() {
    uint64_t t = (uint64_t)time(NULL);
    uint64_t r = ((uint64_t)rand() << 32 | rand());
    return (XXH64_hash_t)(t ^ r);
}

static uint64_t hash_key(uint64_t key, XXH64_hash_t seed) {
    return XXH3_64bits_withSeed(&key, sizeof(key), seed);
}

static size_t get_hash_index(const HashTable* table, uint64_t key) {
    assert(table != NULL);
    assert(table->nodes != NULL);

    uint64_t hash = hash_key(key, table->seed);
    return (size_t)(hash % table->capacity);
}

static bool search_node_index(const HashTable* table, uint64_t key, size_t* index) {
    size_t hash_index = get_hash_index(table, key);
    size_t probing_distance = 0;

    bool found = false;
    while (probing_distance < table->nodes_visited) {
        Node* node = &table->nodes[hash_index];

        if (node->status == NODE_STATUS_EMPTY)
            break;

        if (node->status == NODE_STATUS_OCCUPIED && node->key == key) {
            if (index != NULL)
                *index = hash_index;
            found = true;
            break;
        }

        hash_index = (hash_index + 1) % table->capacity;
        ++probing_distance;
    }
    return found;
}

void initialize_hash_table(HashTable* table) {
    assert(table != NULL);

    memset(table, 0, sizeof(HashTable));
    table->seed = generate_random_seed();
    increase_list_capacity((void**)&table->nodes, &table->capacity, sizeof(Node), INITIAL_CAPACITY);
}

void destroy_hash_table(HashTable* table) {
    assert(table != NULL);
    if (table->nodes != NULL) {
        free(table->nodes);
    }
    memset(table, 0, sizeof(HashTable));
}

bool hash_table_contains_key(const HashTable* table, uint64_t key) {
    assert(table != NULL);
    return search_node_index(table, key, NULL);
}

void hash_table_insert(HashTable* table, uint64_t key, uint32_t value) {
    if (table->count * 2 >= table->capacity) {
        fprintf(stderr, "TODO: handle resize\n");
        return;
    }

    size_t index = get_hash_index(table, key);
    Node node_to_add = {
        .status = NODE_STATUS_OCCUPIED,
        .key = key,
        .value = value,
    };

    size_t probing_distance = 0;
    for (int i = 0; i < table->capacity; i++) {
        size_t probe_index = (index + i) % table->capacity;
        Node* cursor = &table->nodes[probe_index];
        if (cursor->status != NODE_STATUS_OCCUPIED || cursor->key == key) {
            if (cursor->status != NODE_STATUS_OCCUPIED)
                ++table->count;
            if (cursor->status == NODE_STATUS_EMPTY)
                ++table->nodes_visited;

            *cursor = node_to_add;
            cursor->probe_distance = probing_distance;
            return;
        }

        if (cursor->status == NODE_STATUS_OCCUPIED && probing_distance > cursor->probe_distance) {
            probing_distance = cursor->probe_distance;
            Node temp = *cursor;
            *cursor = node_to_add;
            node_to_add = temp;
        }
        ++probing_distance;

        if (probing_distance >= table->capacity)
            break;
    }
    fprintf(stderr, "Failed to insert element. No space in table\n");
}

static void do_backwards_shift(HashTable* table, size_t start_index) {
    assert(table != NULL);

    size_t prev = start_index;
    size_t next = (prev + 1) % table->capacity;
    while (table->nodes[next].status == NODE_STATUS_OCCUPIED && table->nodes[next].probe_distance > 0) {
        table->nodes[prev] = table->nodes[next];
        --table->nodes[prev].probe_distance;

        prev = next;
        next = (next + 1) % table->capacity;
    }
}

bool hash_table_delete_entry(HashTable* table, uint64_t key) {
    size_t index;
    bool success = search_node_index(table, key, &index);
    if (success) {
        table->nodes[index].status = NODE_STATUS_DELETED;
        --table->count;
        do_backwards_shift(table, index);
    }
    else {
        fprintf(stderr, "Error: Cannot find key %zu. Failed to delete\n", key);
    }
    return success;
}

uint32_t* hash_table_get_entry(HashTable* table, uint64_t key) {
    size_t index;
    bool success = search_node_index(table, key, &index);

    uint32_t* result = NULL;
    if (success)
        result = &table->nodes[index].value;

    return result;
}
