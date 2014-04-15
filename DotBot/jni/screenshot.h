#ifndef SCREENSHOT_H
#define SCREENSHOT_H

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
} screenshot_t;

int open_screenshot(const char *filename, screenshot_t *img);
void close_screenshot(screenshot_t *img);
pixel_t get_pixel(screenshot_t *img, int x, int y);

#endif // SCREENSHOT_H
