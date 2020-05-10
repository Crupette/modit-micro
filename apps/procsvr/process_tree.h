#ifndef PROCESS_TREE_H
#define PROCESS_TREE_H 1

#include <stddef.h>
#include <stdint.h>
#include <sys/proc.h>

#include "process.h"

struct process_tree_node;

typedef struct process_list_node {
    struct process_tree_node *data;
    struct process_list_node *next;
} process_list_node_t;

typedef struct process_list {
    process_list_node_t *head;
    process_list_node_t *tail;
    size_t count;
} process_list_t;

typedef struct process_tree_node {
    process_t *proc;
    process_list_t children;
} process_tree_node_t;

typedef struct process_tree {
    process_tree_node_t *root;
    size_t count;
} process_tree_t;

extern process_tree_t *proc_tree;

/*  Creates a new process tree
 * */
void proc_init();

process_t *proc_find_by_pid(pid_t pid);
process_t *proc_find_by_name(const char *name);

void proc_add(process_t *proc, pid_t pid);

#endif
