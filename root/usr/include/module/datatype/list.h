/*  LIST.H -    API for modules to gain access to complex datatypes
 *              Provices: list, round_list
 *              list: Doubly linked list
 *              round_list: List whose tail is its head
 * */

#ifndef MODULE_DT_LIST_H
#define MODULE_DT_LIST_H 1

#include "kernel/types.h"
#include <stddef.h>
#include <stdbool.h>

typedef struct list_node {
    struct list_node *prev;
    struct list_node *next;

    void *data;
} list_node_t;

typedef struct list {
    bool rnd : 1;   //Is round

    list_node_t *head;
    list_node_t *tail;
    size_t elements;
} list_t;

typedef struct dynarray {
    void *array;
    size_t element_size;
} dynarray_t;

/*  Creates a new doubly linked list
 *  r:  Address to new list object
 * */
list_t *new_list(void);

/*  Creates a new round linked list
 *  r:  Address to new list object
 * */
list_t *new_round_list(void);

/*  Pushes new data to the tail of the list
 *  list:   List to append to
 *  data:   Pointer to new data
 *  r:      Index of new list node
 * */
int list_push(list_t *list, void *data);

/*  Adds new data to the list
 *  list:   List to insert to
 *  data:   Pointer to new data
 *  i:      Index to insert data
 *  r:      Index of new list node
 * */
int list_insert(list_t *list, void *data, size_t i);

/*  Gets the data from the list at index i
 *  list:   List to retrieve from
 *  i:  Index to retrieve data
 *  r:  Data retrieved
 * */
void *list_at(list_t *list, size_t i);

/*  Gets the list node at index i
 *  list:   List to retrieve from
 *  i:  Index to retrieve
 *  r:  Node retrieved
 * */
list_node_t *list_get(list_t *list, size_t i);

/*  Searches the list for specific data
 *  list:   List to search
 *  data:   Data to search for
 *  r:      Index in list (first found)
 * */
int list_find(list_t *list, void *data);

#endif
