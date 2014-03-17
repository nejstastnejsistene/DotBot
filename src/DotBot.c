#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cycles.h"
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

void get_moves(board_t *board, vector_t *moves, int depth) {

    /* A lookup table to prevent duplicate paths. This is based
     * on the assumption that all paths can be uniquely identified
     * by their start and end points.
     */
    int visited[NUM_DOTS][NUM_DOTS] = {{0}};

    vector_t partitions;
    vector_init(&partitions);

    color_t color;
    for (color = 0; color < NUM_COLORS; color++) {
        SET color_mask = board->color_masks[color];
        get_partitions(color_mask, &partitions);

        int i;
        for (i = 0; i < partitions.length; i++)  {
            if (!get_cycles(moves, partitions.items[i], color, color_mask)) {

                /* Perform a DFS on each node with a degree of 1 or less. */
                int point;
                for (point = 0; point < NUM_DOTS; point++) {
                    if (board->adj.degree[point] < 2) {
                        depth_first_search(
                                moves, visited, point, &board->adj, 
                                partitions.items[i], emptyset, 0, point,
                                depth);
                    }
                }
            }
        }
        vector_reset(&partitions);
    }
    vector_free(&partitions);
}

void _choose_move(board_t *board, cache_t cache,
        vector_t *moves, move_t *best, int depth, int moves_remaining, int num_empty);

SET choose_move(board_t *board, cache_t cache, int moves_remaining) {
    vector_t moves;
    vector_init(&moves);
    get_moves(board, &moves, 0);

    move_t result;
    _choose_move(board, cache, &moves, &result, 1, moves_remaining, 0);

    vector_free(&moves);

    return result.path;
}


void _choose_move(board_t *board, cache_t cache,
        vector_t *moves, move_t *best, int depth, int moves_remaining, int num_empty) {

    best->weight = 0;
    best->depth = depth;
    best->path = emptyset;

    translation_t future;
    int j;
    for (j = 0; j < moves->length; j++) {
        get_translation(board, cache, moves->items[j], &future);

        float weight = future.score;
        int deepest = depth;

        if (element(CYCLE_FLAG, moves->items[j])) {
            weight *= CYCLE_WEIGHT;
        }

        if (num_empty < CUTOFF && depth < moves_remaining && depth < MAX_DEPTH) {
            board_t new_board;
            memcpy(&new_board.board,
                    future.board, sizeof(future.board));
            board_init(&new_board);

            cache_t new_cache;
            memset(&new_cache, 0, sizeof(new_cache));

            vector_t new_moves;
            vector_init(&new_moves);
            get_moves(&new_board, &new_moves, depth);

            move_t result;
            _choose_move(&new_board, new_cache, &new_moves, &result,
                    depth + 1, moves_remaining, num_empty + future.score);

            weight += DECAY * result.weight;
            deepest = result.depth;

            vector_free(&new_moves);
        }

        if (depth == 1) {
            weight /= deepest;
        }

        if (weight > best->weight) {
            best->weight = weight;
            best->depth = deepest;
            best->path = moves->items[j];
        }
    }
}


int main(int argc, char **argv) {
        board_t board;

        int i;
        for (i = 0; i < NUM_DOTS; i++) {
            fscanf(stdin, "%d", board.board + i);
        }
        board_init(&board);

        cache_t cache;
        memset(&cache, 0, sizeof(cache));

        SET move = choose_move(&board, cache, 35);
        printf("%d 0x%llx\n", element(CYCLE_FLAG, move) ? 1 : 0, move);
        return 0;
}
