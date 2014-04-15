#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <android/log.h>

#include "screenshot.h"

int open_screenshot(const char *filename, screenshot_t *img) {
    img->fd = open(filename, O_RDONLY);
    if (img->fd == -1) {
        __android_log_print(ANDROID_LOG_ERROR, __FUNCTION__,
                "%s: %s", strerror(errno), filename);
        perror(filename);
        return -1;
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
            __android_log_print(ANDROID_LOG_ERROR, __FUNCTION__,
                    "Unsupported pixel format: %d", img->format);
            return -1;
    }
    img->size = img->width * img->height * bpp + 12;
    img->data = mmap(NULL, img->size, PROT_READ, MAP_PRIVATE, img->fd, 0);
    if (img->data == MAP_FAILED) {
        __android_log_print(ANDROID_LOG_ERROR, __FUNCTION__,
                "%s: Map failed", strerror(errno));
        return -1;
    }
    return 0;
}


void close_screenshot(screenshot_t *img) {
    close(img->fd);
    munmap(img->data, img->size);
}


pixel_t get_pixel(screenshot_t *img, int x, int y) {
    rgba_t pixel = { 0, 0, 0, 0 };

    /* Return the unintialized pixel if the requested coordinate
     * is out of bounds.
     */
    if (x < 0 || y < 0 || x >= img->width || y >= img->height) {
        return (pixel_t)pixel;
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
    return (pixel_t)pixel;
}
