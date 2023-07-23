#include "ipc/mutex.h"



void mutex_init (mutex_t mutex) {
    mutex->locked_count = 0;
    mutex->owner = (task_t *)0;
    list_init(&mutex->wait_list);
}