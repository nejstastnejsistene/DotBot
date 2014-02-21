#ifndef DOTBOT_H
#define DOTBOT_H

#include "list.h"
#include "set.h"

/* Macros for dealing with column major arrays of dots. */
#define NUM_ROWS 6
#define NUM_COLS 6
#define NUM_DOTS (NUM_ROWS * NUM_COLS)
#define ROW(point) ((point) % NUM_COLS)
#define COL(point) ((point) / NUM_COLS)
#define POINT(row, col) (NUM_COLS * (col) + (row))

#define GET_COLUMN(board, col) ((board) + (col) * NUM_ROWS)

#define ABS(x) (((x) < 0) ? -(x) : (x))
#define IS_ADJACENT(a, b) \
    (ABS(ROW(b) - ROW(a)) + ABS(COL(b) - COL(a)) == 1)

typedef int column_t[NUM_ROWS];
typedef int board_t[NUM_DOTS];

typedef enum {
    EMPTY = -1,
    RED,
    GREEN,
    YELLOW,
    PURPLE,
    BLUE,
    NUM_COLORS
} color_t;

typedef struct {
    int valid;
    int score;
    column_t translation;
} cache_entry;

typedef struct {
    board_t board;
    int score;
} translation_t;

/*
struct dot_node {
    int position;
    color_t color;
    struct dot_node *neighbors[4];
    int num_neighbors;
};
typedef struct dot_node dot_node_t;
*/

#define NUM_PERMUTATIONS 64 /* 2^6 */
#define COL_MASK 0x3f       /* 0b111111 */
typedef cache_entry cache_t[NUM_COLS][NUM_PERMUTATIONS];

int random_dot(color_t exclude);
void randomize_board(board_t board);

void get_translation(board_t board, cache_t cache,
                     SET mask, translation_t *result);
void compute_translation(board_t board, cache_t cache, int col, int perm);
void shrink(board_t board, int point);
void shrink_column(int column[NUM_ROWS], int row);
void get_partitions(SET mask, SET partitions[NUM_DOTS]);
SET build_partition(SET *mask, int point);

/* ANSI color codes for drawing the dots. */
int color_codes[5] = { 31, 32, 33, 35, 36 };

/* Prints a colorful UTF8 representation of a board. */
void print_board(int *board);
void print_bitmask(SET mask, int fg, int bg);


#endif // DOTBOT_H
