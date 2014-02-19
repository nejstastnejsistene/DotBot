#include <stdio.h>
#include <stdlib.h>

#include "list.h"


list_t *new_list() {
    list_t *list = malloc(sizeof(list_t));
    if (list == NULL) {
        perror("malloc");
        exit(1);
    }
    init_list(list);
    return list;
}

void init_list(list_t *list) {
    list->head = NULL;
    list->length = 0;
}


void push(list_t *list, void *value) {
    list_node_t *node = malloc(sizeof(list_node_t));
    if (node == NULL) {
        perror("malloc");
        exit(1);
    }
    node->value = value;
    node->next = list->head;
    list->head = node;
    list->length++;
}


void *pop(list_t *list) {
    void *value= list->head->value;
    list->head = list->head->next;
    return value;
}
