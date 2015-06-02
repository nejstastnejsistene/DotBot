#include <stdio.h>

#include <string.h>
#include <stdlib.h>

#include "dots.h"

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
                /* Unicode 0x24cf BLACK CIRCLE surrounded by ANSI color formatting. */
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
mask_t naive_choose_move(grid_t grid) {
    mask_t move = EMPTY_MASK;
    int i, num_dots, max_dots = -1;

    int num_moves;
    move_list_t moves;
    get_moves(grid, &num_moves, moves);

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
    color_t cycle_color;
    if (has_cycle) {
        cycle_color = CYCLE_COLOR(move);
    }
    for (col = 0; col < NUM_COLS; col++) {
        for (row = 0; row < NUM_ROWS; row++) {
            shrink = MASK_CONTAINS(move, MASK_INDEX(row, col));
            if (has_cycle) {
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
void get_moves(grid_t grid, int *num_moves, move_list_t moves) {
    color_t color;
    mask_t color_mask, cycles, no_cycles;

    *num_moves = 0;

    for (color = RED; color <= VIOLET; color++) {
        color_mask = get_color_mask(grid, color);
        separate_cycles(color_mask, &cycles, &no_cycles);
        if (cycles) {
            /*printf("ignoring cycles (for now):\n");
            pprint_mask(cycles, color, EMPTY);*/
            find_square(cycles, color, num_moves, moves);
        }
        /* DFS on the dots without cycles. */
        if (no_cycles) {
            get_paths(no_cycles, num_moves, moves);
        }
    }
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
    int i, num_neighbors, done = 0;
    neighbors_t neighbors;

    *cycles = mask;
    *no_cycles = 0;

    /* Repeatedly removing the dots with only one neighbor until there are none left
     * will result in only the dots that form cycles (makes sense if you think how each
     * dot in a cycle must have at least two neighbors).
     */
    while (!done) {
        done = 1;
        for (i = 0; i < NUM_DOTS; i++) {
            if (MASK_CONTAINS(*cycles, i)) {
                /* Remove dots with one neighbor and add to the no_cycles mask. */
                get_neighbors(*cycles, i, &num_neighbors, neighbors);
                if (num_neighbors <= 1) {
                    *cycles = REMOVE_FROM_MASK(*cycles, i);
                    *no_cycles = ADD_TO_MASK(*no_cycles, i);
                    done = 0;
                }
            }
        }
    }
}

#define SQUARE ((mask_t)(3 | (3 << NUM_ROWS)))

void find_square(mask_t mask, color_t color, int *num_moves, move_list_t moves) {
    int col, row;
    mask_t square;
    for (col = 0; col < NUM_COLS - 1; col++) {
        for (row = 0; row < NUM_ROWS - 1; row++) {
            square = SQUARE << MASK_INDEX(row, col);
            if ((mask & square) == square) {
                moves[(*num_moves)++] = SET_CYCLE(square, color);
            }
        }
    }
}

/* Get all possible paths in a color mask without cycles. */
void get_paths(mask_t color_mask, int *num_moves, move_list_t moves) {
    int i, num_neighbors;
    neighbors_t neighbors;
    path_t path;
    visited_t visited = {{0}};

    /* Repeatedly call build_paths() from each end-point of the color mask */
    for (i = 0; i < NUM_DOTS; i++) {
        if (MASK_CONTAINS(color_mask, i)) {
            get_neighbors(color_mask, i, &num_neighbors, neighbors);
            if (num_neighbors <= 1) {
                build_paths(color_mask, visited, i, num_moves, moves, i, 0, path);
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
            get_subpaths(num_moves, moves, visited, path_length, path);
        }
        return;
    }

    /* Recurse on the remaining branches of the intersection. */
    for (i = 0; i < num_neighbors; i++) {
        build_paths(color_mask, visited, start_index, num_moves, moves, neighbors[i], path_length, path);
    }
}

/* Get all unique subpaths of a path with no branches. This adds a total of n(n+1)/2 paths
 * for a path of length n (1 of length n, 2 of length n-1, ..., n of length 1).
 */
void get_subpaths(int *num_moves, move_list_t moves, visited_t visited, int path_length, path_t path) {
    int start, length;
    /* Count backwards through each possible length of the subpaths. */
    for (length = path_length; length >= 1; length--) {
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

void mask_to_path(mask_t mask, int *path_length, path_t path) {
    int i, start_index = -1, min_neighbors = -1;
    int num_neighbors;
    neighbors_t neighbors;

    int has_cycle = HAS_CYCLE(mask);
    mask = REMOVE_CYCLE_INFO(mask);

    for (i = 0; i < NUM_DOTS; i++) {
        if (MASK_CONTAINS(mask, i)) {
            get_neighbors(mask, i, &num_neighbors, neighbors);
            if (min_neighbors == -1 || num_neighbors < min_neighbors) {
                start_index = i;
                min_neighbors = num_neighbors;
            }
        }
    }

    *path_length = 0;
    i = start_index;
    while (mask) {
        path[(*path_length)++] = i;
        mask = REMOVE_FROM_MASK(mask, i);
        get_neighbors(mask, i, &num_neighbors, neighbors);
        i = neighbors[0];
    }
    if (has_cycle) {
        path[(*path_length)++] = path[0];
    }
}

/* Count the occupied neighbor dots, i.e. the degree of a node. */
void get_neighbors(mask_t mask, int i, int *num_neighbors, neighbors_t neighbors) {
    int row = INDEX_ROW(i);
    int col = INDEX_COL(i);

    *num_neighbors = 0;

    i = MASK_INDEX(row-1, col);
    if (row-1 >= 0 && MASK_CONTAINS(mask, i)) {
        neighbors[(*num_neighbors)++] = i;
    }
    i = MASK_INDEX(row+1, col);
    if (row+1 < NUM_ROWS && MASK_CONTAINS(mask, i)) {
        neighbors[(*num_neighbors)++] = i;
    }
    i = MASK_INDEX(row, col-1);
    if (col-1 >= 0 && MASK_CONTAINS(mask, i)) {
        neighbors[(*num_neighbors)++] = i;
    }
    i = MASK_INDEX(row, col+1);
    if (col-1 < NUM_COLS && MASK_CONTAINS(mask, i)) {
        neighbors[(*num_neighbors)++] = i;
    }
}
