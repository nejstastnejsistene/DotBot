#ifndef DOTS_H
#define DOTS_H

#include "set.h"
#include "vector.h"

/* Macros for dealing with column major arrays of dots. */
#define NUM_ROWS 6
#define NUM_COLS 6
#define NUM_DOTS (NUM_ROWS * NUM_COLS)
#define ROW(point) ((point) % NUM_COLS)
#define COL(point) ((point) / NUM_COLS)
#define POINT(row, col) (NUM_COLS * (col) + (row))
#define GET_COLUMN(board, col) ((board) + (col) * NUM_ROWS)

/* Constants for dealing with caches of board translations. */
#define NUM_PERMUTATIONS 64 /* 2^6 */
#define COL_MASK 0x3f       /* 0b111111 */

#define MATCHES(p, mask) (((p) & (mask)) == (p))
#define PERIMETER(rows, cols) (2 * ((rows) + (cols)) - 4)
#define CYCLE_FLAG NUM_DOTS
#define COLOR_SHIFT (NUM_DOTS + 1)
#define ENCODE_CYCLE(path, color) \
    (add(path, CYCLE_FLAG) | ((SET)(color) << COLOR_SHIFT))

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

/* An adjacency matrix for quick lookups. Also includes the a list
 * of the neighbors and the degree of each node.
 */
typedef struct {
   int matrix[NUM_DOTS][NUM_DOTS];
   int neighbors[NUM_DOTS][4];
   int degree[NUM_DOTS];
} adjacency_t;

typedef struct {
    color_t board[NUM_DOTS];
    SET color_masks[NUM_COLORS];
    adjacency_t adj;
} board_t;

/* A single column of a board, useful for */
typedef int column_t[NUM_ROWS];

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
    color_t board[NUM_DOTS];
    int score;
} translation_t;

void board_init(board_t *board);

/* Compute the bitmask for all dots in `board` of color `color`. */
SET get_color_mask(color_t board[NUM_DOTS], color_t color);

void update_adjacency_matrix(SET mask, adjacency_t *adj);
int is_adjacent(int a, int b);

/* Compute all of the partitions connected partitions of a bitmask. */
void get_partitions(SET mask, vector_t *partitions);

/* Build a partition starting at point by performing a flood fill. This
* destructively modifies the mask by removing elements from it as
* it adds them to the partition.
*/
SET build_partition(SET *mask, int point);

void depth_first_search(
        vector_t *moves,
        int visited[NUM_DOTS][NUM_DOTS],
        int start,
        adjacency_t *adj,
        SET partition,
        SET path,
        int length,
        int point,
        int depth);

/* Construct the board resulting from applying `mask` to `board`.
 * The resulting board is placed in `result`, which also includes
 * how many points the move is worth.
 */
void get_translation(board_t *board, cache_t cache,
                     SET mask, translation_t *result);

/* Compute the effect of apply a column mask to the board, and cache
 * the resulting column and score.
 */
void compute_translation(board_t *board, cache_t cache, int col, int perm);

/* Shrink a dot, and make the dots above it fall into place. */
void shrink_column(int column[NUM_ROWS], int row);

int get_encircled_dots(SET x);

/* Print colorful UTF8 representations of a board. */
void print_board(int *board);

/* Print a colorful UTF8 representation of a bitmask. */
void print_bitmask(SET mask, int fg, int bg);

#endif // DOTS_H
