#include "tools/list.h"



void list_init (list_t * list) {
    list->first = list->last = (list_node_t  *)0;
    list->count = 0;
}

void list_insert_first (list_t * list, list_node_t * node) {
    node->next = list->first;
    node->pre = (list_node_t *)0;

    if (list_is_empty(list)) {
        list->last = list->first = node;
    }   else {
        list->first->pre = node;
        list->first = node;
    }

    list->count++;

}

void list_insert_last (list_t * list, list_node_t * node) {
    node->pre = list->last;
    node->next = (list_node_t *)0;

    if (list_is_empty(list)) {
        list->last = list->first = node;
    } else {
        list->last->next = node;
        list->last = node;
    }

    list->count++;

}