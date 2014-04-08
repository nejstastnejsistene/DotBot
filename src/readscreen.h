#ifndef READSCREEN_H
#define READSCREEN_H

#include "dots.h"

#define PIXEL_FORMAT_RGBA_8888 1
#define PIXEL_FORMAT_RGB_888 3
#define PIXEL_FORMAT_RGB_565 4

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} rgba_8888_t;

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} rgb_888_t;

typedef struct {
    unsigned short r : 5; 
    unsigned short g : 6; 
    unsigned short b : 5; 
} rgb_565_t;

typedef rgba_8888_t rgba_t;

typedef union {
    unsigned int value;
    rgba_t rgba;
} pixel_t;

typedef struct {
    int fd;
    int width;
    int height;
    int format;
    int size;
    void *data;
} screencap_t;

typedef enum { LEFT, RIGHT, TOP, BOTTOM } edge_t;

void open_screencap(const char *filename, screencap_t *img);
void close_screencap(screencap_t *img);
pixel_t get_pixel(screencap_t *img, int x, int y);

#define IS_BG(img, x, y) (is_bg(get_pixel(img, x, y)))
int is_bg(pixel_t pixel);

double get_hue(pixel_t c);
color_t get_color(pixel_t c);

int find_edge(screencap_t *img, edge_t edge, int other_coord);
int get_offsets(screencap_t *img, edge_t e, int offs[GRID_DIM]);

int readscreen(screencap_t *img, int colors[NUM_DOTS], int rows[NUM_ROWS], int cols[NUM_COLS]);

#endif // READSCREEN_H
