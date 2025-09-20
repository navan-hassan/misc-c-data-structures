#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hash_table.h"
#include "quad_tree.h"

static void hash_table_example() {
    HashTable hash_table = {0};
    initialize_hash_table(&hash_table);

    const uint64_t max_count = 900;
    uint64_t k = 12;
    uint32_t v = 22222;

    hash_table_insert(&hash_table, k, v);
    uint32_t* result = hash_table_get_entry(&hash_table, k);
    assert(result != NULL);
    assert(*result == v);

    fprintf(stderr, "K: %zu, V: %d\n", k, *result);
    for (int i = 0; i < max_count; i++) {
        uint32_t value = (uint32_t)(i ^ (2 << i));
        uint64_t key = (i + 1);
        hash_table_insert(&hash_table, key, value);
    }

    assert(hash_table.count == max_count);

    for (int i = 0; i < max_count; i++) {
        uint32_t value = (uint32_t)(i ^ (2 << i));
        uint64_t key = (i + 1);
        assert(hash_table_contains_key(&hash_table, key));
        result = hash_table_get_entry(&hash_table, key);
        assert(*result == value);
    }

    for (int i = 0; i < max_count; i++) {
        uint64_t key = (i + 1);
        assert(hash_table_delete_entry(&hash_table, key));
        if ((key + 1) < max_count) {
            assert(hash_table_contains_key(&hash_table, (key + 1)));
        }
    }
    destroy_hash_table(&hash_table);
}

#define MAX_FOUND 100
static const Point points[] = {
    {
        .x = -5.0f,
        .y = -5.0f,
    },
    {
        .x = 5.0f,
        .y = -5.0f,
    },
    {
        .x = -5.0f,
        .y = 5.0f,
    },
    {
        .x = 5.0f,
        .y = 5.0f,
    },
    {
        .x = 1.0f,
        .y = 1.0f,
    },
    {
        .x = -1.0f,
        .y = -1.0f,
    },
    {
        .x = 6.0f,
        .y = 6.0f,
    },
    {
        .x = -6.0f,
        .y = -6.0f,
    },
};

static const size_t num_points = (size_t)(sizeof(points) / sizeof(points[0]));

static bool point_inside_list(Point* list, size_t count, Point target) {
    for (int i = 0; i < count; i++) {
        if (list[i].x == target.x && list[i].y == target.y)
            return true;
    }
    return false;
}

static void quad_tree_example() {
    Rect bounds = create_rect(0.0f, 0.0f, 10.0f, 10.0f);
    QuadTree* tree = create_new_tree(bounds);
    for (int i = 0; i < num_points; i++) {
        insert_point_into_quadtree(tree, points[i]);
    }
    print_quad_tree(tree, 0);

    Point points_found[MAX_FOUND];
    int found_count = 0;
    Rect range = create_rect(0.0f, 0.0f, 3.0f, 3.0f);
    search_space_in_tree(tree, range, points_found, &found_count, MAX_FOUND);

    fprintf(stderr, "Found %d points in range:\n", found_count);
    for (int i = 0; i < found_count; i++) {
        fprintf(stderr, "\t(%.2f, %.2f)\n", points_found[i].x, points_found[i].y);
    }

    Point buf[100];
    size_t buf_count = 0;
    memset(buf, 0, sizeof(buf));
    for (int i = 0; i < num_points; i++) {
        Point point = points[i];
        point.x += (point.x < 0.0f) ? -0.5f : 0.5f;
        point.y += (point.y < 0.0f) ? -0.5f : 0.5f;
        if (!point_inside_list(buf, buf_count, point)) {
            buf[buf_count] = point;
            ++buf_count;
        }
    }

    for (int i = 0; i < buf_count; i++) {
        insert_point_into_quadtree(tree, buf[i]);
    }

    print_quad_tree(tree, 0);
    for (int i = 0; i < buf_count; i++) {
        fprintf(stderr, "\n\n--------------------------\n");
        fprintf(stderr, "Removing Point (%.2f, %.2f)\n", buf[i].x, buf[i].y);
        assert(remove_point_from_quad_tree(tree, buf[i]));

        print_quad_tree(tree, 0);
    }
    fprintf(stderr, "--------------------------\n");
    print_quad_tree(tree, 0);

    free_quad_tree(tree);
}

int main() {
    hash_table_example();
    //quad_tree_example();
    return 0;
}