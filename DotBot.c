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
    if (partition.length < 4) {
        printf("No cycles here!;");
        return NULL;
    }

    printf("New length: %d\n", partition.length);

    return NULL;
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
    board[0] = 0;
    board[1] = 0;
    board[2] = 0;
    board[6] = 0;
    board[8] = 0;
    board[12] = 0;
    board[13] = 0;
    board[14] = 0;
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
    list_node_t *node1, *node2;
    int points[NUM_DOTS];
    dots_set_t part;
    part.points = points;
    node1 = list->head;
    while (node1 != NULL) {
        list_t *partition = node1->value;
        if (partition->length >= 4) {
            printf("Old length: %d\n", partition->length);
            find_cycle(partition);
        }
        part.length = partition->length;
        int j = 0;
        node2 = partition->head;
        while (node2 != NULL) {
            dot_node_t *node = node2->value;
            points[j++] = node->position;
            free(node);
            free(node2);
            node2 = node2->next;
        }
        if (partition->length >= 4) {
            int color = board[part.points[0]];
            print_board(path_mask(&part, EMPTY, color));
        }
        free(partition);
        free(node1);
        node1 = node1->next;
    }

    free(list);
    free(board);
    free(result.board);
    return 0;
}
