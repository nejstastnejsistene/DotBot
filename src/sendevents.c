#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "conf.h"
#include "dots.h"


#define NUM_POINTS 5
#define SHORT_DELAY 7500000
#define LONG_DELAY (20 * SHORT_DELAY)


void do_sleep(int nsec) {
    struct timespec t; 
    t.tv_sec = nsec / 1000000000;
    t.tv_nsec = nsec % 1000000000;
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

void setpos(screen_conf_t *conf, int fd, int x, int y) {
    scale_coord(conf, &x, &y);
    switch (conf->type) {
        case SINGLE_TOUCH:
            event(fd, EV_ABS, ABS_X, x);
            event(fd, EV_ABS, ABS_Y, y);
            break;
        case MULTI_TOUCH_B:
            event(fd, EV_ABS, ABS_MT_POSITION_X, x);
            event(fd, EV_ABS, ABS_MT_POSITION_Y, y);
            break;
        default:
            fprintf(stderr, "UGH!\n");
            exit(1);
            break;
    }
}

#define sync(fd) (event(fd, EV_SYN, SYN_REPORT, 0))

void finger_down(screen_conf_t *conf, int fd, int x, int y) {
    switch (conf->type) {
        case SINGLE_TOUCH:
            setpos(conf, fd, x, y);
            event(fd, EV_KEY, BTN_TOUCH, 1);
            sync(fd);
            break;
        case MULTI_TOUCH_B:
            event(fd, EV_ABS, ABS_MT_TRACKING_ID, tracking_id++);
            setpos(conf, fd, x, y);
            sync(fd);
            break;
        default:
            fprintf(stderr, "UGH!\n");
            exit(1);
            break;
    }
}

void finger_up(screen_conf_t *conf, int fd) {
    switch (conf->type) {
        case SINGLE_TOUCH:
            event(fd, EV_KEY, BTN_TOUCH, 0);
            sync(fd);
            break;
        case MULTI_TOUCH_B:
            event(fd, EV_ABS, ABS_MT_TRACKING_ID, -1);
            sync(fd);
            break;
        default:
            fprintf(stderr, "UGH!\n");
            exit(1);
            break;
    }
}

void click(screen_conf_t *conf, int fd, int x, int y) {
    finger_down(conf, fd, x, y);
    finger_up(conf, fd);
}


void gesture(screen_conf_t *conf, int fd, int num_coords, coord_t *coords) {
    int last_x = coords[0].x;
    int last_y = coords[0].y;
    finger_down(conf, fd, last_x, last_y);
    int i, j;
    for (i = 1; i < num_coords; i++) {
        int x = coords[i].x;
        int y = coords[i].y;
        for (j = 1; j < NUM_POINTS + 1; j++) {
            setpos(conf, fd, last_x + j * (x - last_x) / NUM_POINTS,
                             last_y + j * (y - last_y) / NUM_POINTS);
            sync(fd);
        }
        last_x = x;
        last_y = y;
    }
    printf("%d\n", LONG_DELAY % 1000000000);
    do_sleep(LONG_DELAY);
    finger_up(conf, fd);
}

int main() {
    screen_conf_t conf;
    coord_t coords[NUM_DOTS];
    int i = 0;
    while (i < NUM_DOTS &&
            fscanf(stdin, "%d %d\n", &coords[i].x, &coords[i].y) == 2) {
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
