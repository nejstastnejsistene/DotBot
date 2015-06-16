#include "cycles.h"
#include <stdio.h>

int cycles_limit(mask_t mask) {
    int n = num_dots(mask);
    if (n < 4) {
        return 0;
    }
    if (n < 8) {
        return CYCLES_OFFSET_8;
    }
    if (n < 10) {
        return CYCLES_OFFSET_10;
    }
    if (n < 12) {
        return CYCLES_OFFSET_12;
    }
    if (n < 14) {
        return CYCLES_OFFSET_14;
    }
    if (n < 15) {
        return CYCLES_OFFSET_15;
    }
    if (n < 16) {
        return CYCLES_OFFSET_16;
    }
    if (n < 17) {
        return CYCLES_OFFSET_17;
    }
    if (n < 18) {
        return CYCLES_OFFSET_18;
    }
    if (n < 19) {
        return CYCLES_OFFSET_19;
    }
    if (n < 20) {
        return CYCLES_OFFSET_20;
    }
    return NUM_CYCLES;
}

mask_t get_cycles(mask_t mask, color_t color, int *num_moves, move_list_t moves) {
    mask_t no_cycles = mask;
    int i;
    for (i = 0; i < cycles_limit(mask); i++) {
        if ((mask & cycles[i]) == cycles[i]) {
            moves[(*num_moves)++] = SET_CYCLE(cycles[i], color);
            no_cycles &= ~cycles[i];
        }
    }
    return no_cycles;
}
