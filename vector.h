#ifndef VECTOR_H
#define VECTOR_H

#include "set.h"

#define INITIAL_VECTOR_SIZE 1024

typedef struct {
    SET *items;
    int bufsize;
    int length;
} vector_t;

void vector_init(vector_t *vector);
void vector_append(vector_t *vector, SET set);
int vector_contains(vector_t *vector, SET set);

#endif // VECTOR_H
