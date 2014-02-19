#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "DotBot.h"


color_t random_dot(color_t exclude) {
    color_t dot = rand() % 5;
    while (dot == exclude) {
        dot = rand() % 5;
    }
    return dot;
}


int *random_board() {
    int *board = malloc(sizeof(int) * NUM_DOTS);
    if (board == NULL) {
        perror("malloc");
        exit(1);
    }
    int i;
    for (i = 0; i < NUM_DOTS; i++) {
        board[i] = random_dot(EMPTY);
    }
    return board;
}


int *path_mask(dots_set_t *path, int bg, int fg) {
    int *mask = malloc(sizeof(int) * NUM_DOTS);
    if (mask == NULL) {
        perror("malloc");
        exit(1);
    }
    memset(mask, bg, sizeof(int) * NUM_DOTS);
    int i;
    for (i = 0; i < path->length; i++) {
        mask[path->points[i]] = fg;
    }
    return mask;
}


void get_translation(int *board, cache_t cache,
                     int *perms, translation_t *result) {
    int *new_board = malloc(sizeof(int) * NUM_DOTS);
    if (new_board == NULL) {
        perror("malloc");
        exit(1);
    }

    result->board = new_board;
    result->score = 0;

    int col, perm;
    int *dest, *src;
    for (col = 0; col < NUM_COLS; col++) {
        perm = perms[col];

        /* Compute the translation if its not in the cache. */
        if (!cache[col][perm].valid) {
            compute_translation(board, cache, col, perm);
        }

        /* Copy the translated column from the cache. */
        dest = GET_COLUMN(new_board, col);
        src = cache[col][perm].translation;
        memcpy(dest, src, sizeof(int) * NUM_ROWS);
        
        /* Update the score. */
        result->score += cache[col][perm].score;
    }
}


void compute_translation(int *board, cache_t cache, int col, int perm) {

    /* Mark that this is present in the cache, and reset the score. */
    cache[col][perm].valid = 1;
    cache[col][perm].score = 0;

    /* Copy the row from the board to the cache. */
    int *src = GET_COLUMN(board, col);
    int *dest = cache[col][perm].translation;
    memcpy(dest, src, sizeof(int) * NUM_ROWS);

    /* Shrink dots in the cache. */
    int row, bit_field = perm;
    for (row = 0; row < NUM_ROWS && perm; row++) {
        if (bit_field & 1) {
            shrink_column(dest, row);        
            cache[col][perm].score++;
        }
        bit_field >>= 1;
    }
}


void shrink(int *board, int point) {
    shrink_column(GET_COLUMN(board, COL(point)), ROW(point));
}


void shrink_column(int *column, int row) {
    while (row > 0) {
        column[row] = column[row - 1];
        row--;
    }
    column[0] = EMPTY;
}


list_t *get_partitions(int *board) {    
    int visited[NUM_DOTS] = {0};
    list_t *partition, *partitions = new_list();
    int point;
    for (point = 0; point < NUM_DOTS; point++) {
        if (board[point] == EMPTY) {
            visited[point] = 1;
        }
    }
    for (point = 0; point < NUM_DOTS; point++) {
        if (!visited[point]) {
            partition = build_partition(board, visited, point);
            if (partition->length > 0) {
                append(partitions, partition);
            }
        }
    }
    return partitions;
}


list_t *build_partition(int *board, int *visited, int point) {
    int stack[NUM_DOTS];
    int stacklen = 0;

    visited[point] = 1;
    stack[stacklen++] = point;

    dot_node_t *partition[NUM_DOTS];
    dot_node_t *node;
    int color = board[point];
    int row, col, length = 0;
    while (stacklen > 0) {
        point = stack[--stacklen];

        node = malloc(sizeof(dot_node_t));
        if (node == NULL) {
            perror("malloc");
            exit(1);
        }
        node->position = point;
        node->num_neighbors = 0;
        partition[length++] = node;

        row = ROW(point);
        col = COL(point);

        point = POINT(row - 1, col);
        if (row > 0 && !visited[point] && board[point] == color) {
            visited[point] = 1;
            stack[stacklen++] = point;
        }
        point = POINT(row + 1, col);
        if (row < 5 && !visited[point] && board[point] == color) {
            visited[point] = 1;
            stack[stacklen++] = point;
        }
        point = POINT(row, col - 1);
        if (col > 0 && !visited[point] && board[point] == color) { 
            visited[point] = 1;
            stack[stacklen++] = point;
        }
        point = POINT(row, col + 1);
        if (col < 5 && !visited[point] && board[point] == color) {
            visited[point] = 1;
            stack[stacklen++] = point;
        }
    }

    /* Connect the nodes. */
    int i, j;
    dot_node_t *a, *b;
    for (i = 0; i < length; i++) {
        for (j = i; j < length; j++) {
            a = partition[i];
            b = partition[j];
            if (IS_ADJACENT(a->position, b->position)) {
                a->neighbors[a->num_neighbors++] = b;
                b->neighbors[b->num_neighbors++] = a;
            }
        }
    }

    list_t *new_partition = new_list();
    for (i = 0; i < length; i++) {
        append(new_partition, partition[i]);
    }
    return new_partition;
}


void print_board(int *board) {
    int row, col;
    color_t color;
    for (row = 0; row < NUM_ROWS; row++) {
        for (col = 0; col < NUM_COLS; col++) {
            color = board[POINT(row, col)];
            if (color == EMPTY) {
                printf("  ");
            } else {
                printf(" \033[%dm\xe2\x97\x8f\033[0m", color_codes[color]);
            }
        }
        printf("\n");
    }
    printf("\n");
}


int main() {
    srand(time(NULL));

    int *board = random_board();
    /*
    int points[] = { POINT(0, 0), POINT(0, 1), POINT(0, 2),
                     POINT(0, 3), POINT(0, 4), POINT(0, 5),
                     POINT(1, 5), POINT(2, 5), POINT(3, 5),
                     POINT(4, 5), POINT(5, 5), POINT(5, 4),
                     POINT(5, 3), POINT(4, 3), POINT(3, 3),
                     POINT(3, 2), POINT(3, 1), POINT(3, 0),
                     POINT(2, 0), POINT(1, 0), POINT(0, 0) };
    dots_set_t path;
    path.length = 21;
    path.points = points;
    */
    int perms[] = { 7, 5, 61, 33, 33, 63 };

    //print_board(board);
    //printf("\n");
    //print_board(path_mask(&path, 0, 1));

    cache_t cache;
    memset(cache, 0, sizeof(cache));

    translation_t result;
    get_translation(board, cache, perms, &result);

    print_board(board);
    print_board(result.board);
    printf("Score: %d\n", result.score);

    list_t *list = get_partitions(board);
    int points[NUM_DOTS];
    dots_set_t part;
    part.points = points;
    int i, j;
    for (i = 0; i < list->length; i++) {
        list_t *partition = list->values[i];
        printf("%d\n", partition->length);
        part.length = partition->length;
        for (j = 0; j < part.length; j++) {
            dot_node_t *node = partition->values[j];
            points[j] = node->position;
        }
        int color = board[part.points[0]];
        print_board(path_mask(&part, EMPTY, color));
        free_list(partition);
    }

    free_list(list);
    free(board);
    free(result.board);
    return 0;
}
