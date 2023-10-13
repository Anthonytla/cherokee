#include "../include/queue.h"
#include<pthread.h>
#include <stdlib.h>
#include <stdio.h>

struct queue *queue_init(void)
{
    struct queue *q = calloc(1, sizeof(struct queue));
    if (!q)
        return NULL;
    return q;
}

size_t queue_size(struct queue *q)
{
    return q->size;
}

void queue_print(struct queue *q) {
    struct queue *tmp = q;
    while (tmp->head) {
        printf("%p\n", (void *) tmp->head);
        tmp->head = tmp->head->next;
    }
}

int queue_push(struct queue *q, int *socket)
{
    
    struct list *last = malloc(sizeof(struct list));
    last->next = NULL;
    last->socket = socket;
    q->size++;
    if (!q->tail)
    {
        // puts("tail null");
        q->head = last;
        q->tail = last;
    }
    else
    {
        q->tail->next = last;
        // printf("tail not null %p\n", (void *) q->tail->next);
        q->tail = last;
        
    }
    // printf("%p-%p-%p\n",(void *) q->tail->task, (void *)last, (void *) q->tail->next);
    //printf("%ld\n", q->size);
    return 1;
}

// int *queue_peek(struct queue *q)
// {
//     if (!q->head)
// 	return NULL;
//     return q->head->task;
// }
int *queue_pop(struct queue *q)
{
    
    if (!q->head)
        return NULL;
    int *tmp = NULL;
    if (q->head->socket != NULL) {
        tmp = q->head->socket;
        // Continue with your code here...
    } else {
    }
    // int *tmp = q->head->socket;
    //struct list *tmp2 = q->head;
    q->head = q->head->next;
    if (!q->head) {q->tail = NULL;}
    //free(tmp2);
    q->size = q->size - 1;
    return tmp;
    
}

void queue_destroy(struct queue *q)
{
    while (q->size != 0)
    {
        int *tmp = queue_pop(q);
        free(tmp);
    }
    free(q);
    q = NULL;
}
