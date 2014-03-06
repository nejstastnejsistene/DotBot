#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "readscreen.h"


void open_screencap(const char *filename, screencap_t *img) {
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
    rgba_t pixel = { 0, 0, 0, 0 };

    /* Return the unintialized pixel if the requested coordinate
     * is out of bounds.
     */
    if (x < 0 || y < 0 || x >= img->width || y >= img->height) {
        return (color_t)pixel;
    }

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

    /* Return the uninitialized pixel if the format is unknown. */
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


/* Perform a binary search for the edges of the white playing area.
 * This takes fixed coordinate value `other_coord`, and calculates
 * and returns the other coordinate. */
int find_edge(screencap_t *img, edge_t edge, int other_coord) {

    /* Bookkeeping so the same code can do a search for all four
     * directions. `lo` and `hi` are the boundaries for the binary
     * search. `a` is the coordinate we're searching for, `b` is the
     * its fixed coordinate. `h` is whether or not the direction
     * is horizontal, and `dir` is whether the edge we are looking
     * for is greater than or less than the playing area with respect
     * to whichever axis we're searching on.
     */
    int lo, hi, h, dir, a, b;
    b = other_coord;
    switch (edge) {
        case LEFT:   lo = 0; hi = img->width;  h = 1; dir = -1; break;
        case RIGHT:  lo = 0; hi = img->width;  h = 1; dir =  1; break;
        case TOP:    lo = 0; hi = img->height; h = 0; dir = -1; break;
        case BOTTOM: lo = 0; hi = img->height; h = 0; dir =  1; break;
    }

    /* Binary search. */
    int in_bounds, neighbor_in_bounds;
    while (lo <= hi) {
        a = (lo + hi) / 2;

        /* Is the current midpoint in the white area? */
        in_bounds = h ? IS_WHITE(img, a, b) : IS_WHITE(img, b, a);
        if (in_bounds) {

            /* If the current midpoint is in the white area, and its
             * neighbor in the direction of `edge` is not, we have found
             * the edge pixel.
             */
            neighbor_in_bounds =
                h ? IS_WHITE(img, a + dir, b) : IS_WHITE(img, b, a + dir);
            if (!neighbor_in_bounds) {
                return a;

            /* Otherwise, adjust the boundary towards the edge of the
             * white area. */
            } else if (dir < 0) {
                hi = a - 1;
            } else {
                lo = a + 1;
            }

        /* If not in bounds, adjust the boundary towards the white area. */
        } else if (dir < 0) {
            lo = a + 1;
        } else {
            hi = a - 1;
        }
    }

    /* If the search had succeeded, it would have returned already. */
    return -1;
}


/* Finds the offsets of rows or columns of dots within `bnds`. For the
 * TOP edge, it finds the center of each row of dots in terms of their
 * y coordinates. For the LEFT edge, it does the same with the x
 * coordinates of the center of each column. Returns 0 for success and
 * -1 for failure.
 */
int get_offsets(screencap_t *img, edge_t e, bounds_t *bnds, int offs[6]) {

    /* Only use this function on the top or left edges. */
    if (!(e == TOP || e == LEFT)) {
        fprintf(stderr, "getoffsets() expects either TOP or LEFT edge\n");
        return -1;
    }

    /* Shoot out at a 45 degree angle from the corner of the boundary
     * to find the start of the top left dot. This assumes that the
     * playing area is approximately square.
     */
    int i;
    for (i = 0; i <= bnds->x1 - bnds->x0; i++) {
        if (!IS_WHITE(img, bnds->x0 + i, bnds->y0 + i)) {
            break;
        }
    }

    /* Get our coordinates straight: `a` is the one that's changing, and
     * `b` is fixed.
     */
    int a = ((e == TOP) ? bnds->x0 : bnds->y0) + i;
    int b = ((e == TOP) ? bnds->y0 : bnds->x0) + i;

    for (i = 0; i < 6; i++) {
        
        /* At the beginning of this loop, we should be at the beginning
         * of a dot. Record the beginning in `a0`.
         */
        int a0 = a;

        /* Move to the end of the dot. */
        if (e == TOP) {
            while (a < bnds->x1 && !IS_WHITE(img, ++a, b));
        } else {
            while (a < bnds->y1 && !IS_WHITE(img, b, ++a));
        }

        /* Take the mean of the beginning and end of the dot to get
         * its center. */
        offs[i] = a0 + (a - a0) / 2;

        /* If the current offset is the same as the previous offset,
         * something has gone wrong.
         */
        if (i > 0 && offs[i] == offs[i - 1]) {
            return -1;
        }

        /* If there are more dots to be found, find the beginning of the
         * next dot.
         */
        if (i < 5) {
            if (e == TOP) {
                while (a < bnds->x1 && IS_WHITE(img, ++a, b));
            } else {
                while (a < bnds->y1 && IS_WHITE(img, b, ++a));
            }
        }
    }

    /* Success if we've gotten this far. */
    return 0;
}


int readscreen(screencap_t *img, int colors[36], coord_t coords[36]) {
    bounds_t bounds;
    bounds.x0 = find_edge(img, LEFT,   img->height / 2);
    bounds.y0 = find_edge(img, TOP,    bounds.x0);
    bounds.x1 = find_edge(img, RIGHT,  img->height / 2);
    bounds.y1 = find_edge(img, BOTTOM, bounds.x1);

    if (bounds.x0 < 0 || bounds.y0 < 0 || bounds.x1 < 0 || bounds.y1 < 0) {
        return -1;
    }

    int xs[6], ys[6];
    if (get_offsets(img, TOP,   &bounds, xs) < 0) return -1;
    if (get_offsets(img, LEFT,  &bounds, ys) < 0) return -1;

    int r, c;
    for (c = 0; c < 6; c++) {
        for (r = 0; r < 6; r++) {

            /* If the pixel where we are expecting a dot is white,
             * the dots are probably not done falling yet.
             */
            color_t dot = get_pixel(img, xs[c], ys[r]);
            if (COLOR_EQ(dot, WHITE)) {
                return -1;
            }

            colors[6*c+r]= get_color(dot);
            coords[6*c+r].x = xs[c];
            coords[6*c+r].y = ys[r];
        }
    }

    return 0;
}


int main() {
    int colors[36];
    coord_t coords[36];

    const char *filename = "/data/local/DotBot/screenshot.raw";

    screencap_t img;
    open_screencap(filename, &img);
    int ret = readscreen(&img, colors, coords);
    close_screencap(&img);

    if (ret < 0) {
        fprintf(stderr, "bummer\n");
        return 1; 
    }

    int r, c;
    for (c = 0; c < 6; c++) {
        for (r = 0; r < 6; r++) {
            printf("%d ", colors[6*c+r]);
        }
    }
    printf("\n");
    for (c = 0; c < 6; c++) {
        for (r = 0; r < 6; r++) {
            coord_t coord = coords[6*c+r];
            printf("%d %d\n", coord.x, coord.y);
        }
    }

    return 0;
}
