#include "dots.h"
#include "cycles.h"


int get_cycles(vector_t *moves, SET partition, color_t color, SET color_mask) {
    int num_dots = cardinality(partition);
    if (num_dots < 4) {
        return 0;
    }
    vector_t seen;
    vector_init(&seen);
    int count = 0;
    SET cycle, result;

    int r0, c0, r1, c1;
    get_convex_hull(partition, &r0, &c0, &r1, &c1);

    int rows, cols, i, row, col;
    for (rows = 3; rows < r1 - c0 + 1; rows++) {
        for (cols = 3; cols < c1 - c0 + 1; cols++) {
            for (i = 0; i < NUM_CYCLES(rows, cols); i++) {
                for (row = r0; row < r1 - rows + 2; row++) {
                    for (col = c0; col < c1 - cols + 2; col++) {
                        cycle = CYCLES(rows, cols)[i] << POINT(row, col);
                        if (MATCHES(cycle, partition)) {
                            result = color_mask | get_encircled_dots(cycle);
                            if (!vector_contains(&seen, result)) {
                                vector_append(&seen, result);
                                vector_append(moves, ENCODE_CYCLE(cycle, color));
                                count++;
                            }
                        }
                    }
                }
            }
        }
    }

    vector_free(&seen);

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
