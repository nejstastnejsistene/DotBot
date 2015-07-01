#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "dotbot.h"

#define DEFAULT_NUM_GAMES 1000
#define NUM_TURNS 35

int play_game(int num_turns, int allow_shrinkers) {
    int turn, score = 0;

    grid_t grid = {0};
    fill_grid(grid, EMPTY);

    for (turn = num_turns; turn > 0; turn--) {
        int no_moves;
        do {
            mask_t move = choose_move(grid, allow_shrinkers, turn, &no_moves);
            score += apply_move(grid, move);
            fill_grid(grid, GET_CYCLE_COLOR(move));
        } while (no_moves);
    }

    return score;
}

#define MICRO 1000000

long time_usec() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return MICRO * tv.tv_sec + tv.tv_usec;
}

int main(int argc, char **argv) {
    int num_games = (argc > 1) ? atoi(argv[1]) : DEFAULT_NUM_GAMES;
    int num_turns = NUM_TURNS;
    int allow_shrinkers = 0;
    int i, j;

    float mean, std_dev;
    long start_time;
    float duration;

    int *scores = malloc(num_games * sizeof(int));
    if (scores == NULL) {
        perror("malloc");
        return 1;
    }

    j = printf("%d game%s of %d turn%s (%s shrinkers)\n",
            num_games, (num_games == 1) ? " " : "s",
            num_turns, (num_turns == 1) ? " " : "s",
            allow_shrinkers ? "with" : "without");
    for (i = 0; i < j - 1; i++) {
        putchar('=');
    }
    printf("\n\n");

    start_time = time_usec();

    for (i = 0; i < num_games; i++) {
        scores[i] = play_game(num_turns, allow_shrinkers);
    }

    duration = (float)(time_usec() - start_time) / MICRO;
    printf("total time:            %fs\n", duration);
    printf("average time per game: %fs\n", duration / num_games);
    printf("average time per move: %fms\n", duration / num_games / num_turns * 1000);
    printf("\n");

    for (j = i = 1; i < num_games; j = ++i) {
        while (j > 0 && scores[j] < scores[j-1]) {
            int tmp = scores[j];
            scores[j] = scores[j-1];
            scores[j-1] = tmp;
            j--;
        }
    }

    mean = 0;
    for (i = 0; i < num_games; i++) {
        mean += scores[i];
    }
    mean /= num_games;

    std_dev = 0;
    for (i = 0; i < num_games; i++) {
        float tmp = scores[i] - mean;
        std_dev += tmp * tmp;
    }
    std_dev = sqrt(std_dev / num_games);

    printf("mean:    %f\n", mean);
    printf("std dev: %f\n", std_dev);
    if (num_games > 0) {
        printf("minimum: %d\n", scores[0]);
        printf("median:  %d\n", scores[num_games/2]);
        printf("maximum: %d\n", scores[num_games-1]);
    }

    free(scores);
    return 0;
}
