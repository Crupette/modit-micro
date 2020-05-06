#ifndef PROCESS_TREE_H
#define PROCESS_TREE_H

#include "process.h"

#include <stdint.h>
#include <stddef.h>

typedef struct process_list_node {
    process_t *proc;
    struct process_list_node *next;
};

typedef struct process_list {

    size_t count;
    process_list_node_t *head;
} process_list_t;

typedef struct process_tree_node {
    process_t *proc;
    process_list_t children;
}

#endif
