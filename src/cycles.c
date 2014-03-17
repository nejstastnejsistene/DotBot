#include "dots.h"
#include "cycles.h"
#include "litcycles.h"

int get_cycles(vector_t *moves, SET partition, color_t color, SET color_mask) {

    /* Partitions with less than 4 dots can't contain cycles. */
    int num_dots = cardinality(partition);
    if (num_dots < 4) {
        return 0;
    }

    int r0, c0, r1, c1;
    get_convex_hull(partition, &r0, &c0, &r1, &c1);

    /* Partitions must be at least 2x2 to contain cycles. */
    if (r1 - r0 + 1 < 2 || c1 - c0 + 1 < 2) {
        return 0;
    }

    /* Keep track of the unique moves we've made to avoid listing
     * cycles that have an equivalent affect on the board.
     */
    vector_t seen;
    vector_init(&seen);

    int count = 0;
    SET cycle, result;

    /* Iterate through all of the dimensions from 3x3 up to the dimension
     * of the convex hull.
     */
    int rows, cols, i, row, col;
    for (rows = 3; rows < r1 - c0 + 1; rows++) {
        for (cols = 3; cols < c1 - c0 + 1; cols++) {

            /* Skip cyles that we don't have enough dots to form. */
            if (num_dots < PERIMETER(rows, cols)) {
                continue;
            }

            /* Iterate through each cycle for the given dimension. */
            for (i = 0; i < NUM_CYCLES(rows, cols); i++) {

                /* Iterate through all of the positions within the convex
                 * hull that the cycle could fit in.
                 */
                for (row = r0; row < r1 - rows + 2; row++) {
                    for (col = c0; col < c1 - cols + 2; col++) {

                        /* Shift the cycle to (row, col). */
                        cycle = CYCLES(rows, cols)[i] << POINT(row, col);

                        /* If the partition has this cycle, compute the
                         * resulting move, mark that result as seen, and
                         * append the cycle to the list of moves.
                         */
                        if (MATCHES(cycle, partition)) {
                            result = color_mask | get_encircled_dots(cycle);
                            if (!vector_contains(&seen, result)) {
                                vector_append(&seen, result);
                                cycle = ENCODE_CYCLE(cycle, color);
                                vector_append(moves, cycle);
                                count++;
                            }
                        }
                    }
                }
            }
        }
    }

    vector_free(&seen);

    /* Find at most one square. Finding more squares doesn't make
     * a difference because they all have the same effect on the board.
     */
    for (row = r0; row < r1; row++) {
        for (col = c0; col < c1; col++) {
            cycle = SQUARE << POINT(row, col);
            if (MATCHES(cycle, partition)) {
                vector_append(moves, ENCODE_CYCLE(cycle, color));
                count++;
                break;
            }
        }
    }

    return count;
}


void get_convex_hull(SET mask, int *r0, int *c0, int *r1, int *c1) {
    *r0 = NUM_ROWS;
    *c0 = NUM_COLS;
    *r1 = 0;
    *c1 = 0;
    int row, col;
    for (row = 0; row < NUM_ROWS; row++) {
        for (col = 0; col < NUM_COLS; col++) {
            if (element(POINT(row, col), mask)) {
                if (row < *r0) *r0 = row;
                if (row > *r1) *r1 = row;
                if (col < *c0) *c0 = col;
                if (col > *c1) *c1 = col;
            }
        }
    }
}
