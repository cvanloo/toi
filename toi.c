#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct Rectangle {
    int l, r, t, b;
} Rectangle;


typedef struct GlobalState {
} GlobalState;

GlobalState global;

void print_rectangle(const char *prefix, Rectangle x) {
    fprintf(stderr, "%s: %d -> %d; %d -> %d\n", prefix, x.l, x.r, x.t, x.b);
}

// Initialise a Rectangle structure with the provided values.
Rectangle rectangle_make(int l, int r, int t, int b);

// Returns true if the rectangle is 'valid', which I define to mean it has
// positive width and height.
bool rectangle_valid(Rectangle a);

// Compute the intersection of the rectangles, i.e. the biggest rectangle that
// fits into both. If the rectangles don't overlap, an invalid rectangle is
// returned (as per rectangle_valid).
Rectangle rectangle_intersection(Rectangle a, Rectangle b);

// Compute the smallest rectangle containing both of the input rectangles.
Rectangle rectangle_bounding(Rectangle a, Rectangle b);

// Returns true if all sides are equal.
bool rectangle_equals(Rectangle a, Rectangle b);

// Returns true if the pixel with its top-left at the given coordinate is
// contained inside the rectangle.
bool rectangle_contains(Rectangle a, int x, int y);

void string_copy(char **dst, size_t *ndst, const char *src, ptrdiff_t nsrc);

int main() {
    print_rectangle("make", rectangle_make(10, 20, 30, 40));
    print_rectangle("intersection", rectangle_intersection(rectangle_make(10, 20, 30, 40), rectangle_make(15, 25, 35, 45))); // 15 20 35 40
    print_rectangle("bounding", rectangle_bounding(rectangle_make(10, 20, 30, 40), rectangle_make(15, 25, 35, 45))); // 10 25 30 45
    fprintf(stderr, "valid: %d\n", rectangle_valid(rectangle_make(10, 20, 30, 40))); // true
    fprintf(stderr, "valid: %d\n", rectangle_valid(rectangle_make(20, 10, 30, 40))); // false
    fprintf(stderr, "equals: %d\n", rectangle_equals(rectangle_make(10, 20, 30, 40), rectangle_make(10, 20, 30, 40))); // true
    fprintf(stderr, "equals: %d\n", rectangle_equals(rectangle_make(10, 20, 30, 40), rectangle_make(15, 25, 35, 45))); // false
    fprintf(stderr, "contains: %d\n", rectangle_contains(rectangle_make(10, 20, 30, 40), 15, 35)); // true
    fprintf(stderr, "contains: %d\n", rectangle_contains(rectangle_make(10, 20, 30, 40), 25, 35)); // false

    char *dst = NULL;
    size_t ndst = 0;
    string_copy(&dst, &ndst, "Hello!", 6);
    fprintf(stderr, "'%.*s'\n", (int) ndst, dst);
    string_copy(&dst, &ndst, "World!", 6);
    fprintf(stderr, "'%.*s'\n", (int) ndst, dst);
    free(dst);
}

Rectangle rectangle_make(int l, int r, int t, int b) {
    Rectangle x;
    return x.l = l, x.r = r, x.t = t, x.b = b, x;
}

bool rectangle_valid(Rectangle a) {
    return a.l < a.r && a.t < a.b;
}

Rectangle rectangle_intersection(Rectangle a, Rectangle b) {
    if (a.l < b.l) a.l = b.l;
    if (a.t < b.t) a.t = b.t;
    if (a.r > b.r) a.r = b.r;
    if (a.b > b.b) a.b = b.b;
    return a;
}

Rectangle rectangle_bounding(Rectangle a, Rectangle b) {
    if (a.l > b.l) a.l = b.l;
    if (a.t > b.t) a.t = b.t;
    if (a.r < b.r) a.r = b.r;
    if (a.b < b.b) a.b = b.b;
    return a;
}

bool rectangle_equals(Rectangle a, Rectangle b) {
    return a.l == b.l && a.r == b.r && a.t == b.t && a.b == b.b;
}

bool rectangle_contains(Rectangle a, int x, int y) {
    // (x, y) gives the top-left corner of the pixel.
    // Therefore we use strict inequalities when comparing against the right and
    // bottom sides of the rectangle.
    return a.l <= x && a.r > x && a.t <= y && a.b > y;
}

void string_copy(char **dst, size_t *ndst, const char *src, ptrdiff_t nsrc) {
    if (nsrc == -1) nsrc = strlen(src);
    *dst = (char *) realloc(*dst, nsrc);
    *ndst = nsrc;
    memcpy(*dst, src, nsrc);
}
