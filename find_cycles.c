#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "set.h"


static SET THE_ROW = emptyset;
static SET THE_COL = (1ULL << NUM_ROWS) - 1;

#define PERIMETER   (2 * (NUM_ROWS + NUM_COLS - 2))

#define INDEX(r, c)     (NUM_ROWS * (c) + (r))
#define SQUARE(r, c)    ((3ULL | 3ULL << NUM_ROWS) << INDEX(r, c))
#define MATCHES(p, x)   (((p) & (x)) == (p))

#define ROW(r)          (THE_ROW << (r))
#define COL(c)          (THE_COL << (NUM_ROWS * (c)))
#define UP(x, r, c)     ((r) != 0           && ((x) & ROW((r) - 1) & COL(c)))
#define DOWN(x, r, c)   ((r) + 1 < NUM_ROWS && ((x) & ROW((r) + 1) & COL(c)))
#define LEFT(x, r, c)   ((c) != 0           && ((x) & ROW(r) & COL((c) - 1)))
#define RIGHT(x, r, c)  ((c) + 1 < NUM_COLS && ((x) & ROW(r) & COL((c) + 1)))


void draw(SET x) {
    int r, c;
    for (r = 0; r < NUM_ROWS; r++) {
        for (c = 0; c < NUM_COLS; c++) {
            if ((x >> INDEX(r, c)) & 1) {
                printf("* ");
            } else {
                printf("  ");
            }
        }
        printf("\n");
    }
    printf("\n");
}


SET get_square(SET x) {
    int r, c;
    SET sq;
    for (r = 0; r < NUM_ROWS - 1; r++) {
        for (c = 0; c < NUM_COLS - 1; c++) {
            sq = SQUARE(r, c);
            if (MATCHES(sq, x)) {
                return sq;
            }
        }
    }
    return emptyset;
}


int is_valid_cycle(SET x) {
    int r, c, neighbors;
    for (r = 0; r < NUM_ROWS; r++) {
        for (c = 0; c < NUM_COLS; c++) {
            if (x & ROW(r) & COL(c)) {
                neighbors = 0;
                if (UP(x, r, c)) neighbors++;
                if (DOWN(x, r, c)) neighbors++;
                if (LEFT(x, r, c)) neighbors++;
                if (RIGHT(x, r, c)) neighbors++;
                if (neighbors != 2) {
                    return 0;
                }
            }
        }
    }
    return 1;
}


SET to_6x6(SET x) {
    SET y = emptyset;
    int c;
    SET col;
    for (c = 0; c < NUM_COLS; c++) {
        col = (x & COL(c)) >> (c * NUM_ROWS);
        y |= col << (6 * c);
    }
    return y;
}


int count_encircled_dots(SET x) {
    int r, c;
    for (r = 0; r < NUM_ROWS; r++) {
        c = 0;
        while (!element(INDEX(r, c), x)) {
            x = add(x, INDEX(r, c));
            c++;
        }
        c = NUM_COLS - 1;
        while (!element(INDEX(r, c), x)) {
            x = add(x, INDEX(r, c));
            c--;
        }
    }
    for (c = 0; c < NUM_COLS; c++) {
        r = 0;
        while (!element(INDEX(r, c), x)) {
            x = add(x, INDEX(r, c));
            r++;
        }
        r = NUM_ROWS - 1;
        while (!element(INDEX(r, c), x)) {
            x = add(x, INDEX(r, c));
            r--;
        }
    }
    SET mask = first_set_of_n_elements(NUM_ROWS * NUM_COLS);
    return cardinality((~x) & mask);
}


int main() {
    int i;
    for (i = 0; i < NUM_COLS; i++) {
        THE_ROW |= (1ULL << (NUM_ROWS * i));
    }

    SET x, max;
    x = first_set_of_n_elements(PERIMETER);
    x <<= NUM_ROWS * (NUM_COLS - 1) + 2 - PERIMETER;
    max = 1ULL << (NUM_COLS * NUM_ROWS);

    printf("%d\n", PERIMETER);

    while (x < max) {
        if (!get_square(x) && is_valid_cycle(x)) {
            printf("0x%09llxULL %d\n", to_6x6(x), count_encircled_dots(x));
        }
        x = next_set_of_n_elements(x);
    }

    return 0;
}
