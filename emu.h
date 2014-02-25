#ifndef EMU_H
#define EMU_H

#include "dots.h"

static unsigned int seed;

/* Select a random dot, that is not equal to `exclude`. */
color_t random_dot(color_t exclude);

/* Fill a board with random dots. */
void randomize_board(color_t board[NUM_DOTS]);

void fill_empty_dots(color_t board[NUM_DOTS], int exclude);

#endif // EMU_H
