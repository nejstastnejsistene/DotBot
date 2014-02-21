#ifndef DOTBOT_H
#define DOTBOT_H

#include "set.h"

/* Macros for dealing with column major arrays of dots. */
#define NUM_ROWS 6
#define NUM_COLS 6
#define NUM_DOTS (NUM_ROWS * NUM_COLS)
#define ROW(point) ((point) % NUM_COLS)
#define COL(point) ((point) / NUM_COLS)
#define POINT(row, col) (NUM_COLS * (col) + (row))
#define GET_COLUMN(board, col) ((board) + (col) * NUM_ROWS)

#define ABS(x) (((x) < 0) ? -(x): (x))
#define IS_ADJACENT(a, b) \
    (ABS((b)-(a)) == 1 || ABS((b)-(a)) == NUM_ROWS)

/* Constants for dealing with caches of board translations. */
#define NUM_PERMUTATIONS 64 /* 2^6 */
#define COL_MASK 0x3f       /* 0b111111 */

/* An array of dots and their colors. */
typedef int board_t[NUM_DOTS];

/* A single column of a board, useful for */
typedef int column_t[NUM_ROWS];


/* Each color is represented by a number in [0,5), and -1 means empty. */
typedef enum {
    EMPTY = -1,
    RED,
    GREEN,
    YELLOW,
    PURPLE,
    BLUE,
    NUM_COLORS
} color_t;

/* Moves made to the board are cached by computing the effect on each
 * column separately. This way, instead of needing 2^6 slots, we can
 * use only 6*2^6. The score for each column is also saved. The score
 * and resulting board from a move can be easily reconstructed by
 * doing one cache lookup for each rows. This approach also allows parts
 * of a move to be reused without having to recalculate it later.
 */

typedef struct {
    int valid;
    int score;
    column_t translation;
} cache_entry;

typedef cache_entry cache_t[NUM_COLS][NUM_PERMUTATIONS];

typedef struct {
    board_t board;
    int score;
} translation_t;

/* An adjacency matrix for quick lookups. Also includes the a list
 * of the neighbors and the degree of each node.
 */
typedef struct {
   int matrix[NUM_DOTS][NUM_DOTS];
   int neighbors[NUM_DOTS][4];
   int degree[NUM_DOTS];
} adjacency_t;


/* Select a random dot, that is not equal to `exclude`. */
int random_dot(color_t exclude);

/* Fill a board with random dots. */
void randomize_board(board_t board);

/* Compute the bitmask for all dots in `board` of color `color`. */
SET color_mask(board_t board, color_t color);

/* Construct the board resulting from applying `mask` to `board`.
 * The resulting board is placed in `result`, which also includes
 * how many points the move is worth.
 */
void get_translation(board_t board, cache_t cache,
                     SET mask, translation_t *result);

/* Compute the effect of apply a column mask to the board, and cache
 * the resulting column and score.
 */
void compute_translation(board_t board, cache_t cache, int col, int perm);

/* Shrink a dot, and make the dots above it fall into place. */
void shrink_column(int column[NUM_ROWS], int row);

/* Compute all of the partitions connected partitions of a bitmask.
 * The results are place in `partitions` as a sparse array, where all
 * non-zero entries are a bitmask of the partition that includes that
 * position.
 */
void get_partitions(SET mask, SET partitions[NUM_DOTS]);

/* Build a partition starting at point by performing a flood fill. This
 * destructively modifies the mask by removing elements from it as
 * it adds them to the partition.
 */
SET build_partition(SET *mask, int point);

/* ANSI color codes for drawing the dots. */
int color_codes[5] = { 31, 32, 33, 35, 36 };

/* Print colorful UTF8 representations of a board. */
void print_board(int *board);

/* Print a colorful UTF8 representation of a bitmask. */
void print_bitmask(SET mask, int fg, int bg);

#endif // DOTBOT_H
