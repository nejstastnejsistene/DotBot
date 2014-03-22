#ifndef MONKEY_H
#define MONKEY_H

#include "dots.h"
#include "conf.h"

void click(screen_conf_t *conf, int fd, int x, int y);
void gesture(screen_conf_t *conf, int fd, int num_coords, coord_t *coords);

#endif // MONKEY_H
