#ifndef READSCREEN_H
#define READSCREEN_H

#include "dots.h"

#define PIXEL_FORMAT_RGBA_8888 1
#define PIXEL_FORMAT_RGB_888 3
#define PIXEL_FORMAT_RGB_565 4

#define RGB_MASK 0xffffff
#define WHITE 0xffffff
#define BLACK 0x000000
#define COLOR_EQ(c, x) (((c).value & RGB_MASK) == (x))
#define IS_WHITE(img, x, y) COLOR_EQ(get_pixel((img), (x) , (y)), WHITE)
#define IS_BLACK(img, x, y) COLOR_EQ(get_pixel((img), (x) , (y)), BLACK)


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

typedef struct {
    int x0;
    int y0;
    int x1;
    int y1;
} bounds_t;

typedef enum { LEFT, RIGHT, TOP, BOTTOM } edge_t;

void open_screencap(const char *filename, screencap_t *img);
void close_screencap(screencap_t *img);
pixel_t get_pixel(screencap_t *img, int x, int y);

double get_hue(pixel_t c);
color_t get_color(pixel_t c);

int find_edge(screencap_t *img, edge_t edge, int other_coord);
int get_offsets(screencap_t *img, edge_t e, bounds_t *bnds, int offs[GRID_DIM]);

int readscreen(screencap_t *img, int colors[NUM_DOTS], coord_t coords[NUM_DOTS]);

#endif // READSCREEN_H
