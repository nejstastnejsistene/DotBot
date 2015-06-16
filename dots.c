#include <stdio.h>

#include <string.h>
#include <stdlib.h>

#include <math.h>

#include "dotbot.h"

/* Pretty print a grid using ANSI color codes and unicode dots. */
void pprint_grid(grid_t grid) {
    int row, col;
    color_t color;
    for (row = 0; row < NUM_ROWS; row++) {
        for (col = 0; col < NUM_COLS; col++) {
            color = GET_COLUMN_COLOR(grid[col], row);
            if (color == EMPTY) {
                printf("  ");
            } else {
                /* Unicode 0x25cf BLACK CIRCLE surrounded by ANSI color formatting. */
                printf(" \x1b[%dm\xe2\x97\x8f\x1b[0m", color_codes[color]);
            }
        }
        printf("\n");
    }
    printf("\n");
}

/* Pretty print a mask with a foreground and background color. */
void pprint_mask(mask_t mask, color_t fg, color_t bg) {
    grid_t grid = {0};
    color_t color;
    int col, row;
    /* Convert the mask to a grid first, and print that. */
    for (col = 0; col < NUM_COLS; col++) {
        for (row = 0; row < NUM_ROWS; row++) {
            color = MASK_CONTAINS(mask, MASK_INDEX(row, col)) ? fg : bg;
            grid[col]= SET_COLUMN_COLOR(grid[col], row, color);
        }
    }
    pprint_grid(grid);
}

/* Naively choose the move that will shrink the most dots. */
mask_t naive_choose_move(grid_t grid, int allow_shrinkers) {
    mask_t move = EMPTY_MASK;
    int i, num_dots, max_dots = -1;

    int num_moves;
    move_list_t moves;
    get_moves(grid, allow_shrinkers, &num_moves, moves);

    for (i = 0; i < num_moves; i++) {
        grid_t new_grid;
        memcpy(new_grid, grid, sizeof(grid_t));
        num_dots = apply_move(new_grid, moves[i]);
        if (num_dots > max_dots) {
            max_dots = num_dots;
            move = moves[i];
        }
    }
    return move;
}

/* heuristic = score ** uncertainty_of_board
 *   where uncertainty_of_board = (sum of certainty_of_dots) / num_dots
 *         uncertainty_of_dot | 1/4 if shrunk by cycle
 *                            | 1/5 if shrunk by a regular move
 *                            | 1   otherwise
 */
void _choose_move(grid_t grid, int allow_shrinkers, int turns_remaining, int certainty, float *value, mask_t *move) {
    float best_value = -1;
    mask_t best_move = EMPTY_MASK;

    int i, num_moves;
    move_list_t moves;

    get_moves(grid, allow_shrinkers, &num_moves, moves);

    for (i = 0; i < num_moves; i++) {
        int score;
        float value;
        grid_t new_grid;

        if (turns_remaining > 1 || HAS_CYCLE(moves[i])) {
            memcpy(new_grid, grid, sizeof(grid_t));
            score = apply_move(new_grid, moves[i]);
        } else {
            score = num_dots(moves[i]);
        }

        value = pow(score, certainty / 720.0);

        if (turns_remaining > 1) {
            int next_certainty = certainty - score * (HAS_CYCLE(moves[i]) ? 15 : 16);
            float next_value;
            mask_t next_move;
            _choose_move(new_grid, allow_shrinkers, turns_remaining - 1, next_certainty, &next_value, &next_move);
            value += next_value;
        }

        if (value > best_value) {
            best_value = value;
            best_move = moves[i];
        }
    }

    *value = best_value;
    *move = best_move;
}

#define MAX_DEPTH 4

mask_t choose_move(grid_t grid, int allow_shrinkers, int turns_remaining) {
    float value;
    mask_t move;
    int depth = (turns_remaining > MAX_DEPTH) ? MAX_DEPTH : turns_remaining;
    _choose_move(grid, allow_shrinkers, depth, 720, &value, &move);
    return move;
}

/* Randomly fill a grid with dots. A color can be excluded to simulate the situation
 * where a square was just completed, so none of that color dot will fall into place.
 * To not exclude any color, provide EMPTY.
 */
void fill_grid(grid_t grid, color_t exclude) {
    int col, row;
    color_t color;
    for (col = 0; col < NUM_COLS; col++) {
        for (row = 0; row < NUM_ROWS; row++) {
            if (GET_COLUMN_COLOR(grid[col], row) == EMPTY) {
                while ((color = RED + rand() % (VIOLET - RED + 1)) == exclude);
                grid[col] = SET_COLUMN_COLOR(grid[col], row, color);
            }
        }
    }
}

/* Destructively apply a move to a grid by repeatedly shrink the dots from
 * top to bottom. Returns the number of dots that were shrunk.
 */
int apply_move(grid_t grid, mask_t move) {
    int col, row, shrink, has_cycle = HAS_CYCLE(move), num_dots = 0;
    mask_t encircled;
    color_t cycle_color;
    if (has_cycle) {
        encircled = encircled_dots[GET_CYCLE_NUMBER(move)];
        cycle_color = GET_CYCLE_COLOR(move);
    }
    for (col = 0; col < NUM_COLS; col++) {
        for (row = 0; row < NUM_ROWS; row++) {
            shrink = MASK_CONTAINS(move, MASK_INDEX(row, col));
            if (has_cycle) {
                shrink |= MASK_CONTAINS(encircled, MASK_INDEX(row, col));
                shrink |= GET_COLUMN_COLOR(grid[col], row) == cycle_color;
            }
            if (shrink) {
                grid[col] = SHRINK_COLUMN(grid[col], row);
                num_dots++;
            }
        }
    }
    return num_dots;
}

/* Get all possible moves that can be made on a grid. Comprimises are made to
 * compute the most moves in the smallest amount of time.
 */
void get_moves(grid_t grid, int allow_shrinkers, int *num_moves, move_list_t moves) {
    color_t color;
    mask_t color_mask, cycles, no_cycles;

    *num_moves = 0;

    for (color = RED; color <= VIOLET; color++) {
        color_mask = get_color_mask(grid, color);
        separate_cycles(color_mask, &cycles, &no_cycles);
        if (cycles) {
            get_cycles(cycles, color, num_moves, moves);
        } else {
            get_paths(no_cycles, allow_shrinkers, num_moves, moves);
        }
    }
    /* TODO: figure out how dots does this */
    /*if ((allow_shrinkers && *num_moves == NUM_DOTS) || (!allow_shrinkers && *num_moves == 0)) {
        printf("WARNING/TODO: no moves possible without shrinkers\n");
    }*/
}

/* Create a mask of all of the dots in a grid of a given color. */
mask_t get_color_mask(grid_t grid, color_t color) {
    mask_t mask = EMPTY_MASK;
    int col, row;
    column_t column;
    for (col = 0; col < NUM_COLS; col++) {
        column = grid[col];
        for (row = 0; row < NUM_ROWS; row++) {
            if (GET_COLUMN_COLOR(column, row) == color) {
                mask = ADD_TO_MASK(mask, MASK_INDEX(row, col));
            }
        }
    }
    return mask;
}

/* Split a mask into a mask with just the cycles, and another mask with the remaining dots. */
void separate_cycles(mask_t mask, mask_t *cycles, mask_t *no_cycles) {
    int i, j;

    *cycles = mask;
    *no_cycles = 0;

    for (i = j = 0; i < NUM_DOTS; j = ++i) {
        if (MASK_CONTAINS(*cycles, j)) {
            /* Repeatedly remove dots that only have less than two neighbors and add
             * them to the mask that has no cycles.
             */
            int num_neighbors;
            neighbors_t neighbors;
            get_neighbors(*cycles, j, &num_neighbors, neighbors);
            while (num_neighbors <= 1) {
                *cycles = REMOVE_FROM_MASK(*cycles, j);
                *no_cycles = ADD_TO_MASK(*no_cycles, j);
                if (num_neighbors == 0) {
                    break;
                }
                j = neighbors[0];
                get_neighbors(*cycles, j, &num_neighbors, neighbors);
            }
        }
    }
}

/* Get all possible paths in a color mask without cycles. */
void get_paths(mask_t color_mask, int allow_shrinkers, int *num_moves, move_list_t moves) {
    int i, num_neighbors;
    neighbors_t neighbors;
    path_t path;
    visited_t visited = {{0}};

    /* Repeatedly call build_paths() from each end-point of the color mask */
    for (i = 0; i < NUM_DOTS; i++) {
        if (MASK_CONTAINS(color_mask, i)) {
            get_neighbors(color_mask, i, &num_neighbors, neighbors);
            if (num_neighbors <= 1) {
                build_paths(color_mask, visited, i, allow_shrinkers, num_moves, moves, i, 0, path);
            }
        }
    }
}

/* Get all unique paths through a color mask with no cycles, starting from an end-point
 * (a dot with only one neighbor). This is accomplished by searching for all paths that
 * connect two end-points, and then computing all of its sub-paths. This uses a modified
 * depth-first search that "fast-forwards" through parts of paths with only one way to go.
 */
void build_paths(
        mask_t color_mask,      /* The color mask that paths are being found in. */
        visited_t visited,      /* Matrix of (start,end) pairs representing visited paths. */
        int start_index,        /* Where to start the path (or where it was started, after recursion). */
        int allow_shrinkers,    /* Whether to generate paths of length 1, which requires a power-up in-game. */
        int *num_moves,         /* Number of moves currently stored in moves. */
        move_list_t moves,      /* Array of moves being generated. */
        int current_index,      /* Current end position of the path. */
        int path_length,        /* Current path length. */
        path_t path             /* Array of indices representing the path. */
        ) {

    int num_neighbors, i;
    neighbors_t neighbors;

    /* Follow the path until the end or an intersection is reached, removing
     * dots as we go to avoid moving backwards.
     */
    get_neighbors(color_mask, current_index, &num_neighbors, neighbors);
    color_mask = REMOVE_FROM_MASK(color_mask, current_index);
    path[path_length++] = current_index;
    while (num_neighbors == 1) {
        current_index = neighbors[0];
        get_neighbors(color_mask, current_index, &num_neighbors, neighbors);
        color_mask = REMOVE_FROM_MASK(color_mask, current_index);
        path[path_length++] = current_index;
    }

    /* If there are no neighbors left (i.e. the end of the path has been reached),
     * then yield the resulting path if it hasn't been visited yet.
     */
    if (num_neighbors == 0) {
        if (!visited[start_index][current_index]) {
            get_subpaths(num_moves, moves, visited, allow_shrinkers, path_length, path);
        }
        return;
    }

    /* Recurse on the remaining branches of the intersection. */
    for (i = 0; i < num_neighbors; i++) {
        build_paths(color_mask, visited, start_index, allow_shrinkers, num_moves, moves, neighbors[i], path_length, path);
    }
}

/* Get all unique subpaths of a path with no branches. This adds a total of n(n+1)/2 paths
 * for a path of length n (1 of length n, 2 of length n-1, ..., n of length 1).
 */
void get_subpaths(int *num_moves, move_list_t moves, visited_t visited, int allow_shrinkers, int path_length, path_t path) {
    int start, length;
    int smallest_length = allow_shrinkers ? 1 : 2;
    /* Count backwards through each possible length of the subpaths. */
    for (length = path_length; length >= smallest_length; length--) {
        /* Go through each index a subpath of this length could start. */
        for (start = 0; start < path_length - length + 1; start++) {
            int i = path[start];
            int j = path[start+length-1];
            if (!visited[i][j]) {
                moves[(*num_moves)++] = path_to_mask(path, start, start+length);
                visited[i][j]= visited[j][i] = 1;
            }
        }
    }
}

/* Convert a path represented by a list of indices to a mask. */
mask_t path_to_mask(path_t path, int start, int end) {
    mask_t mask = EMPTY_MASK;
    while (start < end) {
        mask = ADD_TO_MASK(mask, path[start++]);
    }
    return mask;
}

/* Find a path through a mask. Returns the length of the path stored in `path`. The length is 0 if no path was found. */
int _mask_to_path(mask_t mask, mask_t visited, int index, int edges[NUM_DOTS][NUM_DOTS], int path_length, path_t path) {
    int j, num_neighbors, nowhere_to_go;
    neighbors_t neighbors;

    /* All of the branches of this DFS share the same buffer for path, but that's okay because it
     * quits as soon as it finds a path that uses all of the dots.
     */
    path[path_length++] = index;
    visited = ADD_TO_MASK(visited, index);

    nowhere_to_go = 1;
    get_neighbors(mask, index, &num_neighbors, neighbors);
    for (j = 0; j < num_neighbors; j++) {
        int new_index = neighbors[j];

        /* Don't follow the same edge twice. */
        if (!edges[index][new_index]) {
            int final_path_length, new_edges[NUM_DOTS][NUM_DOTS];

            nowhere_to_go = 0;

            /* Make a copy of the edges and mark the edge about to be traversed. */
            memcpy(new_edges, edges, sizeof(new_edges));
            new_edges[index][new_index] = 1;
            new_edges[new_index][index] = 1;

            final_path_length= _mask_to_path(mask, visited, new_index, new_edges, path_length, path);
            if (final_path_length > 0) {
                return final_path_length;
            }
        }
    }

    /* End condition: all dots have been visited and there is nowhere left to go.
     * This works even for cycles where it might visit the same dot twice. */
    if (mask == visited && nowhere_to_go) {
        return path_length;
    }

    /* No path was found. */
    return 0;
}

/* Solve for the sequence of points that goes through all of the points in `mask`. Works with cycles. */
void mask_to_path(mask_t mask, int *path_length, path_t path) {
    int i;
    mask &= ALL_DOTS;

    /* Look for a path starting at each dot, until one is found. */
    for (i = 0; i < NUM_DOTS; i++) {
        if (MASK_CONTAINS(mask, i)) {
            int edges[NUM_DOTS][NUM_DOTS] = {{0}};
            *path_length = _mask_to_path(mask, EMPTY_MASK, i, edges, 0, path);
            if (*path_length > 0) {
                return;
            }
        }
    }
}

/* Count the occupied neighbor dots, i.e. the degree of a node. Directions are in counter-clockwise order. */
void get_neighbors(mask_t mask, int i, int *num_neighbors, neighbors_t neighbors) {
    int row = INDEX_ROW(i);
    int col = INDEX_COL(i);

    *num_neighbors = 0;
    mask &= ALL_DOTS;

    i = MASK_INDEX(row-1, col); /* Up */
    if (row-1 >= 0 && MASK_CONTAINS(mask, i)) {
        neighbors[(*num_neighbors)++] = i;
    }
    i = MASK_INDEX(row, col-1); /* Left */
    if (col-1 >= 0 && MASK_CONTAINS(mask, i)) {
        neighbors[(*num_neighbors)++] = i;
    }
    i = MASK_INDEX(row+1, col); /* DOWN */
    if (row+1 < NUM_ROWS && MASK_CONTAINS(mask, i)) {
        neighbors[(*num_neighbors)++] = i;
    }
    i = MASK_INDEX(row, col+1); /* Right */
    if (col+ 1 < NUM_COLS && MASK_CONTAINS(mask, i)) {
        neighbors[(*num_neighbors)++] = i;
    }
}

int num_dots(mask_t mask) {
    int count = 0;
    mask &= ALL_DOTS;
    while (mask) {
        mask ^= mask & -mask;
        count++;
    }
    return count;
}
