#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "readscreen.h"


void open_screencap(char *filename, screencap_t *img) {
    img->fd = open(filename, O_RDONLY);
    if (img->fd == -1) {
        perror(filename);
        exit(1);
    }
    read(img->fd, &img->width,  sizeof(img->width));
    read(img->fd, &img->height, sizeof(img->height));
    read(img->fd, &img->format, sizeof(img->format));
    int bpp;
    switch (img->format) {
        case PIXEL_FORMAT_RGBA_8888:
            bpp = 4;
            break;
        case PIXEL_FORMAT_RGB_888:
            bpp = 3;
            break;
        case PIXEL_FORMAT_RGB_565:
            bpp = 2;
            break;
        default:
            fprintf(stderr, "Unsupported pixel format: %d\n", img->format);
            exit(1);
    }
    img->size = img->width * img->height * bpp + 12;
    img->data = mmap(NULL, img->size, PROT_READ, MAP_PRIVATE, img->fd, 0);
    if (img->data == MAP_FAILED) {
        perror("open_screenshot");
        exit(1);
    }
}


void close_screencap(screencap_t *img) {
    close(img->fd);
    munmap(img->data, img->size);
}


color_t get_pixel(screencap_t *img, int x, int y) {
    rgba_t pixel;
    rgb_888_t rgb888;
    rgb_565_t rgb565;
    char *base = (char*)img->data + 12;
    int offset = img->width * y + x;
    switch (img->format) {
        case PIXEL_FORMAT_RGBA_8888:
            pixel = ((rgba_8888_t*)base)[offset];
            break;
        case PIXEL_FORMAT_RGB_888:
            rgb888 = ((rgb_888_t*)base)[offset];
            pixel.r = rgb888.r;
            pixel.g = rgb888.g;
            pixel.b = rgb888.b;
            pixel.a = 0xff;
            break;
        case PIXEL_FORMAT_RGB_565:
            rgb565 = ((rgb_565_t*)base)[offset];
            pixel.r = rgb565.r << 3;
            pixel.g = rgb565.g << 2;
            pixel.b = rgb565.b << 3;
            pixel.a = 0xff;
            break;
    }
    return (color_t)pixel;
}

double get_hue(color_t c) {
    unsigned char r, g, b;
    double x;
    r = c.rgba.r;
    g = c.rgba.g;
    b = c.rgba.b;
    x = atan2(sqrt(3) * (g - b), 2 * r - g - b);
    if (x < 0) {
        return -x;
    } else {
        return 2 * M_PI - x;
    }
}

int get_color(color_t c) {
    return (int) (5 * get_hue(c) / (2 * M_PI) + 0.5);
}

void get_playing_area_dim_start(screencap_t *img, int *x_dest, int *y_dest) {
    int x, y;
    int lo = 0;
    int hi = img->width;
    y = img->height / 2;
    while (lo <= hi) {
        x = (lo + hi) / 2;
        if (IS_WHITE(img, x, y)) {
            if (!IS_WHITE(img, x - 1, y)) {
                break;
            } else {
                hi = x - 1;
            }
        } else {
            lo = x + 1;
        }
    }

    lo = 0;
    hi = img->height;
    while (lo <= hi) {
        y = (lo + hi) / 2;
        if (IS_WHITE(img, x, y)) {
            if (!IS_WHITE(img, x, y - 1)) {
                break;
            } else { 
                hi = y - 1;
            }
        } else {
            lo = y + 1;
        }
    }

    *x_dest = x;
    *y_dest = y;
}

void get_playing_area_dim_end(screencap_t *img, int *x_dest, int *y_dest) {
    int x, y;
    int lo = 0;
    int hi = img->width;
    y = img->height / 2;
    while (lo <= hi) {
        x = (lo + hi) / 2;
        if (IS_WHITE(img, x, y)) {
            if (IS_WHITE(img, x + 1, y)) {
                lo = x + 1;
            } else {
                break;
            }
        } else {
            hi = x - 1;
        }
    }

    lo = 0;
    hi = img->height;
    while (lo <= hi) {
        y = (lo + hi) / 2;
        if (IS_WHITE(img, x, y)) {
            if (IS_WHITE(img, x, y + 1)) {
                lo = y + 1;
            } else { 
                break;
            }
        } else {
            hi = y - 1;
        }
    }

    *x_dest = x;
    *y_dest = y;
}

void get_row_coords(screencap_t *img, int x0, int y0, int x1, int y1, int rows[6]) {
    int x, y;
    for (y = y0; y <= y1; y++) {
        for (x = x0; x <= x0 + (x1 - x0) / 4; x++) {
            if (!IS_WHITE(img, x, y)) {
                int i;
                for (i = 0; i < 6; i++) {
                    int y0 = y;
                    while (!IS_WHITE(img, x, ++y));
                    rows[i] = y0 + (y - y0) / 2;
                    if (i < 5) {
                        while (IS_WHITE(img, x, ++y));
                    } else {
                        return;
                    }
                }
                return;
            }
        }
    }
}

void get_col_coords(screencap_t *img, int x0, int y0, int x1, int y1, int cols[6]) {
    int x, y;
    for (x = x0; x <= x1; x++) {
        for (y = y0; y <= y0 + (y1 - y0) / 4; y++) {
            if (!IS_WHITE(img, x, y)) {
                int i;
                for (i = 0; i < 6; i++) {
                    int x0 = x;
                    while (!IS_WHITE(img, ++x, y));
                    cols[i] = x0 + (x - x0) / 2;
                    if (i < 5) {
                        while (IS_WHITE(img, ++x, y));
                    } else {
                        return;
                    }
                }
                return;
            }
        }
    }
}


void readscreen(char *filename) {
    screencap_t img;
    open_screencap(filename, &img);

    int x0, y0, x1, y1;
    get_playing_area_dim_start(&img, &x0, &y0);
    get_playing_area_dim_end(&img, &x1, &y1);

    int xs[6], ys[6];
    get_row_coords(&img, x0, y0, x1, y1, ys);
    get_col_coords(&img, x0, y0, x1, y1, xs);

    coord_t coords[36];
    int r, c;
    for (c = 0; c < 6; c++) {
        for (r = 0; r < 6; r++) {
            coords[6*c+r].x = xs[c];
            coords[6*c+r].y = ys[r];
        }
    }

    for (c = 0; c < 6; c++) {
        for (r = 0; r < 6; r++) {
            coord_t coord = coords[6*c+r];
            color_t color = get_pixel(&img, coord.x, coord.y);
            printf("%d ", get_color(color));
        }
    }
    printf("\n");
    for (c = 0; c < 6; c++) {
        for (r = 0; r < 6; r++) {
            coord_t coord = coords[6*c+r];
            printf("%d %d\n", coord.x, coord.y);
        }
    }
}

int main() {
    readscreen("/data/local/DotBot/screenshot.raw");
    return 0;
}
