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


int search_for_edge(screencap_t *img, edge_t edge, int other_coord) {
    int lo, hi, h, dir, a, b;
    b = other_coord;
    switch (edge) {
        case LEFT:   lo = 0; hi = img->width;  h = 1; dir = -1; break;
        case RIGHT:  lo = 0; hi = img->width;  h = 1; dir =  1; break;
        case TOP:    lo = 0; hi = img->height; h = 0; dir = -1; break;
        case BOTTOM: lo = 0; hi = img->height; h = 0; dir =  1; break;
    }
    int in_bounds, neighbor_in_bounds;
    while (lo <= hi) {
        a = (lo + hi) / 2;
        in_bounds = h ? IS_WHITE(img, a, b) : IS_WHITE(img, b, a);
        if (in_bounds) {
            neighbor_in_bounds =
                h ? IS_WHITE(img, a + dir, b) : IS_WHITE(img, b, a + dir);
            if (!neighbor_in_bounds) {
                return a;
            } else if (dir < 0) {
                hi = a - 1;
            } else {
                lo = a + 1;
            }
        } else if (dir < 0) {
            lo = a + 1;
        } else {
            hi = a - 1;
        }
    }
    return -1;
}


void get_offsets(screencap_t *img, edge_t e, bounds_t *bnds, int offs[6]) {
    int i;
    for (i = 0; i <= bnds->x1 - bnds->x0; i++) {
        if (!IS_WHITE(img, bnds->x0 + i, bnds->y0 + i)) {
            break;
        }
    }

    int a = ((e == LEFT) ? bnds->x0 : bnds->y0) + i;
    int b = ((e == LEFT) ? bnds->y0 : bnds->x0) + i;

    int j;
    for (j = 0; j < 6; j++) {
        int b0 = b;
        if (e == LEFT) {
            while (!IS_WHITE(img, a, ++b));
        } else {
            while (!IS_WHITE(img, ++b, a));
        }
        offs[j] = b0 + (b - b0) / 2;
        if (j < 5) {
            if (e == LEFT) {
                while (IS_WHITE(img, a, ++b));
            } else {
                while (IS_WHITE(img, ++b, a));
            }
        }
    }
}


void readscreen(char *filename) {
    screencap_t img;
    open_screencap(filename, &img);

    bounds_t bounds;
    bounds.x0 = search_for_edge(&img, LEFT,   img.height / 2);
    bounds.y0 = search_for_edge(&img, TOP,    bounds.x0);
    bounds.x1 = search_for_edge(&img, RIGHT,  img.height / 2);
    bounds.y1 = search_for_edge(&img, BOTTOM, bounds.x1);

    int xs[6], ys[6];
    get_offsets(&img, TOP,   &bounds, xs);
    get_offsets(&img, LEFT,  &bounds, ys);

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
