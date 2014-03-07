#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/input.h>


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

void get_screen_info(int fd, screen_conf_t *conf) {
    capabilities_t caps;
    if (ioctl(fd, EVIOCGBIT(EV_ABS, CAPLEN), caps) != CAPLEN) {
        fprintf(stderr, "Error reading device capabilities.\n");
        conf->type = INVALID;
        return;
    }

    struct input_absinfo abs;
    if (HAS_CAP(caps, ABS_MT_POSITION_X) &&
            HAS_CAP(caps, ABS_MT_POSITION_Y) &&
            HAS_CAP(caps, ABS_MT_TRACKING_ID)) {
        conf->type = MULTI_TOUCH_B;
        ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &abs);
        conf->xmin = abs.minimum;
        conf->xmax = abs.maximum;
        ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &abs);
        conf->ymin = abs.minimum;
        conf->ymax = abs.maximum;
    } else if (HAS_CAP(caps, ABS_X) && HAS_CAP(caps, ABS_Y)) {
        conf->type = SINGLE_TOUCH;
        ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &abs);
        conf->xmin = abs.minimum;
        conf->xmax = abs.maximum;
        ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &abs);
        conf->ymin = abs.minimum;
        conf->ymax = abs.maximum;
    } else {
        conf->type = INVALID;
    }
}

void find_touchscreen(screen_conf_t *conf) {
    char *filename;
    DIR *dir;
    struct dirent *de;
    const char *dirname = "/dev/input";
    dir = opendir(dirname);
    if (dir == NULL) {
        perror(dirname);
        exit(1);
    }
    strcpy(conf->path, dirname);
    filename = conf->path + strlen(conf->path);
    *filename++ = '/';
    while ((de = readdir(dir))) {
        if (strcmp(de->d_name, ".") == 0 ||
                strcmp(de->d_name, "..") == 0) {
            continue;
        }
        strcpy(filename, de->d_name);
        int fd = open(conf->path, O_RDONLY);
        if (fd < 0) {
            perror(conf->path);
            continue;
        } 
        get_screen_info(fd, conf);
        close(fd);
        if (conf->type != INVALID) {
            break;
        }
    }
}





void get_touchscreen(screen_conf_t *conf) {
    int exists = access(confpath, R_OK) == 0;
    FILE *f;
    if (exists) {
        f = fopen(confpath, "r");
        fscanf(f, devicename_fmt, conf->path);
        fscanf(f, screentype_fmt, (int*)&conf->type);
        fscanf(f, xmin_fmt, &conf->xmin);
        fscanf(f, xmax_fmt, &conf->xmax);
        fscanf(f, ymin_fmt, &conf->ymin);
        fscanf(f, ymax_fmt, &conf->ymax);
    } else {
        f = fopen(confpath, "w+");
        find_touchscreen(conf);
        fprintf(f, devicename_fmt, conf->path);
        fprintf(f, screentype_fmt, conf->type);
        fprintf(f, xmin_fmt, conf->xmin);
        fprintf(f, xmax_fmt, conf->xmax);
        fprintf(f, ymin_fmt, conf->ymin);
        fprintf(f, ymax_fmt, conf->ymax);
    }
    fclose(f);
}


int main() {
    screen_conf_t conf;
    get_touchscreen(&conf);
    printf("path=%s, type=%d\n", conf.path, conf.type);
    return 0;
}
