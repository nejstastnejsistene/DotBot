#include <stdlib.h>

#include "emu.h"


/* Select a random dot, that is not equal to `exclude`. */
color_t random_dot(color_t exclude) {
    color_t dot = rand_r(&seed) % NUM_COLORS;
    while (dot == exclude) {
        dot = rand_r(&seed) % NUM_COLORS;
    }
    return dot;
}


/* Fill a board with random dots. */
void randomize_board(color_t board[NUM_DOTS]) {
    int i;
    for (i = 0; i < NUM_DOTS; i++) {
        board[i] = random_dot(EMPTY);
    }
}


void fill_empty_dots(color_t board[NUM_DOTS], int exclude) {
    int point;
    for (point = 0; point < NUM_DOTS; point++) {
        if (board[point] == EMPTY) {
            board[point] = random_dot(exclude);
        }
    }
}
