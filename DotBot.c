#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "DotBot.h"

color_t random_dot(color_t exclude) {
    color_t dot = rand() % 5;
    while (dot == exclude) {
        dot = rand() % 5;
    }
    return dot;
}

int *random_board() {
    int *board = malloc(sizeof(int) * NUM_DOTS);
    if (board == NULL) {
        perror("malloc");
        exit(1);
    }
    int i;
    for (i = 0; i < NUM_DOTS; i++) {
        board[i] = random_dot(EMPTY);
    }
    return board;
}

int *path_mask(path_t *path, int bg, int fg) {
    int *mask = malloc(sizeof(int) * NUM_DOTS);
    if (mask == NULL) {
        perror("malloc");
        exit(1);
    }
    memset(mask, bg, sizeof(mask));
    int i;
    for (i = 0; i < path->length; i++) {
        mask[path->points[i]] = fg;
    }
    return mask;
}

void compute_translation(int *board, cache_t *cache, int col, int perm) {

    /* Mark that this is present in the cache, and reset the score. */
    cache->hits[col][perm] = 1;
    cache->scores[col][perm] = 0;


    /* Copy the row from the board to the cache. */
    int *src = GETCOLUMN(board, col);
    int *dest = cache->translations[col][perm];
    memcpy(dest, src, sizeof(int) * NUM_ROWS);

    /* Shrink dots in the cache. */
    int row;
    for (row = 0; row < NUM_ROWS && perm; row++) {
        if (perm & 1) {
            cache->scores[col][perm]++;
            shrink_column(dest, row);        
        }
        perm >>= 1;
    }
}

int *get_translation(int *board, cache_t *cache, int *perms) {
    int *new_board = malloc(sizeof(int) * NUM_DOTS);
    if (board == NULL) {
        perror("malloc");
        exit(1);
    }
    int col, perm;
    int *dest, *src;
    for (col = 0; col < NUM_COLS; col++) {
        perm = perms[col];

        /* Compute the translation if its not in the cache. */
        if (!cache->hits[col][perm]) {
            compute_translation(board, cache, col, perm);
        }

        /* Copy the translated column from the cache. */
        dest = GETCOLUMN(new_board, col);
        src = cache->translations[col][perm];
        memcpy(dest, src, sizeof(int) * NUM_ROWS);
    }
    return new_board;
}

void shrink(int *board, int point) {
    shrink_column(GETCOLUMN(board, COL(point)), ROW(point));
}

void shrink_column(int *column, int row) {
    while (row > 0) {
        column[row] = column[row - 1];
        row--;
    }
    column[0] = EMPTY;
}

void print_board(int *board) {
    int row, col;
    color_t color;
    for (row = 0; row < NUM_ROWS; row++) {
        for (col = 0; col < NUM_COLS; col++) {
            color = board[POINT(row, col)];
            if (color == -1) {
                printf("  ");
            } else {
                printf(" \033[%dm\xe2\x97\x8f\033[0m", color_codes[color]);
            }
        }
        printf("\n");
    }
    printf("\n");
}

int main() {
    srand(time(NULL));

    int *board = random_board();
    /*
    int points[] = { POINT(0, 0), POINT(0, 1), POINT(0, 2),
                     POINT(0, 3), POINT(0, 4), POINT(0, 5),
                     POINT(1, 5), POINT(2, 5), POINT(3, 5),
                     POINT(4, 5), POINT(5, 5), POINT(5, 4),
                     POINT(5, 3), POINT(4, 3), POINT(3, 3),
                     POINT(3, 2), POINT(3, 1), POINT(3, 0),
                     POINT(2, 0), POINT(1, 0), POINT(0, 0) };
    path_t path;
    path.points = points;
    path.length = 21;
    */

    print_board(board);
    printf("\n");
    //print_board(path_mask(&path, 0, 1));

    cache_t cache;

    int perms[] = {2, 2, 2, 2, 2, 2};
    int *board2 = get_translation(board, &cache, perms);

    print_board(board);
    print_board(board2);

    free(board);
    return 0;
}
