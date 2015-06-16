#ifndef CYCLES_H
#define CYCLES_H

#include "cycle_literals.h"

#define CYCLE_FLAG_INDEX                NUM_DOTS
#define CYCLE_COLOR_OFFSET              (CYCLE_FLAG_INDEX + 1)
#define CYCLE_NUMBER_OFFSET             (CYCLE_COLOR_OFFSET + BITS_PER_COLOR)

#define MAKE_CYCLE(mask, color, num)    (ADD_TO_MASK(mask, CYCLE_FLAG_INDEX) \
                                        | (((mask_t)(color)) << CYCLE_COLOR_OFFSET) \
                                        | (((mask_t)(num)) << CYCLE_NUMBER_OFFSET))

#define HAS_CYCLE(mask)                 MASK_CONTAINS(mask, CYCLE_FLAG_INDEX)
#define GET_CYCLE_COLOR(mask)           (((mask) >> CYCLE_COLOR_OFFSET) & ((1 << BITS_PER_COLOR)- 1))
#define GET_CYCLE_NUMBER(mask)          ((mask) >> CYCLE_NUMBER_OFFSET)

mask_t get_cycles(mask_t, color_t, int*, move_list_t);

#endif
