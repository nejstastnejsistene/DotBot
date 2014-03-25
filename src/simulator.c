#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "moves.h"
#include "emu.h"

#define NUM_TURNS 35

int main(int argc, char **argv) {

    if (argc == 3 && strcmp(argv[1], "-s") == 0) {
        srand(atoi(argv[2]));
    }

    int score = 0;

    board_t board;
    randomize_board(&board);

    int turns_remaining = NUM_TURNS;
    do {
        board_init(&board);

        cache_t cache;
        memset(&cache, 0, sizeof(cache));

        SET move = choose_move(&board, cache, turns_remaining);

        translation_t result;
        get_translation(&board, cache, move, &result);
        score += result.score;
        printf("%3d", score);
        printf("%s", turns_remaining > 1 ? ", " : "\n");

        memcpy(&board, &result.board, sizeof(board));

        color_t exclude = EMPTY;
        if (element(CYCLE_FLAG, move)) {
            exclude = move >> COLOR_SHIFT;
        }
        fill_empty(&board, exclude);
    } while (--turns_remaining > 0);

    return 0;
}
