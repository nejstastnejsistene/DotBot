#ifndef READSCREEN_H
#define READSCREEN_H


#define PIXEL_FORMAT_RGBA_8888 1
#define PIXEL_FORMAT_RGB_888 3
#define PIXEL_FORMAT_RGB_565 4

#define RGB_MASK 0xffffff
#define WHITE 0xffffff
#define COLOR_EQ(c, x) (((c).value & RGB_MASK) == (x))
#define IS_WHITE(img, x, y) COLOR_EQ(get_pixel((img), (x) , (y)), WHITE)


typedef struct {
    unsigned char r;
    unsigned char b;
    unsigned char g;
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
} color_t;

typedef struct {
    int fd;
    int width;
    int height;
    int format;
    int size;
    void *data;
} screencap_t;

typedef struct {
    int x;
    int y;
} coord_t;

void open_screencap(char *filename, screencap_t *img);
void close_screencap(screencap_t *img);

#endif // READSCREEN_H
