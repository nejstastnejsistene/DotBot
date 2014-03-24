#include <stdio.h>
#include <string.h>

#include "moves.h"

int main(int argc, char **argv) {
    board_t board;

    int i;
    for (i = 0; i < NUM_DOTS; i++) {
        fscanf(stdin, "%d", board.board + i);
    }
    board_init(&board);

    cache_t cache;
    memset(&cache, 0, sizeof(cache));

    SET move = choose_move(&board, cache, 30);
    printf("%d 0x%llx\n", element(CYCLE_FLAG, move) ? 1 : 0, move);
    return 0;
}
