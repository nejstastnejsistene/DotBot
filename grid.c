#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grid.h"

int string_to_grid(const char *s, grid_t grid) {
    int row, col, offset;
    color_t color;
    if (strlen(s) != FMT_SIZE) {
        return -1;
    }
    for (row = 0; row < NUM_ROWS; row++) {
        for (col = 0; col < NUM_COLS; col++) {
            offset = FMT_OFFSET(row, col);
            if (s[offset+1] != FMT_DELIMITER(col)) {
                return -1;
            }
            switch (s[offset]) {
                case 'R':
                    color = RED;
                    break;
                case 'Y':
                    color = YELLOW;
                    break;
                case 'G':
                    color = GREEN;
                    break;
                case 'B':
                    color = BLUE;
                    break;
                case 'V':
                    color = VIOLET;
                    break;
                default:
                    return -1;
            }
            grid[col] = SET_COLUMN_COLOR(grid[col], row, color);
        }

    }
    return 0;
}

char *grid_to_string(grid_t grid) {
    int row, col, offset;
    char *s = malloc(FMT_SIZE + 1);
    if (s == NULL) {
        return s;
    }
    for (row = 0; row < NUM_ROWS; row++) {
        for (col = 0; col < NUM_COLS; col++) {
            offset = FMT_OFFSET(row, col);
            switch (GET_COLUMN_COLOR(grid[col], row)) {
                case RED:
                    s[offset] = 'R';
                    break;
                case YELLOW:
                    s[offset] = 'Y';
                    break;
                case GREEN:
                    s[offset] = 'G';
                    break;
                case BLUE:
                    s[offset] = 'B';
                    break;
                case VIOLET:
                    s[offset] = 'V';
                    break;
                default:
                    s[offset] = ' ';
                    break;
            }
            s[offset + 1] = FMT_DELIMITER(col);
        }
    }
    return s;
}

void pprint_grid(grid_t grid) {
    int row, col;
    color_t color;
    for (row = 0; row < NUM_ROWS; row++) {
        for (col = 0; col < NUM_COLS; col++) {
            color = GET_COLUMN_COLOR(grid[col], row);
            if (color == EMPTY) {
                printf("  ");
            } else {
                /* Unicode 0x24cf BLACK CIRCLE surrounded by ANSI color formatting. */
                printf(" \x1b[%dm\xe2\x97\x8f\x1b[0m", color_codes[color]);
            }
        }
        printf("\n");
    }
    printf("\n");
}
