#ifndef SCREENREADER_H
#define SCREENREADER_H

#include "screenshot.h"

typedef struct {
    int x;
    int y;
} coord_t;

typedef enum {
    EMPTY = -1,
    RED,
    YELLOW,
    GREEN,
    BLUE,
    PURPLE,
    NUM_COLORS
} color_t;

jobject try_dots_grid_screen(JNIEnv *env, screenshot_t *img);
jobject new_unknown_screen(JNIEnv *env, screenshot_t *img);

jobject get_dot_color(JNIEnv *env, color_t color);

#endif // SCREENREADER_H
