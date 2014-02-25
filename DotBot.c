#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "DotBot.h"
#include "cycles.h"


#define MAX_DEPTH 3
#define CUTOFF (NUM_DOTS / 2)
#define DECAY 0.5
#define CYCLE_WEIGHT (1 / DECAY)

static unsigned int seed;

/* Select a random dot, that is not equal to `exclude`. */
color_t random_dot(color_t exclude) {
    color_t dot = rand_r(&seed) % NUM_COLORS;
    while (dot == exclude) {
        dot = rand_r(&seed) % NUM_COLORS;
    }
    return dot;
}


/* Fill a board with random dots. */
void randomize_board(color_t board[NUM_DOTS]) {
    int i;
    for (i = 0; i < NUM_DOTS; i++) {
        board[i] = random_dot(EMPTY);
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
        if (row < NUM_ROWS && element(point, *mask)) {
            *mask = remove(*mask, point);
            stack[stacklen++] = point;
        }
        point = POINT(row, col - 1);
        if (col > 0 && element(point, *mask)) {
            *mask = remove(*mask, point);
            stack[stacklen++] = point;
        }
        point = POINT(row, col + 1);
        if (col < NUM_COLS && element(point, *mask)) {
            *mask = remove(*mask, point);
            stack[stacklen++] = point;
        }
    }

    return partition;
}


void board_init(board_t *board) {
    memset(&board->adj, 0, sizeof(board->adj));
    color_t color;
    for (color = 0; color < NUM_COLORS; color++) {
        board->color_masks[color] = get_color_mask(board->board, color);
        update_adjacency_matrix(board->color_masks[color], &board->adj);
    }
}


/* Update an adjacency matrix for a bitmask. */
void update_adjacency_matrix(SET mask, adjacency_t *adj) {
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


SET choose_move(board_t *board, cache_t cache, int moves_remaining) {
    moves_t moves;
    memset(&moves, 0, sizeof(moves));
    get_moves(board, moves, 0);

    move_t result;
    _choose_move(board, cache, moves, &result, 1, moves_remaining, 0);

    moves_free(moves);

    return result.path;
}


void _choose_move(board_t *board, cache_t cache,
        moves_t moves, move_t *best, int depth, int moves_remaining, int num_empty) {

    best->weight = 0;
    best->depth = depth;
    best->path = emptyset;

    translation_t future;
    int i, j;
    for (i = NUM_DOTS - 1; i >= 0; i--) {
        for (j = 0; j < moves[i].length; j++) {
            get_translation(board, cache, moves[i].items[j], &future);

            float weight = future.score;
            int deepest = depth;

            if (element(CYCLE_FLAG, moves[i].items[j])) {
                weight *= CYCLE_WEIGHT;
            }

            if (num_empty < CUTOFF && depth < moves_remaining && depth < MAX_DEPTH) {
                board_t new_board;
                memcpy(&new_board.board,
                        future.board, sizeof(future.board));
                board_init(&new_board);

                cache_t new_cache;
                memset(&new_cache, 0, sizeof(new_cache));

                moves_t new_moves;
                memset(&new_moves, 0, sizeof(new_moves));
                get_moves(&new_board, new_moves, depth);

                move_t result;
                _choose_move(&new_board, new_cache, new_moves, &result,
                        depth + 1, moves_remaining, num_empty + future.score);

                weight += DECAY * result.weight;
                deepest = result.depth;

                moves_free(new_moves);
            }

            if (depth == 1) {
                weight /= deepest;
            }

            if (weight > best->weight) {
                best->weight = weight;
                best->depth = deepest;
                best->path = moves[i].items[j];
            }
        }
    }
}


void get_moves(board_t *board, moves_t moves, int depth) {

    /* A lookup table to prevent duplicate paths. This is based
     * on the assumption that all paths can be uniquely identified
     * by their start and end points.
     */
    int visited[NUM_DOTS][NUM_DOTS] = {{0}};

    vector_t partitions;
    vector_init(&partitions);

    color_t color;
    for (color = 0; color < NUM_COLORS; color++) {
        SET color_mask = board->color_masks[color];
        get_partitions(color_mask, &partitions);

        int i;
        for (i = 0; i < partitions.length; i++)  {
            if (!get_cycles(moves, partitions.items[i], color, color_mask)) {

                /* Perform a DFS on each node with a degree of 1 or less. */
                int point;
                for (point = 0; point < NUM_DOTS; point++) {
                    if (board->adj.degree[point] < 2) {
                        depth_first_search(
                                moves, visited, point, &board->adj, 
                                partitions.items[i], emptyset, 0, point,
                                depth);
                    }
                }
            }
        }
        vector_reset(&partitions);
    }
    vector_free(&partitions);
}


int get_cycles(moves_t moves, SET partition, color_t color, SET color_mask) {
    int num_dots = cardinality(partition);
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
                    if (MATCHES(cycle, partition)) {
                        result = color_mask | get_encircled_dots(cycle);
                        value = cardinality(result);
                        if (!moves_contains(seen, value, result)) {
                            moves_add(seen, value, result);
                            moves_add(moves, value,
                                    ENCODE_CYCLE(cycle, color));
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
        if (MATCHES(cycle, partition)) {
            value = cardinality(color_mask);
            moves_add(moves, value, ENCODE_CYCLE(cycle, color));
            count++;
            break;
        }
    }
    return count;
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


void depth_first_search(
        moves_t moves,
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
        moves_add(moves, length, path);
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


void fill_empty_dots(color_t board[NUM_DOTS], int exclude) {
    int point;
    for (point = 0; point < NUM_DOTS; point++) {
        if (board[point] == EMPTY) {
            board[point] = random_dot(exclude);
        }
    }
}

//#define DEBUG

int play_round() {
    board_t board;
    cache_t cache;

    printf("Seed: %u\n", seed);

    randomize_board(board.board);
    print_board(board.board);

    int turn, score = 0;
    for (turn = 0; turn < 35; turn++) {
        board_init(&board);
        memset(&cache, 0, sizeof(cache));

        SET move = choose_move(&board, cache, 35 - turn);

        translation_t result;
        get_translation(&board, cache, move, &result);

        score += result.score;

        color_t color = EMPTY;
        if (element(CYCLE_FLAG, move)) {
            color = move >> COLOR_SHIFT;
        }
        fill_empty_dots(result.board, color);

#ifdef DEBUG
        color = EMPTY;
        int i;
        for (i = 0; i < NUM_DOTS; i++) {
            if (element(i, move)) {
                color = board.board[i];
                break;
            }
        }
        print_bitmask(move, EMPTY, color);
        printf("Moves remaining: %d, Score: %d\n", 35 - turn - 1, score);
        printf("Seed: %u\n", seed);
        print_board(result.board);
#endif

        memcpy(board.board, result.board, sizeof(result.board));
    }

    return score;
}


int main(int argc, char **argv) {
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    int score = play_round();
    printf("Final score: %d\n", score);
    return 0;
}
