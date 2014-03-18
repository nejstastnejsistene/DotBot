#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "conf.h"

#define SHORT_DELAY 50000000
#define LONG_DELAY (3 * SHORT_DELAY)


void do_sleep(int nsec) {
    struct timespec t; 
    t.tv_sec = 0;
    t.tv_nsec = nsec;
    nanosleep(&t, NULL);
}

void event(int fd, int type, int code, int value) {
    struct input_event e;
    memset(&e, 0, sizeof(e));
    e.type = type;
    e.code = code;
    e.value = value;
    if (write(fd, &e, sizeof(e)) < sizeof(e)) {
        perror("error writing event");
        exit(1);
    }
    if (type == EV_SYN && code == SYN_REPORT) {
        do_sleep(SHORT_DELAY);
    }
}

void scale_coord(screen_conf_t *conf, int *x, int *y) {
    *x = conf->xmin + (*x - conf->xmin) * conf->xmax / 1200;
    *y = conf->ymin + (*y - conf->ymin) * conf->ymax / 1920;
}

static int tracking_id = 0;

void click(screen_conf_t *conf, int fd, int x, int y) {
    scale_coord(conf, &x, &y);
    switch (conf->type) {
        case SINGLE_TOUCH:
            event(fd, EV_ABS, ABS_X, x);
            event(fd, EV_ABS, ABS_Y, y);
            event(fd, EV_KEY, BTN_TOUCH, 1);
            event(fd, EV_SYN, SYN_REPORT, 0);
            event(fd, EV_KEY, BTN_TOUCH, 0);
            event(fd, EV_SYN, SYN_REPORT, 0);
            break;
        case MULTI_TOUCH_B:
            event(fd, EV_ABS, ABS_MT_TRACKING_ID, tracking_id++);
            event(fd, EV_ABS, ABS_MT_POSITION_X, x);
            event(fd, EV_ABS, ABS_MT_POSITION_Y, y);
            event(fd, EV_SYN, SYN_REPORT, 0);
            event(fd, EV_ABS, ABS_MT_TRACKING_ID, -1);
            event(fd, EV_SYN, SYN_REPORT, 0);
            break;
        default:
            fprintf(stderr, "UGH!\n");
            exit(1);
            break;
    }
}

void gesture(screen_conf_t *conf, int fd, int num_coords, coord_t *coords) {
    int i;
    switch (conf->type) {
        case SINGLE_TOUCH:
            scale_coord(conf, &coords[0].x, &coords[0].y);
            event(fd, EV_ABS, ABS_X, coords[0].x); 
            event(fd, EV_ABS, ABS_Y, coords[0].y); 
            event(fd, EV_KEY, BTN_TOUCH, 1);
            event(fd, EV_SYN, SYN_REPORT, 0);
            for (i = 1; i < num_coords; i++) {
                scale_coord(conf, &coords[0].x, &coords[0].y);
                event(fd, EV_ABS, ABS_X, coords[i].x); 
                event(fd, EV_ABS, ABS_Y, coords[i].y); 
                event(fd, EV_SYN, SYN_REPORT, 0);
            }
            event(fd, EV_KEY, BTN_TOUCH, 0);
            do_sleep(LONG_DELAY);
            event(fd, EV_SYN, SYN_REPORT, 0);
            break;
        case MULTI_TOUCH_B:
            event(fd, EV_ABS, ABS_MT_TRACKING_ID, tracking_id++);
            for (i = 0; i < num_coords; i++) {
                scale_coord(conf, &coords[i].x, &coords[i].y);
                event(fd, EV_ABS, ABS_MT_POSITION_X, coords[i].x); 
                event(fd, EV_ABS, ABS_MT_POSITION_Y, coords[i].y); 
                event(fd, EV_SYN, SYN_REPORT, 0);
            }
            event(fd, EV_ABS, ABS_MT_TRACKING_ID, -1);
            do_sleep(LONG_DELAY);
            event(fd, EV_SYN, SYN_REPORT, 0);
            break;
        default:
            fprintf(stderr, "UGH!\n");
            exit(1);
            break;
    }
}

int main() {
    screen_conf_t conf;
    coord_t coords[1000];
    int i = 0;
    while (i < 1000 && fscanf(stdin, "%d %d\n", &coords[i].x, &coords[i].y) == 2) {
        i++;
    }
    get_touchscreen(&conf);
    int fd = open(conf.path, O_RDWR);
    if (i == 1) {
        click(&conf, fd, coords[0].x, coords[0].y);
        click(&conf, fd, coords[0].x, coords[0].y);
    } else {
        gesture(&conf, fd, i, coords);
    }
    close(fd);
    return 0;
}
