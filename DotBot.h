#ifndef DOTBOT_H
#define DOTBOT_H

/* Macros for dealing with column major arrays of dots. */
#define NUM_ROWS 6
#define NUM_COLS 6
#define NUM_DOTS (NUM_ROWS * NUM_COLS)
#define ROW(point) (point % NUM_COLS)
#define COL(point) (point / NUM_COLS)
#define POINT(row, col) (NUM_COLS * col + row)

#define GETCOLUMN(board, col) (board + col * NUM_ROWS)

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
    int *points;
    int length;
} path_t;

#define NUM_PERMUTATIONS 64 /* 2^6 */

typedef struct {
    int hits[NUM_COLS][NUM_PERMUTATIONS];
    int scores[NUM_COLS][NUM_PERMUTATIONS];
    int translations[NUM_COLS][NUM_PERMUTATIONS][NUM_ROWS];
} cache_t;

int random_dot(color_t exclude);
int *random_board();
int *path_mask(path_t *path, int bg, int fg);
void shrink(int *board, int point);
void shrink_column(int *column, int row);

/* ANSI color codes for drawing the dots. */
int color_codes[5] = { 31, 32, 33, 35, 36 };

/* Prints a colorful UTF8 representation of a board. */
void print_board(int *board);

#endif // DOTBOT_H
