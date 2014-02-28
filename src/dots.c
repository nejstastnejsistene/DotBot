#include <stdio.h>
#include <string.h>

#include "dots.h"


void board_init(board_t *board) {
    memset(&board->adj, 0, sizeof(board->adj));
    color_t color;
    for (color = 0; color < NUM_COLORS; color++) {
        board->color_masks[color] = get_color_mask(board->board, color);
        update_adjacency_matrix(board->color_masks[color], &board->adj);
    }
}


/* Compute the bitmask for all dots in `board` of color `color`. */
SET get_color_mask(color_t board[NUM_DOTS], color_t color) {
    SET mask = emptyset;
    int i;
    for (i = 0; i < NUM_DOTS; i++) {
        if (board[i] == color) {
            mask = add(mask, i); 
        }
    }
    return mask;
}


/* Update an adjacency matrix for a bitmask. */
void update_adjacency_matrix(SET mask, adjacency_t *adj) {
    int i, j;
    for (i = 0; i < NUM_DOTS; i++) {
        if ((mask >> i) & 1) {
            for (j = i; j < NUM_DOTS; j++) {
                if (((mask >> j) & 1) && is_adjacent(i, j)) {
                    adj->neighbors[i][adj->degree[i]++] = j;
                    adj->neighbors[j][adj->degree[j]++] = i;
                } 
            }
        }
    }
}


int is_adjacent(int a, int b) {
    if (b < a) {
        int tmp = a; 
        a = b;
        b = tmp;
    }
    switch (b-a) {
        case 1:
            return COL(a) == COL(b);
        case 6:
            return 1;
        default:
            return 0;
    }
}


/* Compute all of the partitions connected partitions of a bitmask. */
void get_partitions(SET mask, vector_t *partitions) {
    int point;
    for (point = 0; point < NUM_DOTS; point++) {
        if (element(point, mask)) {
            vector_append(partitions, build_partition(&mask, point));
        }
    }
}


/* Build a partition starting at point by performing a flood fill. This
 * destructively modifies the mask by removing elements from it as
 * it adds them to the partition.
 */
SET build_partition(SET *mask, int point) {
    SET partition = emptyset;

    int stack[NUM_DOTS];
    int stacklen = 0;

    /* Push `point` onto the stack. */
    *mask = remove(*mask, point);
    stack[stacklen++] = point;

    int row, col;
    while (stacklen > 0) {
        /* Pop a point from the stack. */
        point = stack[--stacklen];
        partition = add(partition, point);

        row = ROW(point);
        col = COL(point);

        /* Check each point;, and if it is in the mask, pop it from
         * the mask and push it onto the stack.
         */
        point = POINT(row - 1, col);
        if (row > 0 && element(point, *mask)) {
            *mask = remove(*mask, point);
            stack[stacklen++] = point;
        }
        point = POINT(row + 1, col);
        if (row + 1 < NUM_ROWS && element(point, *mask)) {
            *mask = remove(*mask, point);
            stack[stacklen++] = point;
        }
        point = POINT(row, col - 1);
        if (col > 0 && element(point, *mask)) {
            *mask = remove(*mask, point);
            stack[stacklen++] = point;
        }
        point = POINT(row, col + 1);
        if (col + 1 < NUM_COLS && element(point, *mask)) {
            *mask = remove(*mask, point);
            stack[stacklen++] = point;
        }
    }

    return partition;
}


void depth_first_search(
        vector_t *moves,
        int visited[NUM_DOTS][NUM_DOTS],
        int start,
        adjacency_t *adj,
        SET partition,
        SET path,
        int length,
        int point,
        int depth) {
    partition = remove(partition, point);
    path = add(path, point);
    length++;
    if (!visited[start][point] && (length != 1 || depth < 1)) {
        visited[start][point] = 1;
        visited[point][start] = 1;
        vector_append(moves, path);
    }
    int i, neighbor;
    for (i = 0; i < adj->degree[point]; i++) {
        neighbor = adj->neighbors[point][i];
        if (element(neighbor, partition)) {
            depth_first_search(moves, visited,
                    start, adj, partition, path, length, neighbor, depth);
        }
    }
}


/* Construct the board resulting from applying `move` to `board`.
 * The resulting board is placed in `result`, which also includes
 * how many points the move is worth.
 */
void get_translation(
        board_t *board, cache_t cache, SET move, translation_t *result) {

    if (element(CYCLE_FLAG, move)) {
        color_t color = move >> COLOR_SHIFT;
        move = board->color_masks[color] | get_encircled_dots(move);
    }

    result->score = 0;

    int col, col_mask;
    int *dest, *src;
    for (col = 0; col < NUM_COLS; col++, move >>= NUM_COLS) {
        col_mask = move & COL_MASK;

        /* Compute the translation if its not in the cache. */
        if (!cache[col][col_mask].valid) {
            compute_translation(board, cache, col, col_mask);
        }

        /* Copy the translated column from the cache. */
        dest = GET_COLUMN(result->board, col);
        src = cache[col][col_mask].translation;
        memcpy(dest, src, sizeof(int) * NUM_ROWS);
        
        /* Update the score. */
        result->score += cache[col][col_mask].score;
    }
}


/* Compute the effect of apply a column mask to the board, and cache
 * the resulting column and score.
 */
void compute_translation(
        board_t *board, cache_t cache, int col, int col_mask) {

    /* Mark that this is present in the cache, and reset the score. */
    cache[col][col_mask].valid = 1;
    cache[col][col_mask].score = 0;

    /* Copy the row from the board to the cache. */
    int *src = GET_COLUMN(board->board, col);
    int *dest = cache[col][col_mask].translation;
    memcpy(dest, src, sizeof(int) * NUM_ROWS);

    /* Shrink dots in the cache. */
    int row, perm = col_mask;
    for (row = 0; row < NUM_ROWS && col_mask; row++, col_mask >>= 1) {
        if (col_mask & 1) {
            shrink_column(dest, row);        
            cache[col][perm].score++;
        }
    }
}


/* Shrink a dot, and make the dots above it fall into place. */
void shrink_column(column_t column, int row) {
    while (row > 0) {
        column[row] = column[row - 1];
        row--;
    }
    column[0] = EMPTY;
}


int get_encircled_dots(SET x) {
    int r, c;
    for (r = 0; r < NUM_ROWS; r++) {
        c = 0;
        while (!element(POINT(r, c), x)) {
            x = add(x, POINT(r, c));
            c++;
        }
        c = NUM_COLS - 1;
        while (!element(POINT(r, c), x)) {
            x = add(x, POINT(r, c));
            c--;
        }
    }
    for (c = 0; c < NUM_COLS; c++) {
        r = 0;
        while (!element(POINT(r, c), x)) {
            x = add(x, POINT(r, c));
            r++;
        }
        r = NUM_ROWS - 1;
        while (!element(POINT(r, c), x)) {
            x = add(x, POINT(r, c));
            r--;
        }
    }
    return (~x) & first_set_of_n_elements(NUM_ROWS * NUM_COLS);
}


/* ANSI color codes for drawing the dots. */
static int color_codes[5] = { 31, 32, 33, 35, 36 };


/* Print a colorful UTF8 representation of a board. */
void print_board(color_t board[NUM_DOTS]) {
    int row, col;
    color_t color;
    for (row = 0; row < NUM_ROWS; row++) {
        for (col = 0; col < NUM_COLS; col++) {
            color = board[POINT(row, col)];
            if (color == EMPTY) {
                /* Print a space for EMPTY dots. */
                printf("  ");
            } else {
                /* Print unicode "\u24cf" BLACK CIRCLE surrounded
                 * by ANSI color codes for the appropriate color.
                 */
                printf(" \033[%dm\xe2\x97\x8f\033[0m", color_codes[color]);
            }
        }
        printf("\n");
    }
    printf("\n");
}


/* Print a colorful UTF8 representation of a bitmask. */
void print_bitmask(SET mask, int bg, int fg) {
    int board[NUM_DOTS];

    /* Set all points whose bits are set to `fg`, and everything
     * else to `bg`. Then print the board!
     */
    int i;
    for (i = 0; i < NUM_DOTS; i++, mask >>= 1) {
        board[i] = (mask & 1) ? fg : bg;
    }
    print_board(board);
}
