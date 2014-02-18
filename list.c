#include <stdio.h>
#include <stdlib.h>

#include "list.h"


list_t *new_list() {
    list_t *list = malloc(sizeof(list));
    if (list == NULL) {
        perror("malloc");
        exit(1);
    }
    list->values = malloc(sizeof(void*) * INITIAL_BUFSIZE);
    if (list->values == NULL) {
        perror("malloc");
        exit(1);
    }
    list->bufsize = INITIAL_BUFSIZE;
    list->length = 0;
    return list;
}


void free_list(list_t *list) {
    free(list->values);
    free(list);
}


void append(list_t *list, void *value) {
    if (list->length == list->bufsize) {
        list->bufsize *= 2;
        list->values = realloc(list->values, list->bufsize);
    }
    list->values[list->length++] = value;
}


void *pop(list_t *list) {
    return list->values[--list->length];
}
