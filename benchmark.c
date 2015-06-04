#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dots.h"

#define DEFAULT_NUM_GAMES 1000
#define NUM_TURNS 35

int play_game(int num_turns) {
    int turn, score = 0;

    grid_t grid = {0};
    fill_grid(grid, EMPTY);

    for (turn = num_turns; turn > 0; turn--) {
        mask_t move = naive_choose_move(grid);
        score += apply_move(grid, move);
        fill_grid(grid, HAS_CYCLE(move) ? CYCLE_COLOR(move) : EMPTY);
    }

    return score;
}

int main(int argc, char **argv) {
    int i, total_score = 0;
    int num_games = (argc > 1) ? atoi(argv[1]) : DEFAULT_NUM_GAMES;
    for (i = 0; i < num_games; i++) {
        total_score += play_game(NUM_TURNS);
    }
    printf("average score over %d games: %f\n", num_games, (float)total_score / num_games);
    return 0;
}
