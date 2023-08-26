#define PLATFORM_LINUX

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef PLATFORM_WIN32
#define Rectangle W32Rectangle
#include <windows.h>
#undef Rectangle
#endif

#ifdef PLATFORM_LINUX
#define Window X11Window
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#undef Window
#endif

/*
 * Rectangles
*/

typedef struct Rectangle {
    int l, r, t, b;
} Rectangle;


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

/*
 * Windows
*/

typedef struct Window {
    uint32_t *bits;
    int width, height;

#ifdef PLATFORM_WIN32
    HWND hwnd;
    bool trackingLeave;
#endif

#ifdef PLATFORM_LINUX
    X11Window window;
    XImage *image;
#endif
} Window;

void initialise();
Window *window_create(const char *title, int w, int h);
int message_loop();

/*
 * Main
*/

typedef struct GlobalState {
    Window **windows;
    size_t window_count;

#ifdef PLATFORM_LINUX
    Display *display;
    Visual *visual;
    Atom window_closed_id;
#endif
} GlobalState;

GlobalState global;


int main() {
    initialise();
    window_create("Hello, World!", 300, 200);
    window_create("Hello, Moon!", 300, 200);
    return message_loop();
}

/*
 * Implement Rectangle
*/

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

/*
 * Implement Window
*/

#ifdef PLATFORM_WIN32

// TODO: once I'm on Windows

#endif

#ifdef PLATFORM_LINUX

Window *_find_window(X11Window window) {
    for (uintptr_t i = 0; i < global.window_count; ++i) {
        if (global.windows[i]->window == window) {
            return global.windows[i];
        }
    }
    return NULL;
}

Window *window_create(const char *title, int w, int h) {
    Window *window = (Window *) calloc(1, sizeof(Window));
    global.windows = realloc(global.windows, sizeof(Window *) * (++global.window_count));
    global.windows[global.window_count-1] = window;

    XSetWindowAttributes attributes = {};
    window->window = XCreateWindow(global.display,
                                   DefaultRootWindow(global.display),
                                   0, 0, // origin
                                   w, h,
                                   0, 0, // border width, depth
                                   InputOutput,
                                   CopyFromParent,
                                   CWOverrideRedirect,
                                   &attributes);
    XStoreName(global.display, window->window, title);
    XSelectInput(global.display, window->window,
                 SubstructureNotifyMask | ExposureMask | PointerMotionMask |
                 ButtonPressMask | ButtonReleaseMask | KeyPressMask |
                 KeyReleaseMask | StructureNotifyMask | EnterWindowMask |
                 LeaveWindowMask | ButtonMotionMask | KeymapStateMask |
                 FocusChangeMask | PropertyChangeMask);
    XMapRaised(global.display, window->window);
    XSetWMProtocols(global.display, window->window, &global.window_closed_id, 1 /* count */);
    window->image = XCreateImage(global.display, global.visual,
                                 24 /* depth */,
                                 ZPixmap,
                                 0 /* offset */,
                                 NULL /* data */,
                                 10, 10, 32, 0 /* width, height, bitmap_pad, bytes_per_line */);
    return window;
}

int message_loop() {
    while (true) {
        XEvent event;
        XNextEvent(global.display, &event);

        if (event.type == ClientMessage && (Atom) event.xclient.data.l[0] == global.window_closed_id) {
            return 0;
        } else if (event.type == ConfigureNotify) {
            Window *window = _find_window(event.xconfigure.window);
            if (!window) continue;

            if (window->width != event.xconfigure.width || window->height != event.xconfigure.height) {
                window->width = event.xconfigure.width;
                window->height = event.xconfigure.height;
                window->image->width = window->width;
                window->image->height = window->height;
                window->image->bytes_per_line = window->width * 4;
                window->image->data = (char *) window->bits;
            }
        }
    }
}

void initialise() {
    global.display = XOpenDisplay(NULL);
    global.visual = XDefaultVisual(global.display, 0 /* screen number */);
    global.window_closed_id = XInternAtom(global.display, "WM_DELETE_WINDOW", 0 /* only if exists */);
}

#endif
