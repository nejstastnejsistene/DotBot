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

typedef enum { INVALID, SINGLE_TOUCH, MULTI_TOUCH_B } screen_type_t;

const char *conf = "/data/local/DotBot/touchscreen.conf";
const char *devicename_fmt = "device-name: %s\n";
const char *screentype_fmt = "screen-type: %d\n";

screen_type_t get_screen_type(fd) {
    capabilities_t caps;
    if (ioctl(fd, EVIOCGBIT(EV_ABS, CAPLEN), caps) != CAPLEN) {
        fprintf(stderr, "Error reading device capabilities.\n");
        return INVALID;
    }

    if (HAS_CAP(caps, ABS_MT_POSITION_X) &&
            HAS_CAP(caps, ABS_MT_POSITION_Y) &&
            HAS_CAP(caps, ABS_MT_TRACKING_ID)) {
        return MULTI_TOUCH_B;
    } else if (HAS_CAP(caps, ABS_X) && HAS_CAP(caps, ABS_Y)) {
        return SINGLE_TOUCH;
    } else {
        return INVALID;
    }
}

void find_touchscreen(char *devname, screen_type_t *type) {
    char *filename;
    DIR *dir;
    struct dirent *de;
    const char *dirname = "/dev/input";
    dir = opendir(dirname);
    if (dir == NULL) {
        perror(dirname);
        exit(1);
    }
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while ((de = readdir(dir))) {
        if (strcmp(de->d_name, ".") == 0 ||
                strcmp(de->d_name, "..") == 0) {
            continue;
        }
        strcpy(filename, de->d_name);
        int fd = open(devname, O_RDONLY);
        if (fd < 0) {
            continue;
        } 
        *type = get_screen_type(fd);
        close(fd);
        if (*type != INVALID) {
            break;
        }
    }
}


void get_touchscreen(char *devname, screen_type_t *type) {
    int exists = access(conf, R_OK) == 0;
    FILE *f;
    if (exists) {
        f = fopen(conf, "r");
        fscanf(f, devicename_fmt, devname);
        fscanf(f, screentype_fmt, (int*)type);
    } else {
        f = fopen(conf, "w+");
        find_touchscreen(devname, type);
        fprintf(f, devicename_fmt, devname);
        fprintf(f, screentype_fmt, *type);
    }
    fclose(f);
}


int main() {
    char devname[PATH_MAX];
    screen_type_t type;
    get_touchscreen(devname, &type);
    printf("path=%s, type=%d\n", devname, type);
    return 0;
}
