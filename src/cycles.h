#ifndef CYCLES_H
#define CYCLES_H

#include "dots.h"
#include "vector.h"

int get_cycles(vector_t *moves, SET partition, color_t color, SET color_mask);
void get_convex_hull(SET mask, int *r0, int *c0, int *r1, int *c1);

#endif // CYCLES_H
