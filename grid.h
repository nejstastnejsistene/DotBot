#ifndef GRID_H
#define GRID_H

#include <stdint.h>

#define NUM_ROWS 6
#define NUM_COLS 6

/* Only red through violet are in the game, but empty is useful for ignoring dots
 * that we can't be sure the value of, and white and black are nice for debugging.
 */
typedef enum {
    EMPTY,
    RED,
    YELLOW,
    GREEN,
    BLUE,
    VIOLET,
    WHITE,
    BLACK
} color_t;

#define BITS_PER_COLOR  3

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

/* Convenience macros for consistently formatting to/from a string. */
#define FMT_ROW_SIZE            (2 * NUM_COLS)
#define FMT_SIZE                (NUM_ROWS * FMT_ROW_SIZE)
#define FMT_OFFSET(row, col)    ((row) * FMT_ROW_SIZE + (col) * 2)
#define FMT_DELIMITER(col)      (((col) == NUM_COLS - 1) ? '\n' : ' ')

/* Masks for 0..6 dots in a column. */
static const column_t column_masks[] = { 0x0, 0x7, 0x3f, 0x1ff, 0xfff, 0x7fff, 0x3ffff };

/* ANSI color codes for each color (except for EMPTY, which is denoted by a lack of formatting). */
static const int color_codes[] = { 0, 31, 33, 32, 36, 35, 37, 30 };

int string_to_grid(const char*, grid_t);
char *grid_to_string(grid_t);

void pprint_grid(grid_t);

#endif
