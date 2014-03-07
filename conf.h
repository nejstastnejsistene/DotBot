#ifndef CONF_H
#define CONF_H

#define CAPLEN (ABS_CNT / 8)
#define HAS_CAP(caps, cap) ((caps)[(cap) / 8] & 1 << ((cap) % 8))

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
} screen_conf_t;

const char *confpath = "/data/local/DotBot/touchscreen.conf";
const char *devicename_fmt = "device-name: %s\n";
const char *screentype_fmt = "screen-type: %d\n";
const char *xmin_fmt = "minimum-x: %d\n";
const char *xmax_fmt = "maximum-x: %d\n";
const char *ymin_fmt = "minimum-y: %d\n";
const char *ymax_fmt = "maximum-y: %d\n";

void get_touchscreen(screen_conf_t *conf);
void read_config(screen_conf_t *conf);
void update_config(screen_conf_t *conf);
void find_touchscreen(screen_conf_t *conf);
void get_screen_info(int fd, screen_conf_t *conf);

#endif // CONF_H
