#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "DotBot.h"
#include "cycles.h"


/* Select a random dot, that is not equal to `exclude`. */
color_t random_dot(color_t exclude) {
    color_t dot = rand() % NUM_COLORS;
    while (dot == exclude) {
        dot = rand() % NUM_COLORS;
    }
    return dot;
}


/* Fill a board with random dots. */
void randomize_board(board_t board) {
    int i;
    for (i = 0; i < NUM_DOTS; i++) {
        board[i] = random_dot(EMPTY);
    }
}

/* Compute the bitmask for all dots in `board` of color `color`. */
SET color_mask(board_t board, color_t color) {
    SET mask = emptyset;
    int i;
    for (i = 0; i < NUM_DOTS; i++) {
        if (board[i] == color) {
            mask = add(mask, i); 
        }
    }
    return mask;
}

/* Construct the board resulting from applying `mask` to `board`.
 * The resulting board is placed in `result`, which also includes
 * how many points the move is worth.
 */
void get_translation(
        board_t board, cache_t cache, SET mask, translation_t *result) {

    result->score = 0;

    int col, col_mask;
    int *dest, *src;
    for (col = 0; col < NUM_COLS; col++, mask >>= NUM_COLS) {
        col_mask = mask & COL_MASK;

        /* Compute the translation if its not in the cache. */
        if (!cache[col][col_mask].valid) {
            compute_translation(board, cache, col, col_mask);
        }

        /* Copy the translated column from the cache. */
        dest = GET_COLUMN(result->board, col);
        src = cache[col][col_mask].translation;
        memcpy(dest, src, sizeof(int) * NUM_ROWS);
        
        /* Update the score. */
        result->score += cache[col][col_mask].score;
    }
}

/* Compute the effect of apply a column mask to the board, and cache
 * the resulting column and score.
 */
void compute_translation(
        board_t board, cache_t cache, int col, int col_mask) {

    /* Mark that this is present in the cache, and reset the score. */
    cache[col][col_mask].valid = 1;
    cache[col][col_mask].score = 0;

    /* Copy the row from the board to the cache. */
    int *src = GET_COLUMN(board, col);
    int *dest = cache[col][col_mask].translation;
    memcpy(dest, src, sizeof(int) * NUM_ROWS);

    /* Shrink dots in the cache. */
    int row, perm = col_mask;
    for (row = 0; row < NUM_ROWS && col_mask; row++, col_mask >>= 1) {
        if (col_mask & 1) {
            shrink_column(dest, row);        
            cache[col][perm].score++;
        }
    }
}


/* Shrink a dot, and make the dots above it fall into place. */
void shrink_column(column_t column, int row) {
    while (row > 0) {
        column[row] = column[row - 1];
        row--;
    }
    column[0] = EMPTY;
}


/* Compute all of the partitions connected partitions of a bitmask.
 * The results are place in `partitions` as a sparse array, where all
 * non-zero entries are a bitmask of the partition that includes that
 * position.
 */
void get_partitions(SET mask, SET partitions[NUM_DOTS]) {    
    int point;
    for (point = 0; point < NUM_DOTS; point++) {
        if (element(point, mask)) {
            partitions[point] = build_partition(&mask, point);
        } else {
            partitions[point] = emptyset;
        }
    }
}


/* Build a partition starting at point by performing a flood fill. This
 * destructively modifies the mask by removing elements from it as
 * it adds them to the partition.
 */
SET build_partition(SET *mask, int point) {
    SET partition = emptyset;

    int stack[NUM_DOTS];
    int stacklen = 0;

    /* Push `point` onto the stack. */
    *mask = remove(point, *mask);
    stack[stacklen++] = point;

    int row, col;
    while (stacklen > 0) {
        /* Pop a point from the stack. */
        point = stack[--stacklen];
        partition = add(partition, point);

        row = ROW(point);
        col = COL(point);

        /* Check each point;, and if it is in the mask, pop it from
         * the mask and push it onto the stack.
         */
        point = POINT(row - 1, col);
        if (row > 0 && element(point, *mask)) {
            *mask = remove(point, *mask);
            stack[stacklen++] = point;
        }
        point = POINT(row + 1, col);
        if (row < NUM_ROWS && element(point, *mask)) {
            *mask = remove(point, *mask);
            stack[stacklen++] = point;
        }
        point = POINT(row, col - 1);
        if (col > 0 && element(point, *mask)) {
            *mask = remove(point, *mask);
            stack[stacklen++] = point;
        }
        point = POINT(row, col + 1);
        if (col < NUM_COLS && element(point, *mask)) {
            *mask = remove(point, *mask);
            stack[stacklen++] = point;
        }
    }

    return partition;
}


SET find_cycle(SET mask) {
    int num_dots = cardinality(mask);
    SET cycle;
    int i, j, k;
    for (i = 0; i < NUM_PERIMETERS; i++) {
        if (num_dots >= perimeters[i]) {
            for (j = 0; j < CYCLES_DIM_2 && cycles[i][j]; j++) {
                for (k = 0; k < CYCLES_DIM_3 && cycles[i][j][k]; k++) {
                    cycle = cycles[i][j][k];
                    if ((cycle & mask) == cycle) {
                        return cycles[i][j][k];
                    }
                }
            }  
        }
    }
    for (i = 0; i < NUM_SQUARES; i++) {
        cycle = SQUARES[i];
        if ((cycle & mask) == cycle) {
            return cycle;
        }
    }
    return emptyset;
}


/* Print a colorful UTF8 representation of a board. */
void print_board(board_t board) {
    int row, col;
    color_t color;
    for (row = 0; row < NUM_ROWS; row++) {
        for (col = 0; col < NUM_COLS; col++) {
            color = board[POINT(row, col)];
            if (color == EMPTY) {
                /* Print a space for EMPTY dots. */
                printf("  ");
            } else {
                /* Print unicode "\u24cf" BLACK CIRCLE surrounded
                 * by ANSI color codes for the appropriate color.
                 */
                printf(" \033[%dm\xe2\x97\x8f\033[0m", color_codes[color]);
            }
        }
        printf("\n");
    }
    printf("\n");
}


/* Print a colorful UTF8 representation of a bitmask. */
void print_bitmask(SET mask, int bg, int fg) {
    int board[NUM_DOTS];

    /* Set all points whose bits are set to `fg`, and everything
     * else to `bg`. Then print the board!
     */
    int i;
    for (i = 0; i < NUM_DOTS; i++, mask >>= 1) {
        board[i] = (mask & 1) ? fg : bg;
    }
    print_board(board);
}

int main() {
    srand(time(NULL));

    int board[NUM_DOTS];
    randomize_board(board);
    print_board(board);

    cache_t cache;
    memset(cache, 0, sizeof(cache));

    translation_t result;

    color_t color;
    for (color = 0; color < NUM_COLORS; color++) {
        SET mask = color_mask(board, color);
        SET partitions[NUM_DOTS];
        get_partitions(mask, partitions);
        int i;
        for (i = 0; i < NUM_DOTS; i++) {
            SET partition = partitions[i];
            if (partition) {
                SET cycle = find_cycle(partition);
                if (cycle) {
                    print_bitmask(cycle, EMPTY, color);
                    get_translation(board, cache, cycle, &result);
                    print_board(result.board);
                }
            }
        }
    }

    return 0;
}
