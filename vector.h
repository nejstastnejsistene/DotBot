#ifndef VECTOR_H
#define VECTOR_H

#include "set.h"

#define INITIAL_VECTOR_SIZE 1024

typedef struct {
    SET *items;
    int bufsize;
    int length;
} vector_t;

vector_t *vector_new();
void vector_free(vector_t *vector);
void vector_append(vector_t *vector, SET set);

#endif // VECTOR_H
