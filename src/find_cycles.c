#include <stdlib.h>
#include <stdio.h>

#include "dots.h"


static int num_rows, num_cols;


void build_candidate_cycles(vector_t *cycles, int col, SET cycle, int prev_start, int prev_end) {
    if (col == num_cols) {
        vector_append(cycles, cycle);
    } else {
        int start, end, row;
        for (start = 0; start < num_rows - 2; start++) {
            for (end = start + 2; end < num_rows; end++) {
                SET new_cycle = cycle;
                new_cycle = add(new_cycle, POINT(start, col));
                new_cycle = add(new_cycle, POINT(end,   col));

                if (col == 0 || col == num_cols - 1) {
                    for (row = start + 1; row < end; row++) {
                        new_cycle = add(new_cycle, POINT(row, col));
                    }
                } 

                if (col > 0) {
                    for (row = start + 1; row < prev_start + 1; row++) {
                        new_cycle = add(new_cycle, POINT(row, col));
                    }
                    for (row = prev_start + 1; row < start + 1; row++) {
                        new_cycle = add(new_cycle, POINT(row, col - 1));
                    }
                    for (row = prev_end; row < end; row++) {
                        new_cycle = add(new_cycle, POINT(row, col));
                    }
                    for (row = end; row < prev_end; row++) {
                        new_cycle = add(new_cycle, POINT(row, col - 1));
                    }
                }

                build_candidate_cycles(cycles, col + 1, new_cycle, start, end);
            }
        }
    }
}

int is_valid_cycle(SET cycle) {

    /* The number of dots in the cycle should be equal to the
     * number of dots in the perimeter.
     */
    if (cardinality(cycle) != PERIMETER(num_rows, num_cols)) {
        return 0;
    }

    /* Eliminate cycles that contain squares. */
    SET square = 3ULL | (3ULL << NUM_ROWS);
    int row, col;
    for (col = 0; col < num_cols - 1; col++) {
        for (row = 0; row < num_rows - 1; row++) {
            if (MATCHES(square << POINT(row, col), cycle)) {
                return 0;
            }
        }
    }

    /* Check that every point has at least two neighbors. */
    int neighbors;
    for (col = 0; col < num_cols; col++) {
        for (row = 0; row < num_rows; row++) {
            if (element(POINT(row, col), cycle)) {
                neighbors = 0;
                if (col > 0 && element(POINT(row, col - 1), cycle))
                    neighbors++;
                if (col < num_cols - 1 && element(POINT(row, col + 1), cycle))
                    neighbors++;
                if (row > 0 && element(POINT(row - 1, col), cycle))
                    neighbors++;
                if (row < num_rows - 1 && element(POINT(row + 1, col), cycle))
                    neighbors++;
                if (neighbors < 2) {
                    return 0;
                }
            }
        }
    }

    return 1;
}


int main(int argc, char **argv) {
    for (num_rows = 3; num_rows <= 6; num_rows++) {
        for (num_cols = 3; num_cols <= 6; num_cols++) {
            vector_t cycles;
            vector_init(&cycles);

            build_candidate_cycles(&cycles, 0, emptyset, -1, -1);
            int i;
            for (i = 0; i < cycles.length; i++) {
                if (is_valid_cycle(cycles.items[i])) {
                    printf("%d,%d,", num_rows, num_cols);
                    printf("0x%llx\n", cycles.items[i]);
                }
            }

            vector_free(&cycles);
        }
    }
    return 0;
}
