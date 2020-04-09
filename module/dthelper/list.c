#include "module/datatype/list.h"
#include "module/heap.h"

#include "kernel/logging.h"

static inline list_t *_list_allocator(bool r){
    list_t *list = kalloc(sizeof(list_t));
    if(list == 0){
        log_printf(LOG_WARNING, "Failed to allocate list!\n");
        return 0;
    }
    memset(list, 0, sizeof(*list));
    list->rnd = r;
    return list;
}

list_t *new_list(void){
    return _list_allocator(false);
}

list_t *new_round_list(void){
    return _list_allocator(true);
}

int list_push(list_t *list, void *data){
    if(list == 0) {
        log_printf(LOG_WARNING, "Tried to push to NULL list\n");
        return -1;    //List invalid
    }
    list_node_t *node = kalloc(sizeof(list_node_t));
    if(node == 0) {
        log_printf(LOG_WARNING, "Failed to allocate list node\n");
        return -2;    //Node allocation failed
    }
    memset(node, 0, sizeof(list_node_t));

    if(list->elements == 0){
        list->head = list->tail = node;
        if(list->rnd){
            node->next = node->prev = node;
        }
        node->data = data;
        list->elements++;
        return 0;
    }

    node->prev = list->tail;
    list->tail->next = node;
    list->tail = node;

    if(list->rnd){
        node->next = list->head;
    }

    node->data = data;
    return list->elements++;
}

static list_node_t *_seek_node(list_t *list, uint32_t i){
    list_node_t *node = list->head;
    if(i > list->elements && list->rnd) i %= list->elements;
    if(i > list->elements) return 0;

    while(i && node){
        i--;
        node = node->next;
    }
    return node;
}

int list_insert(list_t *list, void *data, size_t i){
    if(list == 0){
        log_printf(LOG_WARNING, "Tried to insert to NULL list\n");
        return -1;
    }
    list_node_t *node = kalloc(sizeof(list_node_t));
    if(node == 0) {
        log_printf(LOG_WARNING, "Failed to allocate list node\n");
        return -2;    //Node allocation failed
    }
    memset(node, 0, sizeof(list_node_t));
    
    list_node_t *dp = _seek_node(list, i);  //Displacing
    if(dp == 0) return list_push(list, data);

    node->next = dp;
    node->prev = dp->prev;
    dp->prev = node;

    if(node->prev) node->prev->next = node;
    if(list->head == dp) list->head = node;

    node->data = data;
    return list->elements++;
}

void *list_at(list_t *list, size_t i){
    if(list == 0){
        log_printf(LOG_WARNING, "Tried to retrieve from NULL list\n");
        return 0;
    }
    list_node_t *node = _seek_node(list, i);
    if(node == 0){
        log_printf(LOG_WARNING, "Node seek for list %p (%i) failed\n", list, i);
        return 0;
    }
    return node->data;
}

list_node_t *list_get(list_t *list, size_t i){
    if(list == 0){
        log_printf(LOG_WARNING, "Tried to retrieve from NULL list\n");
        return 0;
    }
    list_node_t *node = _seek_node(list, i);
    if(node == 0){
        log_printf(LOG_WARNING, "Node seek for list %p (%i) failed\n", list, i);
        return 0;
    }
    return node;
}

int list_find(list_t *list, void *data){
    if(list == 0){
        log_printf(LOG_WARNING, "Tried to retrieve from NULL list\n");
        return 0;
    }
    int i = 0;
    for(list_node_t *node = list->head; node; node = node->next, i++){
        if(node->data == data) return i;
    }
    return -1;
}
