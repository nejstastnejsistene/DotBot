#include <stdlib.h>

#include "emu.h"

void randomize_board(board_t *board) {
    int i;
    for (i = 0; i < NUM_DOTS; i++) {
        board->board[i] = rand() % NUM_COLORS;        
    }
}

void fill_empty(board_t *board, color_t exclude) {
    int i;
    for (i = 0; i < NUM_DOTS; i++) {
        if (board->board[i] == EMPTY) {
            do {
                board->board[i] = rand() % NUM_COLORS;
            } while (board->board[i] == exclude);
        }
    }
}
