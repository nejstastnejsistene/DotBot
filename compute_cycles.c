#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "dots.h"


/* Find all the dots encircled by a cycle by doing a flood fill from the outside. */
mask_t get_encircled_dots(mask_t mask) {
    mask_t outside = EMPTY_MASK;
    int i, stack_size = 0, stack[NUM_DOTS] = {0};

#define ADD_TO_OUTSIDE(j) { \
    if (!MASK_CONTAINS(outside, j)) { \
        /* Mark the dot as being on the outside. */ \
        outside = ADD_TO_MASK(outside, j); \
        /* Don't flood past the mask, which is the boundary between outside and in. */ \
        if (!MASK_CONTAINS(mask, j)) { \
            stack[stack_size++] = j; \
        } \
    } \
}
 
    /* Add the whole outside border to the stack. */
    for (i = 0; i < NUM_ROWS; i++) {
        ADD_TO_OUTSIDE(MASK_INDEX(i, 0));
        ADD_TO_OUTSIDE(MASK_INDEX(i, NUM_COLS - 1));
    }
    for (i = 1; i < NUM_COLS - 1; i++) {
        ADD_TO_OUTSIDE(MASK_INDEX(0, i));
        ADD_TO_OUTSIDE(MASK_INDEX(NUM_ROWS - 1, i));
    }

    while (stack_size) {
        int row, r, r0, r1, col, c, c0, c1;

        i = stack[--stack_size];

        row = INDEX_ROW(i);
        col = INDEX_COL(i);

        /* Bounds checking */
        r0 = (row == 0) ? row : row - 1;
        r1 = (row == NUM_ROWS - 1) ? row : row + 1;
        c0 = (col == 0) ? col : col - 1;
        c1 = (col == NUM_COLS - 1) ? col : col + 1;

        /* Flood in all directions, including diagonally. */
        for (r = r0; r <= r1; r++) {
            for (c = c0; c <= c1; c++) {
                int j = MASK_INDEX(r, c);
                if (i != j) {
                    ADD_TO_OUTSIDE(j);
                }
            }
        }
    }

    /* Everything that isn't on the outside, is on the inside! */
    return ~outside & ALL_DOTS;
}

int main() {
    int i;

    /* Try each permuation of the center 16 dots that could be encircled by a cycle. */
    for (i = 1; i < (1 << 16); i++) {
        mask_t encircled_dots, mask, inside_dots;
        int j, path_length;
        path_t path;

        /* Move the 16 bits into the center 16 dots of a mask. */
        encircled_dots = EMPTY_MASK;
        encircled_dots |= ((i >>  0) & 15) << 7;
        encircled_dots |= ((i >>  4) & 15) << 13;
        encircled_dots |= ((i >>  8) & 15) << 19;
        encircled_dots |= ((i >> 12) & 15) << 25;

        /* Mask a 3x3 square around each of the encircled dots, and then negate the encircled dots.
         * This leaves behind just the potential cycle.
         */
        mask = EMPTY_MASK;
        for (j = 0; j < NUM_DOTS; j++) {
            if (MASK_CONTAINS(encircled_dots, j)) {
                mask |= (mask_t)7 << (j - 1 - NUM_ROWS);
                mask |= (mask_t)7 << (j - 1); 
                mask |= (mask_t)7 << (j - 1 + NUM_ROWS);
            }
        }
        mask &= ~encircled_dots & ALL_DOTS;

        /* Disregard moves that have extraneous dots on the inside. For example:
         * 
         * X X X X X X           X X X X X X
         * X         X           X         X
         * X         X    and    X         X
         * X         X           X         X
         * X   X X   X           X         X
         * X X X X X X           X X X X X X
         *
         * are equivalent, so disregard the first one.
         */
        inside_dots = get_encircled_dots(mask);
        if (inside_dots != encircled_dots) {
            continue;
        }

        /* Only consider cycles that we can actually find a solution to. The cycles affected
         * by this check are kind of dumb like this:
         *
         * X X X
         * X   X
         * X X X
         *       X X X
         *       X   X
         *       X X X
         *
         * It's a good sanity check nonetheless.
         */
        mask_to_path(SET_CYCLE(mask, EMPTY), &path_length, path);
        if (path_length == 0) {
            continue;
        }

        printf("%" PRId64 " %" PRId64 "\n", mask, mask | inside_dots);
    }

    return 0;
}
