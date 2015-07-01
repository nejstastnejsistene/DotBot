#ifndef DOTS_H
#define DOTS_H

#include <stdint.h>

#define NUM_ROWS 6
#define NUM_COLS 6
#define NUM_DOTS (NUM_ROWS * NUM_COLS)

/* Only red through violet are in the game, but empty is useful for ignoring dots
 * that we can't be sure the value of, and white and black are nice for debugging.
 */
#define BITS_PER_COLOR  3
typedef enum {
    EMPTY,
    RED,
    YELLOW,
    GREEN,
    BLUE,
    VIOLET,
    WHITE,
    BLACK,
    NUM_COLORS
} color_t;

/* Column major array. Each column is stored as a bitmask of 6 3-bit colors. */
typedef uint32_t column_t;
typedef column_t grid_t[NUM_COLS];

#define EMPTY_COLUMN            ((column_t) 0)

#define DOT_MASK                ((1 << BITS_PER_COLOR) - 1)
#define COLUMN_OFFSET(row)      ((row) * BITS_PER_COLOR)

/* Shift over, then mask. */
#define GET_COLUMN_COLOR(column, row) \
    (((column) >> COLUMN_OFFSET(row)) & DOT_MASK)

/* Sets the dot to empty, then to the color. */
#define SET_COLUMN_COLOR(column, row, color) \
    (((column) & ~(DOT_MASK << COLUMN_OFFSET(row))) | ((color) << COLUMN_OFFSET(row)))

/* Combines the top dots shifted down one with the bottom dots. */
#define SHRINK_COLUMN(column, row) \
    (((column) & column_masks[row]) << BITS_PER_COLOR) | \
    ((column) & (column_masks[NUM_ROWS-(row)-1] << COLUMN_OFFSET((row)+1)))

/* Column major bitmask for dots. A dot is there when the bit is set, otherwise not.
 * Used for efficient computations on sets of dots.
 */
typedef uint64_t mask_t;

#define EMPTY_MASK                  ((mask_t) 0)
#define SINGLETON_MASK(i)           (((mask_t) 1) << (i))
#define ALL_DOTS                    (SINGLETON_MASK(NUM_DOTS) - 1)
#define MASK_CONTAINS(mask, i)      (!!(SINGLETON_MASK(i) & (mask)))
#define ADD_TO_MASK(mask, i)        ((mask) | SINGLETON_MASK(i))
#define REMOVE_FROM_MASK(mask, i)   ((mask) & ~SINGLETON_MASK(i))

#define INDEX_ROW(i)                ((i) % NUM_COLS)
#define INDEX_COL(i)                ((i) / NUM_COLS)
#define MASK_INDEX(row, col)        (NUM_COLS * (col) + (row)) 

#define MAX_MOVES 5000

/* Masks for 0..6 dots in a column. */
static const column_t column_masks[] = { 0x0, 0x7, 0x3f, 0x1ff, 0xfff, 0x7fff, 0x3ffff };

/* ANSI color codes for each color (except for EMPTY, which is denoted by a lack of formatting). */
static const int color_codes[] = { 0, 31, 33, 32, 36, 35, 37, 30 };


/* A list of moves used when generating possible moves. */
typedef mask_t move_list_t[MAX_MOVES];

/* At most all 36 dots, plus one more to connect it into a loop. */
#define MAX_PATH_LENGTH (NUM_DOTS + 1)

/* A path as a sequence of indices. Unlike masks, this preserves the order of the path. */
typedef int path_t[MAX_PATH_LENGTH];

/* Matrix for constant time checking whether a path has been visited. In graphs without
 * cycles, a path is uniquely by its start and end point. Set visited[start][end] and
 * visited[end][start] to mark a path as visited.
 */
typedef int visited_t[NUM_DOTS][NUM_DOTS];

#define MAX_NEIGHBORS 4

typedef int neighbors_t[MAX_NEIGHBORS];

void pprint_grid(grid_t);

void pprint_mask(mask_t, color_t, color_t);

mask_t naive_choose_move(grid_t, int);
mask_t choose_move(grid_t, int, int);

void fill_grid(grid_t, color_t);

int apply_move(grid_t, mask_t);

void get_moves(grid_t, int, int*, move_list_t);

mask_t get_color_mask(grid_t, color_t);

void separate_cycles(mask_t, mask_t*, mask_t*);

void find_square(mask_t, color_t, int*, move_list_t);

void get_paths(mask_t, int, int*, move_list_t);
void build_paths(mask_t, visited_t, int, int, int*, move_list_t, int, int, path_t);
void get_subpaths(int*, move_list_t, visited_t, int, int, path_t);

mask_t path_to_mask(path_t, int, int);
void mask_to_path(mask_t, int*, path_t);

void get_neighbors(mask_t, int, int*, neighbors_t);

int num_dots(mask_t);

#endif
