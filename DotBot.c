#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "DotBot.h"
#include "cycles.h"


/* Select a random dot, that is not equal to `exclude`. */
color_t random_dot(color_t exclude) {
    color_t dot = rand() % NUM_COLORS;
    while (dot == exclude) {
        dot = rand() % NUM_COLORS;
    }
    return dot;
}


/* Fill a board with random dots. */
void randomize_board(board_t board) {
    int i;
    for (i = 0; i < NUM_DOTS; i++) {
        board[i] = random_dot(EMPTY);
    }
}

/* Compute the bitmask for all dots in `board` of color `color`. */
SET color_mask(board_t board, color_t color) {
    SET mask = emptyset;
    int i;
    for (i = 0; i < NUM_DOTS; i++) {
        if (board[i] == color) {
            mask = add(mask, i); 
        }
    }
    return mask;
}

/* Construct the board resulting from applying `mask` to `board`.
 * The resulting board is placed in `result`, which also includes
 * how many points the move is worth.
 */
void get_translation(
        board_t board, cache_t cache, SET mask, translation_t *result) {

    result->score = 0;

    int col, col_mask;
    int *dest, *src;
    for (col = 0; col < NUM_COLS; col++, mask >>= NUM_COLS) {
        col_mask = mask & COL_MASK;

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
        board_t board, cache_t cache, int col, int col_mask) {

    /* Mark that this is present in the cache, and reset the score. */
    cache[col][col_mask].valid = 1;
    cache[col][col_mask].score = 0;

    /* Copy the row from the board to the cache. */
    int *src = GET_COLUMN(board, col);
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


/* Compute an adjacency matrix for a bitmask. */
void get_adjacency_matrix(SET mask, adjacency_t *adj) {
    int i, j;
    for (i = 0; i < NUM_DOTS; i++) {
        if ((mask >> i) & 1) {
            for (j = i; j < NUM_DOTS; j++) {
                if (((mask >> j) & 1) && is_adjacent(i, j)) {
                    adj->matrix[i][j] = 1;
                    adj->matrix[j][i] = 1;
                    adj->neighbors[i][adj->degree[i]++] = j;
                    adj->neighbors[j][adj->degree[j]++] = i;
                } 
            }
        }
    }
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

void moves_free(moves_t moves) {
    int i;
    for (i = 0; i < 36; i++) {
        if (moves[i].bufsize > 0) {
            free(moves[i].items);
        }
    }
}

void moves_add(moves_t moves, int value, SET move) {
    if (moves[value - 1].bufsize == 0) {
        vector_init(&moves[value - 1]);
    }
    vector_append(&moves[value - 1], move);
}

int moves_contains(moves_t moves, int value, SET set) {
    return vector_contains(&moves[value - 1], set);
}

int find_cycles(moves_t moves, SET mask) {
    int num_dots = cardinality(mask);
    if (num_dots < 4) {
        return 0;
    }
    moves_t seen;
    memset(&seen, 0, sizeof(seen));
    int value, count = 0;
    SET cycle, result;
    int i, j, k;
    for (i = 0; i < NUM_PERIMETERS; i++) {
        if (num_dots >= perimeters[i]) {
            for (j = 0; j < CYCLES_DIM_2 && cycles[i][j]; j++) {
                for (k = 0; k < CYCLES_DIM_3 && cycles[i][j][k]; k++) {
                    cycle = cycles[i][j][k];
                    result = mask | get_encircled_dots(cycle);
                    value = cardinality(mask | get_encircled_dots(result));
                    if ((cycle & mask) == cycle) {
                        if (!moves_contains(seen, value, result)) {
                            moves_add(seen, value, result);
                            moves_add(moves, value, cycle | CYCLE_FLAG);
                            count++;
                        }
                    }
                }
            }  
        }
    }
    moves_free(seen);
    for (i = 0; i < NUM_SQUARES; i++) {
        cycle = SQUARES[i];
        if ((cycle & mask) == cycle) {
            moves_add(moves, num_dots, cycle | CYCLE_FLAG);
            count++;
            break;
        }
    }
    return count;
}


/* Print a colorful UTF8 representation of a board. */
void print_board(board_t board) {
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

/* Print an adjacency matrix as an ascii grid, with the numeric
 * values of a nodes neighbors listed to the right of each row.
 */
void print_adjacency_matrix(adjacency_t *adj) {
    int i, j, k;
    for (i = 0; i < NUM_DOTS; i++) {
        printf("(%d, %d) ", ROW(i), COL(i));
        for (j = 0; j < NUM_DOTS; j++) {
            if (adj->matrix[i][j]) {
                printf("@ ");
            } else {
                printf(". ");
            }
        }
        for (k = 0; k < adj->degree[i]; k++) {
            int neighbor = adj->neighbors[i][k];
            printf("(%d, %d) ", ROW(neighbor), COL(neighbor));
        }
        printf("\n");
    }
    printf("\n");
}


void depth_first_search(
        moves_t moves,
        int visited[NUM_DOTS][NUM_DOTS],
        int start,
        adjacency_t *adj,
        SET partition,
        SET path,
        int length,
        int point) {
    partition = remove(partition, point);
    path = add(path, point);
    length++;
    if (!visited[start][point]) {
        visited[start][point] = 1;
        visited[point][start] = 1;
        moves_add(moves, length, path);
    }
    int i, neighbor;
    for (i = 0; i < adj->degree[point]; i++) {
        neighbor = adj->neighbors[point][i];
        if (element(neighbor, partition)) {
            depth_first_search(moves, visited, start, adj, partition, path, length, neighbor);
        }
    }
}


void get_moves(board_t board, moves_t moves) {

    /* A lookup table to prevent duplicate paths. This is based
     * on the assumption that all paths can be uniquely identified
     * by their start and end points.
     */
    int visited[NUM_DOTS][NUM_DOTS] = {{0}};

    adjacency_t adj;
    memset(&adj, 0, sizeof(adj));
    color_t color;
    for (color = 0; color < NUM_COLORS; color++) {
        SET mask = color_mask(board, color);
        get_adjacency_matrix(mask, &adj);

        if (!find_cycles(moves, mask)) {

            /* Perform a DFS on each node with a degree of 1 or less. */
            int point;
            for (point = 0; point < NUM_DOTS; point++) {
                if (adj.degree[point] < 2) {
                    depth_first_search(moves, visited, point, &adj, mask, emptyset, 0, point);
                }
            }
        }
    }
}


int main() {
    srand(time(NULL));

    int board[NUM_DOTS] = {RED};
    randomize_board(board);
    print_board(board);

    moves_t moves;
    memset(&moves, 0, sizeof(moves));
    get_moves(board, moves);

    int i, j, count = 0;
    for (i = 0; i < NUM_DOTS; i++) {
        if (moves[i].length > 0) {
            printf("Score: %d\n", i + 1);
            for (j = 0; j < moves[i].length; j++, count++) {
                print_bitmask(moves[i].items[j], GREEN, RED);
            }
        }
    }

    moves_free(moves);

    printf("Total moves: %d\n", count);

    return 0;
}
