#ifndef MOVES_H
#define MOVES_H

#include "dots.h"

#define MAX_DEPTH 3
#define CUTOFF (NUM_DOTS / 2)
#define DECAY 0.5
#define CYCLE_WEIGHT (1 / DECAY)

typedef struct {
    float weight;
    int depth;
    SET path;
} move_t;

void get_moves(board_t *board, vector_t *moves, int depth);
SET choose_move(board_t *board, cache_t cache, int moves_remaining);
void _choose_move(board_t *board, cache_t cache,
        vector_t *moves, move_t *best, int depth, int moves_remaining, int num_empty);

#endif // MOVES_H
