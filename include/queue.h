#ifndef QUEUE_H
# define QUEUE_H

#include <stddef.h>
#include "../include/process.h"


struct list
{
    int *socket;
    struct list *next;    
};

struct queue
{
    struct list *head;
    struct list *tail;
    size_t size;
};

/**
**      \return     new allocated queue
*/
struct queue *queue_init(void);

/**
**      \param      q               queue
**
**      \return     size of queue q
*/
size_t queue_size(struct queue *q);

/**
**      \param      q               queue
**      \param      elm             token to be pushed at the tail of the queue
**
**      \return     information about the completion of the task
*/
int queue_push(struct queue *q, int *socket);

/**
**      \param      q               queue
**
**      \return     token at the head of the queue
*/
int *queue_peek(struct queue *q);

/**
**      \param      q               queue
**
**      \return     token at the head of the queue
*/
int *queue_pop(struct queue *q);

void queue_print(struct queue *q);

/**
**      \param      q               queue
*/
void queue_destroy(struct queue *q);

#endif