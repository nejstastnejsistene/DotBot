#include "dotbot.h"

/* Determine when to stop scanning the list of cycles. The list is sorted based on
 * the number of dots in the cycle, so we can stop searching once the cycles have more
 * dots than the mask does.
 */
int cycles_limit(mask_t mask) {
    int n = num_dots(mask);
    if      (n < 4)  return CYCLES_OFFSET_4;
    else if (n < 8)  return CYCLES_OFFSET_8;
    else if (n < 10) return CYCLES_OFFSET_10;
    else if (n < 12) return CYCLES_OFFSET_12;
    else if (n < 14) return CYCLES_OFFSET_14;
    else if (n < 15) return CYCLES_OFFSET_15;
    else if (n < 16) return CYCLES_OFFSET_16;
    else if (n < 17) return CYCLES_OFFSET_17;
    else if (n < 18) return CYCLES_OFFSET_18;
    else if (n < 19) return CYCLES_OFFSET_19;
    else if (n < 20) return CYCLES_OFFSET_20;
    else             return NUM_CYCLES;
}

/* Find all of the cycles in a mask. */
void get_cycles(mask_t mask, color_t color, int *num_moves, move_list_t moves) {
    int i;
    for (i = 0; i < cycles_limit(mask); i++) {
        if ((mask & cycles[i]) == cycles[i]) {
            moves[(*num_moves)++] = MAKE_CYCLE(cycles[i], color, i);
        }
    }
}
