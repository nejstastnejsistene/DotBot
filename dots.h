#ifndef DOTS_H
#define DOTS_H

#include <stdint.h>

#include "grid.h"

#define NUM_DOTS                    (NUM_ROWS * NUM_COLS)

/* Column major bitmask for dots. A dot is there when the bit is set, otherwise not.
 * Used for efficient computations on sets of dots.
 */
typedef uint64_t mask_t;

#define EMPTY_MASK                  ((mask_t) 0)
#define SINGLETON_MASK(i)           (((mask_t) 1) << (i))
#define MASK_CONTAINS(mask, i)      (SINGLETON_MASK(i) & (mask))
#define ADD_TO_MASK(mask, i)        ((mask) | SINGLETON_MASK(i))
#define REMOVE_FROM_MASK(mask, i)   ((mask) & ~SINGLETON_MASK(i))

#define INDEX_ROW(i)                ((i) % NUM_COLS)
#define INDEX_COL(i)                ((i) / NUM_COLS)
#define MASK_INDEX(row, col)        (NUM_COLS * (col) + (row)) 

/* The maximum number of moves is 343. Or at least I haven't thought up a scenario
 * where more than that could fit on a single board. My justification is that the most
 * moves are computed from the DFS, which makes n(n+1)/2 moves (this is understandably
 * true for get_subpaths() but also seems to be true for build_paths()... That'd be
 * something nice to prove. The DFS only runs when there aren't any cycles, and the
 * longest paths without cycles I can imagine fitting on a board is two intertwined
 * paths of length 17 and 19. 7*18/2 + 19*20/2 = 343
 */
#define MAX_MOVES 343

/* A list of moves used when generating possible moves. */
typedef mask_t move_list_t[MAX_MOVES];

/* At most all 36 dots, plus one more to connect it into a loop. */
#define MAX_PATH_LENGTH (NUM_DOTS + 1)

/* A path as a sequence of indices. Unlike masks, this preserves the order of the path. */
typedef int path_t[MAX_PATH_LENGTH];

/* Matrix for constant time checking whether a path has been visited. In graphs without
 * cycles, a path is uniquely by its start and end point. Set visited[start][end] and
 * visited[end][start] to mark a path as visited.
 */
typedef int visited_t[NUM_DOTS][NUM_DOTS];

#define MAX_NEIGHBORS 4

typedef int neighbors_t[MAX_NEIGHBORS];


void pprint_mask(mask_t, color_t, color_t);

void apply_mask(grid_t, mask_t);

void get_moves(grid_t, int*, move_list_t);

mask_t get_color_mask(grid_t, color_t);

void separate_cycles(mask_t, mask_t*, mask_t*);

void get_paths(mask_t, int*, move_list_t);
void build_paths(mask_t, visited_t, int, int*, move_list_t, int, int, path_t);
void get_subpaths(int*, move_list_t, visited_t, int, path_t);

mask_t path_to_mask(path_t, int, int);

void get_neighbors(mask_t, int, int*, neighbors_t);

#endif
