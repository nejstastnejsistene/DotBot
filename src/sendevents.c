#include <stdio.h>
#include <fcntl.h>

#include "monkey.h"

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
