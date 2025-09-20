#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <xxhash.h>

typedef struct Node Node;

typedef struct HashTable {
    size_t capacity;
    size_t count;
    size_t nodes_visited;
    Node* nodes;
    XXH64_hash_t seed;
} HashTable;

void initialize_hash_table(HashTable* table);
void destroy_hash_table(HashTable* table);
bool hash_table_contains_key(const HashTable* table, uint64_t key);
bool hash_table_insert(HashTable* table, uint64_t key, uint32_t value);
bool hash_table_delete_entry(HashTable* table, uint64_t key);
uint32_t* hash_table_get_entry(HashTable* table, uint64_t key);