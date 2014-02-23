#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

void vector_init(vector_t *vector) {
    vector->items = malloc(sizeof(SET) * INITIAL_VECTOR_SIZE);
    if (vector->items == NULL) {
        perror("vector_init");
        exit(1);
    }
    vector->bufsize = INITIAL_VECTOR_SIZE;
    vector->length = 0;
}

void vector_free(vector_t *vector) {
    free(vector->items);
}

void vector_reset(vector_t *vector) {
    vector->length = 0;
}

void vector_append(vector_t *vector, SET set) {
    if (vector->length == vector->bufsize) {
        vector->bufsize *= 2;
        vector->items = realloc(vector->items, sizeof(SET) * vector->bufsize);
        if (vector->items == NULL) {
            perror("vector_append");
            exit(1);
        }
    }
    vector->items[vector->length++] = set;
}

int vector_contains(vector_t *vector, SET set) {
    int i;
    for (i = 0; i < vector->length; i++) {
        if (vector->items[i] == set) {
            return 1;
        }
    }
    return 0;
}
