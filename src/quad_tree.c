#include "quad_tree.h"
#include <assert.h>
#include <corecrt_search.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QUAD_TREE_INDEX_NE 0
#define QUAD_TREE_INDEX_NW 1
#define QUAD_TREE_INDEX_SE 2
#define QUAD_TREE_INDEX_SW 3

static void subdivide_quad_tree(QuadTree* tree) {
    assert(tree->is_leaf);

    float x = tree->bounds.center.x;
    float y = tree->bounds.center.y;
    float hw = tree->bounds.half_width / 2.0f;
    float hh = tree->bounds.half_height / 2.0f;

    QuadTree* northeast = create_new_tree(create_rect(x + hw, y - hh, hw, hh));
    QuadTree* northwest = create_new_tree(create_rect(x - hw, y - hh, hw, hh));
    QuadTree* southeast = create_new_tree(create_rect(x + hw, y + hh, hw, hh));
    QuadTree* southwest = create_new_tree(create_rect(x - hw, y + hh, hw, hh));

    for (int i = 0; i < tree->count; i++) {
        Point point = tree->points[i];
        if (insert_point_into_quadtree(northeast, point))
            continue;
        if (insert_point_into_quadtree(northwest, point))
            continue;
        if (insert_point_into_quadtree(southeast, point))
            continue;
        if (insert_point_into_quadtree(southwest, point))
            continue;
    }

    tree->children[QUAD_TREE_INDEX_NE] = northeast;
    tree->children[QUAD_TREE_INDEX_NW] = northwest;
    tree->children[QUAD_TREE_INDEX_SE] = southeast;
    tree->children[QUAD_TREE_INDEX_SW] = southwest;
    tree->is_leaf = false;
}

static uint32_t count_points_in_tree(const QuadTree* tree) {
    assert(tree != NULL);

    if (tree->is_leaf)
        return tree->count;
    uint32_t total = 0;
    for (int i = 0; i < QUAD_TREE_MAX_CHILDREN; i++) {
        total += count_points_in_tree(tree->children[i]);
    }
    return total;
}

static void collect_points_in_quad_tree(const QuadTree* tree, Point* buffer, uint32_t buffer_size, uint32_t* count) {
    if (*count >= buffer_size)
        return;
    if (tree->is_leaf) {
        for (int i = 0; i < tree->count; i++) {
            buffer[*count] = tree->points[i];
            ++(*count);
            if (*count >= buffer_size)
                break;
        }
    }
    else {
        for (int i = 0; i < QUAD_TREE_MAX_CHILDREN; i++) {
            collect_points_in_quad_tree(tree->children[i], buffer, buffer_size, count);
        }
    }
}

static void try_balance_quad_tree(QuadTree* tree) {
    if (tree == NULL || tree->is_leaf)
        return;

    uint32_t total_points = count_points_in_tree(tree);
    assert(total_points == tree->count);
    if (tree->count <= QUAD_TREE_MAX_POINTS) {
        fprintf(stderr, "Rebalancing Tree\n");
        Point buffer[QUAD_TREE_MAX_POINTS];
        uint32_t count = 0;
        collect_points_in_quad_tree(tree, buffer, QUAD_TREE_MAX_POINTS, &count);
        assert(count == tree->count);
        for (int i = 0; i < QUAD_TREE_MAX_CHILDREN; i++) {
            free_quad_tree(tree->children[i]);
        }
        memset(&tree->points, 0, sizeof(tree->points));
        tree->is_leaf = true;
        tree->count = count;
        for (int i = 0; i < count; i++) {
            tree->points[i] = buffer[i];
        }
    }
}

Rect create_rect(float x, float y, float half_width, float half_height) {
    Rect rect = {
        .center = {
            .x = x,
            .y = y,
        },
        .half_width = half_width,
        .half_height = half_height,
    };
    return rect;
}

QuadTree* create_new_tree(Rect bounds) {
    QuadTree* tree = (QuadTree*)malloc(sizeof(QuadTree));
    memset(tree, 0, sizeof(QuadTree));
    tree->bounds = bounds;
    tree->is_leaf = true;
    return tree;
}

bool is_point_inside_rect(Rect rect, Point point) {
    return (
        point.x >= rect.center.x - rect.half_width && point.x <= rect.center.x + rect.half_width && point.y >= rect.center.y - rect.half_height && point.y <= rect.center.y + rect.half_height);
}

bool is_point_inside_circle(Circle circle, Point point) {
    float dist_x = point.x - circle.center.x;
    float dist_y = point.y - circle.center.y;
    return ((dist_x * dist_x) + (dist_y * dist_y)) <= (circle.radius * circle.radius);
}

bool rects_intersect(Rect a, Rect b) {
    return !(
        b.center.x - b.half_width > a.center.x + a.half_width || b.center.x + b.half_width < a.center.x - a.half_width || b.center.y - b.half_height > a.center.y + a.half_height || b.center.y + b.half_height < a.center.y - a.half_height);
}

bool circle_rect_intersect(Circle circle, Rect rect) {
    float nearest_x = fmaxf(rect.center.x - rect.half_width, fminf(circle.center.x, rect.center.x + rect.half_width));
    float nearest_y = fmaxf(rect.center.y - rect.half_height, fminf(circle.center.y, rect.center.y + rect.half_height));
    float dist_x = circle.center.x - nearest_x;
    float dist_y = circle.center.y - nearest_y;
    return ((dist_x * dist_x) + (dist_y * dist_y)) <= (circle.radius * circle.radius);
}

bool insert_point_into_quadtree(QuadTree* tree, Point point) {
    if (!is_point_inside_rect(tree->bounds, point))
        return false;

    bool success = false;
    if (tree->is_leaf) {
        for (int i = 0; i < tree->count; i++) {
            if (tree->points[i].x == point.x && tree->points[i].y == point.y) {
                fprintf(stderr, "Point %.2f, %.2f already exists in tree. Skipping...\n", point.x, point.y);
                return false;
            }
        }

        if (tree->count < QUAD_TREE_MAX_POINTS) {
            tree->points[tree->count] = point;
            //++tree->count;
            success = true;
        }
        else {
            subdivide_quad_tree(tree);
            return insert_point_into_quadtree(tree, point);
        }
    }
    else {
        for (int i = 0; i < QUAD_TREE_MAX_CHILDREN; i++) {
            success = insert_point_into_quadtree(tree->children[i], point);
            if (success)
                break;
        }
    }

    if (success) {
        ++tree->count;
    }
    return success;
}

void free_quad_tree(QuadTree* tree) {
    if (tree == NULL)
        return;

    if (!tree->is_leaf) {
        for (int i = 0; i < QUAD_TREE_MAX_CHILDREN; i++) {
            free_quad_tree(tree->children[i]);
        }
    }

    fprintf(stderr, "Freeing Quad Tree\n");
    free(tree);
}

void search_space_in_tree(const QuadTree* tree, Rect range, Point* found, int* found_count, int max_count) {
    if (!rects_intersect(tree->bounds, range))
        return;

    if (tree->is_leaf) {
        for (int i = 0; i < tree->count; i++) {
            Point point = tree->points[i];
            if (!is_point_inside_rect(range, point))
                continue;

            if (*found_count < max_count) {
                found[*found_count] = point;
                (*found_count)++;
            }
            else {
                return;
            }
        }
    }
    else {
        for (int i = 0; i < QUAD_TREE_MAX_CHILDREN; i++) {
            search_space_in_tree(tree->children[i], range, found, found_count, max_count);
        }
    }
}

void search_circle_in_tree(const QuadTree* tree, Circle range, Point* found, int* found_count, int max_count) {
    if (!circle_rect_intersect(range, tree->bounds))
        return;

    if (tree->is_leaf) {
        for (int i = 0; i < tree->count; i++) {
            Point point = tree->points[i];
            if (!is_point_inside_circle(range, point))
                continue;

            if (*found_count < max_count) {
                found[*found_count] = point;
                (*found_count)++;
            }
            else {
                return;
            }
        }
    }
    else {
        for (int i = 0; i < QUAD_TREE_MAX_CHILDREN; i++) {
            search_circle_in_tree(tree->children[i], range, found, found_count, max_count);
        }
    }
}

void print_quad_tree(const QuadTree* tree, int level) {
    if (tree == NULL) return;
    for (int i = 0; i < level; i++) {
        fprintf(stderr, "  ");
    }

    fprintf(stderr, "Node at (%.2f, %.2f), half_w=%.2f, half_h=%.2f, leaf=%d, count=%d\n", tree->bounds.center.x, tree->bounds.center.y, tree->bounds.half_width, tree->bounds.half_height, tree->is_leaf, tree->count);

    if (tree->is_leaf) {
        for (int i = 0; i < tree->count; i++) {
            for (int j = 0; j < level + 1; j++) {
                fprintf(stderr, "  ");
            }
            fprintf(stderr, "Point: (%.2f, %.2f)\n", tree->points[i].x, tree->points[i].y);
        }
    }
    else {
        for (int i = 0; i < QUAD_TREE_MAX_CHILDREN; i++) {
            print_quad_tree(tree->children[i], level + 1);
        }
    }
}

static bool remove_point_from_leaf(QuadTree* tree, Point point) {
    assert(tree->is_leaf);
    for (int i = 0; i < tree->count; i++) {
        if (tree->points[i].x != point.x || tree->points[i].y != point.y)
            continue;

        for (int j = i; j < tree->count - 1; j++) {
            tree->points[j] = tree->points[j + 1];
        }

        return true;
    }
    return false;
}

bool remove_point_from_quad_tree(QuadTree* tree, Point point) {
    if (!is_point_inside_rect(tree->bounds, point))
        return false;

    bool success = false;
    if (tree->is_leaf) {
        success = remove_point_from_leaf(tree, point);
    }
    else {
        for (int i = 0; i < QUAD_TREE_MAX_CHILDREN; i++) {
            success = remove_point_from_quad_tree(tree->children[i], point);
            if (success)
                break;
        }
    }

    if (success) {
        --tree->count;
        try_balance_quad_tree(tree);
    }
    else {
        fprintf(stderr, "Failed to remove point %.2f, %.2f\n", point.x, point.y);
    }
    return success;
}