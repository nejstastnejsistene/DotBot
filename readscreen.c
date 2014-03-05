#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#define COLOR_EQ(c, x) (((c).value & 0xffffff) == (x))
#define IS_WHITE(img, x, y) COLOR_EQ(get_pixel((img), (x) , (y)), 0xffffff)
// #define IS_BG(c) ((c).value == 0xffeaebea || (c).value == 0xffebebeb)

typedef struct {
    unsigned char r, b, g, a;
} rgba_t;

typedef union {
    unsigned int value;
    rgba_t rgba;
} color_t;

typedef struct {
    int width, height, f;
    color_t *mm;
} image_t;

color_t get_pixel(image_t *img, int x, int y) {
    return img->mm[img->width * y + x + 3];
}

double hue(color_t c) {
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

int color(color_t c) {
    return (int) (5 * hue(c) / (2 * M_PI) + 0.5);
}

void get_playing_area_dim_start(image_t *img, int *x_dest, int *y_dest) {
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

void get_playing_area_dim_end(image_t *img, int *x_dest, int *y_dest) {
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

void get_row_coords(image_t *img, int x0, int y0, int x1, int y1, int rows[6]) {
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

void get_col_coords(image_t *img, int x0, int y0, int x1, int y1, int cols[6]) {
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
    int fd = open(filename, O_RDONLY);
    image_t img;
    read(fd, &img.width,  sizeof(img.width));
    read(fd, &img.height, sizeof(img.height));
    read(fd, &img.f,      sizeof(img.f));
    if (img.f != 1) {
        fprintf(stderr, "f must equal 1\n");
        exit(1);
    }
    int size = img.width * img.height * sizeof(color_t) + 12;
    img.mm = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (img.mm != MAP_FAILED) {
        int x0, y0, x1, y1;
        get_playing_area_dim_start(&img, &x0, &y0);
        get_playing_area_dim_end(&img, &x1, &y1);

        int xs[6], ys[6];
        get_row_coords(&img, x0, y0, x1, y1, ys);
        get_col_coords(&img, x0, y0, x1, y1, xs);
        int r, c;
        for (c = 0; c < 6; c++) {
            for (r = 0; r < 6; r++) {
                printf("%d ", color(get_pixel(&img, xs[c], ys[r])));
            }
        }
        printf("\n");
        for (c = 0; c < 6; c++) {
            for (r = 0; r < 6; r++) {
                printf("%d %d\n", xs[c], ys[r]);
            }
        }

        munmap(img.mm, size);
    } else {
        perror("mmap");
    }
}

int main() {
    readscreen("/data/local/DotBot/screenshot.raw");
    return 0;
}
