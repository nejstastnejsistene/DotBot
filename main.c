#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dots.h"

const char *s = "B G G G G G\n" \
                "B G B B B G\n" \
                "B G B G B G\n" \
                "B G B G B G\n" \
                "B G B G B G\n" \
                "B B B G G G\n";

int main() {
    grid_t grid, new_grid;
    char *s2;
    int err, j, i, num_moves, row, col;
    mask_t moves[MAX_MOVES];

    err = string_to_grid(s, grid);
    if (err != 0) {
        return err;
    }
    pprint_grid(grid);

    s2 = grid_to_string(grid);
    printf("%s\n%s\n", s, s2);

    if (strcmp(s, s2) != 0) {
        return 1;
    }

    for (j = 0; j < 1000000; j++) {
        for (col = 0; col < NUM_COLS; col++) {
            for (row = 0; row < NUM_ROWS; row++) {
                grid[col] = SET_COLUMN_COLOR(grid[col], row, (rand()%5)+1);
            }
        }
        get_moves(grid, &num_moves, moves);

        for (i = 0; i < num_moves; i++) {
            memcpy(new_grid, grid, sizeof(new_grid));
            apply_mask(new_grid, moves[i]);
        }
    }

    return 0;
}
