#include <queue.h>

//funcion para la nueva queue de bloqueados
BlockedQueueADT newQueue() {
    BlockedQueueADT queue = memory_manager_malloc(sizeof(BlockedQueueCDT));
    if (queue != NULL) {
        queue->head = NULL;
        queue->tail = NULL;
        queue->size = 0;
        return queue;
    }
    else{
        return NULL;
    }
}

//funcion para eliminar un proceso de la queue de bloqueados. Devuelve el PID
pid_t dequeuePid(BlockedQueueADT queue) {
    if (queue == NULL || queue->head == NULL) {
        return -1;
    }
    //apuntamos al primer elemento de la queue
    blockednode *node = queue->head;
    //actualizamos el puntero
    queue->head = node->next;
    pid_t pid = node->pid;
    free_memory_manager(node);
    queue->size--;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    return pid;
}

//funcion para agregar un proceso a la queue
void enqueuePid(BlockedQueueADT queue, pid_t pid) {
    if (queue == NULL) {
        return;
    }
    blockednode *newNode = memory_manager_malloc(sizeof(blockednode));
    if (newNode == NULL) {
        return;
    }
    newNode->pid = pid;
    newNode->next = NULL;
    if (queue->tail == NULL) {
        queue->head = newNode;
        queue->tail = newNode;
    } else {
        queue->tail->next = newNode;
        queue->tail = newNode;
    }
    queue->size++;
}

//funcion para liberar la memoria de la queue
void freeQueue(BlockedQueueADT queue) {
    if (queue == NULL) {
        return;
    }
    while (queue->head != NULL) {
        blockednode *node = queue->head;
        queue->head = node->next;
        free_memory_manager(node);
    }
    free_memory_manager(queue);
}
