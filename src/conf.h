#ifndef CONF_H
#define CONF_H

#include <limits.h>
#include <linux/input.h>

#define CAPLEN (ABS_CNT / 8)
#define HAS_CAP(caps, cap) ((caps)[(cap) / 8] & 1 << ((cap) % 8))

#define confpath       ((const char *)"/data/local/DotBot/touchscreen.conf")
#define devicename_fmt ((const char *)"device-name: %s\n")
#define screentype_fmt ((const char *)"screen-type: %d\n")
#define xmin_fmt       ((const char *)"minimum-x: %d\n")
#define xmax_fmt       ((const char *)"maximum-x: %d\n")
#define ymin_fmt       ((const char *)"minimum-y: %d\n")
#define ymax_fmt       ((const char *)"maximum-y: %d\n")

typedef unsigned char capabilities_t[CAPLEN];

typedef enum {
    INVALID,
    SINGLE_TOUCH,
    MULTI_TOUCH_A,
    MULTI_TOUCH_B,
} screen_type_t;

typedef struct {
    char path[PATH_MAX];
    screen_type_t type;
    int xmin;
    int xmax;
    int ymin;
    int ymax;
    int width;
    int height;
} screen_conf_t;

void get_touchscreen(screen_conf_t *conf);
void read_config(screen_conf_t *conf);
void update_config(screen_conf_t *conf);
void find_touchscreen(screen_conf_t *conf);
void get_screen_info(int fd, screen_conf_t *conf);

#endif // CONF_H
