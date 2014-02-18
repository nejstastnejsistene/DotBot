#ifndef LIST_H
#define LIST_H

#define INITIAL_BUFSIZE 64

typedef struct {
    void **values;
    int bufsize;
    int length;
} list_t;

list_t *new_list();
void free_list(list_t *list);
void append(list_t *list, void *value);
void *pop(list_t *list);

#endif // LIST_H
