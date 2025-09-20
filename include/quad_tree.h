#pragma once

#include <stdbool.h>
#include <stdint.h>
typedef struct Point {
    float x;
    float y;
} Point;

typedef struct Rect {
    Point center;
    float half_width;
    float half_height;
} Rect;

typedef struct Circle {
    Point center;
    float radius;
} Circle;

#define QUAD_TREE_MAX_CHILDREN 4
#define QUAD_TREE_MAX_POINTS   4

typedef struct QuadTree {
    Rect bounds;
    bool is_leaf;
    uint32_t count;
    union {
        Point points[QUAD_TREE_MAX_POINTS];
        struct QuadTree* children[QUAD_TREE_MAX_CHILDREN];
    };
} QuadTree;

bool is_point_inside_rect(Rect rect, Point point);
bool is_point_inside_circle(Circle circle, Point point);
bool rects_intersect(Rect a, Rect b);
bool circle_rect_intersect(Circle circle, Rect rect);

QuadTree* create_new_tree(Rect bounds);
Rect create_rect(float x, float y, float half_width, float half_height);
bool insert_point_into_quadtree(QuadTree* tree, Point point);
bool remove_point_from_quad_tree(QuadTree* tree, Point point);
void free_quad_tree(QuadTree* tree);
void search_space_in_tree(const QuadTree* tree, Rect range, Point* found, int* found_count, int max_count);
void search_circle_in_tree(const QuadTree* tree, Circle range, Point* found, int* found_count, int max_count);
void print_quad_tree(const QuadTree* tree, int level);