/* This BlockingQueue blocks on enqueueing and dequeueing. 
 * enqueueBlockingQueue() blocks if the queue is full.
 * dequeueBlockingQueue() blocks if the queue is empty .
 * All functions (return type int) return 0 on success. */
 
#ifndef BLOCKING_QUEUE_H_
#define BLOCKING_QUEUE_H_

#include <pthread.h>

#define BLOCKING_QUEUE_SIZE 5

typedef struct blockingQueue{
	void *elements[BLOCKING_QUEUE_SIZE];
	unsigned int size;
	unsigned int maxSize;
	
	pthread_mutex_t mutex;
	pthread_cond_t popCondition;
	pthread_cond_t pushCondition;
} blockingQueue_t;

int initBlockingQueue(blockingQueue_t *p_queue);
int destroyBlockingQueue(blockingQueue_t *p_queue, int p_freeElements);
void enqueueBlockingQueue(blockingQueue_t *p_queue, void *p_element);
void* dequeueBlockingQueue(blockingQueue_t *p_queue);
void clearBlockingQueue(blockingQueue_t *p_queue, int p_freeElements);

unsigned int sizeBlockingQueue(blockingQueue_t *p_queue);

#endif