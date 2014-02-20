#include <stdint.h>
#include <stdio.h>

#include "set.h"

/* http://playingwithpointers.com/swar.html */
int swar(uint32_t i) {
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0f0f0f0f) * 0x01010101) >> 24;
}

void draw(SET map) {
    int r, c;
    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            if ((map>>(4*c+r)) & 1) {
                printf("* ");
            } else {
                printf("  ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

#define INDEX(r, c)     (4 * (c) + (r))
#define SQUARE(r, c)    (0x0033 << INDEX(r, c))
#define MATCHES(p, x)   (((p) & (x)) == (p))
#define ROW(r)          (0x1111 << (r))
#define COL(c)          (0x000f << (4 * (c)))
#define UP(x, r, c)     ((r) != 0 && ((x) & ROW((r) - 1) & COL(c)))
#define DOWN(x, r, c)   ((r) != 3 && ((x) & ROW((r) + 1) & COL(c)))
#define LEFT(x, r, c)   ((c) != 0 && ((x) & ROW(r) & COL((c) - 1)))
#define RIGHT(x, r, c)  ((c) != 3 && ((x) & ROW(r) & COL((c) + 1)))

SET get_square(SET x) {
    int r, c;
    SET sq;
    for (r = 0; r < 3; r++) {
        for (c = 0; c < 3; c++) {
            sq = SQUARE(r, c);
            if (MATCHES(sq, x)) {
                return sq;
            }
        }
    }
    return 0;
}

int is_valid_cycle(SET x) {
    int i, r, c, neighbors;
    SET row, col;
    for (i = 0; i < 4; i++) {
        if (cardinality(x & ROW(i)) < 2) {
            return 0;
        }
        if (cardinality(x & COL(i)) < 2) {
            return 0;
        }
    }
    for (r = 0; r < 4; r++) {
        row = ROW(r);
        for (c = 0; c < 4; c++) {
            col = COL(c);
            if (x & row & col) {
                neighbors = 0;
                if (UP(x, r, c)) neighbors++;
                if (DOWN(x, r, c)) neighbors++;
                if (LEFT(x, r, c)) neighbors++;
                if (RIGHT(x, r, c)) neighbors++;
                if (neighbors < 2) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

int main() {
    SET x = first_set_of_n_elements(12);
    while (x < 0x10000) {
        if (!get_square(x) && is_valid_cycle(x)) {
            draw(x);
        }
        x = next_set_of_n_elements(x);
    }

    return 0;
}
