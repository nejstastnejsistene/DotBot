#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

vector_t *vector_new() {
    vector_t *vector = malloc(sizeof(vector_t));
    SET *items = malloc(sizeof(SET) * INITIAL_VECTOR_SIZE);
    if (vector == NULL || items == NULL) {
        perror("new_vector");
        exit(1);
    }
    vector->items = items;
    vector->bufsize = INITIAL_VECTOR_SIZE;
    vector->length = 0;
    return vector;
}

void vector_free(vector_t *vector) {
    free(vector->items);
    free(vector);
}

void vector_append(vector_t *vector, SET set) {
    if (vector->length == vector->bufsize) {
        vector->bufsize *= 2;
        vector->items = realloc(vector->items, sizeof(SET) * vector->bufsize);
        if (vector->items == NULL) {
            perror("append");
            exit(1);
        }
    }
    vector->items[vector->length++] = set;
}
