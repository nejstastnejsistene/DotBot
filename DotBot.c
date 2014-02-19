#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "DotBot.h"
#include "cycles.h"


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


mask_t bitmask(list_t *dots) {
    mask_t mask = 0;
    list_node_t *node = dots->head;
    dot_node_t *dot;
    while (node != NULL) {
        dot = node->value;
        mask |= MASK(dot->position);
        node = node->next;
    }
    return mask;
}


void get_translation(int *board, cache_t cache,
                     mask_t mask, translation_t *result) {
    int *new_board = malloc(sizeof(int) * NUM_DOTS);
    if (new_board == NULL) {
        perror("malloc");
        exit(1);
    }

    result->board = new_board;
    result->score = 0;

    int col, col_mask;
    int *dest, *src;
    for (col = 0; col < NUM_COLS; col++) {
        col_mask = mask & COL_MASK;

        /* Compute the translation if its not in the cache. */
        if (!cache[col][col_mask].valid) {
            compute_translation(board, cache, col, col_mask);
        }

        /* Copy the translated column from the cache. */
        dest = GET_COLUMN(new_board, col);
        src = cache[col][col_mask].translation;
        memcpy(dest, src, sizeof(int) * NUM_ROWS);
        
        /* Update the score. */
        result->score += cache[col][col_mask].score;

        mask >>= NUM_COLS;
    }
}


void compute_translation(int *board, cache_t cache, int col, int col_mask) {

    /* Mark that this is present in the cache, and reset the score. */
    cache[col][col_mask].valid = 1;
    cache[col][col_mask].score = 0;

    /* Copy the row from the board to the cache. */
    int *src = GET_COLUMN(board, col);
    int *dest = cache[col][col_mask].translation;
    memcpy(dest, src, sizeof(int) * NUM_ROWS);

    /* Shrink dots in the cache. */
    int row, bitfield = col_mask;
    for (row = 0; row < NUM_ROWS && bitfield; row++) {
        if (bitfield & 1) {
            shrink_column(dest, row);        
            cache[col][col_mask].score++;
        }
        bitfield >>= 1;
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
    list_t *partitions = new_list();
    int point;
    for (point = 0; point < NUM_DOTS; point++) {
        if (board[point] == EMPTY) {
            visited[point] = 1;
        }
    }
    for (point = 0; point < NUM_DOTS; point++) {
        if (!visited[point]) {
            push(partitions, build_partition(board, visited, point));
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
        node->color = board[point];
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
        push(new_partition, partition[i]);
    }
    return new_partition;
}


void *find_cycle(list_t *original_partition) {
    if (original_partition->length < 4) {
        return NULL;
    }

    /* Create a copy of the partition so we can modify it.
     * `partition` is a shallow copy so we can remove nodes
     * from the partition, and `neighbors` and `num_neighbors`
     * are copies of the lists of neighbors from the original
     * partition so that we can remove dots from the graph
     * without affecting the original partition.
     */
    list_t partition;
    init_list(&partition);
    dot_node_t *neighbors[NUM_DOTS][4];
    int num_neighbors[NUM_DOTS];

    list_node_t *node = original_partition->head;
    dot_node_t *prev_dot, *dot = NULL;

    /* A lookup table of previous nodes so we can remove nodes
     * in constant time.
     */
    list_node_t *prev_node_lookup[NUM_DOTS] = {NULL};

    int i, j;
    for (i = 0; i < original_partition->length; i++) {
        prev_dot = dot;
        dot = node->value;

        /* Copy the neighbors stuff into the local copy. */
        for (j = 0; j < dot->num_neighbors; j++) {
            neighbors[dot->position][j] = dot->neighbors[j];
        }
        num_neighbors[dot->position] = dot->num_neighbors;

        /* Push each dot onto the local partition list. */
        push(&partition, dot);

        /* Record the node preceding each dot. */
        if (prev_dot != NULL) {
            prev_node_lookup[prev_dot->position] = partition.head;
        }

        node = node->next;
    }

    /* Repeatedly remove all nodes whose degree is less than two.
     * If there are any nodes remaining after this, there is guaranteed
     * to be a cycle in it somewhere.
     */
    int visited[NUM_DOTS] = {0};
    dot_node_t *stack[NUM_DOTS];
    int stacklen = 0;

    /* Add all nodes of degree one to the stack. */
    node = partition.head;
    while (node != NULL) {
        dot = node->value;
        if (num_neighbors[dot->position] < 2) {
            visited[dot->position] = 1;
            stack[stacklen++] = dot;
        }
        node = node->next;
    }

    /* Remove each node in the stack from the partition. Also
     * add the nodes neighbors if removing the node causes the
     * neighbor to have a degree less than two.
     */
    dot_node_t *neighbor;
    int point, k;
    while (partition.length >= 4 && stacklen > 0) {
        dot = stack[--stacklen];
        
        /* Remove this dot from all of all of its neighbors' neighbors. */
        for (i = 0; i < num_neighbors[dot->position]; i++) {
            neighbor = neighbors[dot->position][i];
            point = neighbor->position;
            if (!visited[point]) {
                for (j = 0; j < num_neighbors[point]; j++) {
                    if (neighbors[point][j] == dot) {
                        for (k = j; k < num_neighbors[point]; k++) {
                            neighbors[point][k] = neighbors[point][k + 1];
                        }
                        num_neighbors[point]--;
                        break;
                    }
                }
            }

            /* If this leaves the current dot with less than 2 neighbors,
             * add it to the stack as well. Use `visited` to determine
             * if a dot has been in the stack yet or not.
             */
            if (num_neighbors[point] == 1 && !visited[point]) {
                visited[point] = 1;
                stack[stacklen++] = neighbor;
            }
        }

        /* Remove this dot from the partition. */
        list_node_t *tmp, *prev = prev_node_lookup[dot->position];

        if (prev == NULL) {
            /* The dot is stored in the head of the list. We just 
             * reassign the head.
             */
            tmp = partition.head;
            partition.head = tmp->next;
            free(tmp);
        } else {
            /* Otherwise we have a reference to the node prior to the item
             * we are trying to remove. Simply reconnect the pointers.
             */
            tmp = prev->next;
            prev->next = tmp->next;
            free(tmp);
        }
        partition.length--;
    }

    /* If removing those dots caused the size of the partition to go less
     * than four, there can't be a cycle in there.
     */
    printf("=================\n");
    if (partition.length < 4) {
        printf("No cycles here!\n");
    }

    color_t color = ((dot_node_t*)partition.head->value)->color;

    printf("Old length: %d\n", original_partition->length);
    mask_t mask = bitmask(original_partition);
    print_bitmask(mask, EMPTY, color);

    printf("New length: %d\n", partition.length);
    mask = bitmask(&partition);
    for (i = 0; i < NUM_CYCLES_1; i++) {
        if (HAS_CYCLE(cycles1[i], mask)) {
            printf("Found cycle at cycles1[%d]\n", i);
            print_bitmask(mask, EMPTY, color);
        }
    }

    return NULL;
}


void free_partition(list_t *partition) {
    list_node_t *node = partition->head;
    while (node != NULL) {
        free(node->value);
        node = node->next;
    }
    free(partition);
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


void print_bitmask(mask_t mask, int bg, int fg) {
    int board[NUM_DOTS];
    int i;
    for (i = 0; i < NUM_DOTS; i++, mask >>= 1) {
        board[i] = (mask & 1) ? fg : bg;
    }
    print_board(board);
}

int main() {
    srand(time(NULL));

    int *board = random_board();
    board[0] = 0;
    board[1] = 0;
    board[2] = 0;
    board[6] = 0;
    board[8] = 0;
    board[12] = 0;
    board[13] = 0;
    board[14] = 0;

    print_board(board);

    list_t *partitions = get_partitions(board);
    list_node_t *node = partitions->head;
    while (node != NULL) {
        list_t *partition = node->value;
        find_cycle(partition);
        free_partition(partition);
        node = node->next;
    }

    free(board);
    free(partitions);
    return 0;
}
