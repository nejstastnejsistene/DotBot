#ifndef LIST_H
#define LIST_H

typedef struct _list_node {
    void *value;
    struct _list_node *next;
} list_node_t;

typedef struct {
    list_node_t *head;
    int length;
} list_t;

list_t *new_list();
void init_list(list_t *list);
void push(list_t *list, void *value);
void *pop(list_t *list);

#endif // LIST_H
