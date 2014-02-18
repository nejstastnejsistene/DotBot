#ifndef DOTBOT_H
#define DOTBOT_H

/* Macros for dealing with column major arrays of dots. */
#define NUM_ROWS 6
#define NUM_COLS 6
#define NUM_DOTS (NUM_ROWS * NUM_COLS)
#define ROW(point) (point % NUM_COLS)
#define COL(point) (point / NUM_COLS)
#define POINT(row, col) (NUM_COLS * col + row)

#define GET_COLUMN(board, col) (board + col * NUM_ROWS)

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
    int length;
    int *points;
} path_t;

typedef struct {
    int valid;
    int score;
    int translation[NUM_ROWS];
} cache_entry;

typedef struct {
    int *board;
    int score;
} translation_t;

#define NUM_PERMUTATIONS 64 /* 2^6 */
typedef cache_entry cache_t[NUM_COLS][NUM_PERMUTATIONS];

int random_dot(color_t exclude);
int *random_board();
int *path_mask(path_t *path, int bg, int fg);
void get_translation(int *board, cache_t cache,
                     int* perms, translation_t *result);
void compute_translation(int *board, cache_t cache, int col, int perm);
void shrink(int *board, int point);
void shrink_column(int *column, int row);

/* ANSI color codes for drawing the dots. */
int color_codes[5] = { 31, 32, 33, 35, 36 };

/* Prints a colorful UTF8 representation of a board. */
void print_board(int *board);

#endif // DOTBOT_H
