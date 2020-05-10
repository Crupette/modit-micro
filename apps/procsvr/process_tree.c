#include "process_tree.h"

#include <stdlib.h>
#include <string.h>

process_tree_t *proc_tree;

void proc_init(){
    proc_tree = malloc(sizeof(process_tree_t));
    proc_tree->count = 0;
    proc_tree->root = malloc(sizeof(process_tree_node_t));

    proc_tree->root->proc = malloc(sizeof(process_t));
    proc_tree->root->children.head = 
        proc_tree->root->children.tail = NULL;
    proc_tree->root->children.count = 0;

    proc_tree->root->proc->pid = 0;
    memcpy(proc_tree->root->proc->name, "init", 5);
}

static void proc_list_append(process_list_t *list, process_tree_node_t *data){
   list->count++;

   process_list_node_t *node = malloc(sizeof(process_list_node_t));
   node->data = data;
   node->next = NULL;

   if(list->head == NULL){
       list->head = list->tail = node;
       return;
   }

   list->tail->next = node;
   list->tail = node;
}

static process_tree_node_t *traverse_by_pid(process_tree_node_t *start, pid_t pid){
    if(start == NULL) return start;
    if(start->proc->pid == pid) return start;
    for(process_list_node_t *node = start->children.head;
            node; node = node->next){
        if(node->data->proc->pid == pid) return node->data;
        process_tree_node_t *trv = traverse_by_pid(node->data, pid);
        if(trv == NULL) continue;
        return trv;
    }
    return NULL;
}

static process_tree_node_t *traverse_by_name(process_tree_node_t *start, const char *name){
    if(start == NULL) return start;
    if(strcmp(start->proc->name, name) == 0) return start;
    for(process_list_node_t *node = start->children.head;
            node; node = node->next){
        if(strcmp(node->data->proc->name, name) == 0) return node->data;
        process_tree_node_t *trv = traverse_by_name(node->data, name);
        if(trv == NULL) continue;
        return trv;
    }
    return NULL;
}

process_t *proc_find_by_pid(pid_t pid){
    process_tree_node_t *found = traverse_by_pid(proc_tree->root, pid);
    if(found == NULL) return NULL;

    return found->proc;
}

process_t *proc_find_by_name(const char *name){
    process_tree_node_t *found = traverse_by_name(proc_tree->root, name);
    if(found == NULL) return NULL;
    return found->proc;
}

void proc_add(process_t *proc, pid_t parent){
    process_tree_node_t *found = traverse_by_pid(proc_tree->root, parent);
    if(!found){
        if(parent == 0){
            printf("Missing init? dropping\n");
            return;
        }
        printf("Failed to add process %s to proper parent (%i) : appending to init\n",
                proc->name, parent);
        proc_add(proc, 0);
        return;
    }

    process_tree_node_t *newnode = malloc(sizeof(process_tree_node_t));
    memset(newnode, 0, sizeof(*newnode));

    newnode->proc = proc;
    proc_list_append(&found->children, newnode);
}
