#ifndef BLOCKING_QUEUE_H_
#define BLOCKING_QUEUE_H_

#include <pthread.h>

typedef struct blockingQueueElement {
	void *data;
	struct blockingQueueElement *next, *prev;
} blockingQueueElement_t;

typedef struct blockingQueue{
	blockingQueueElement_t *first, *last;
	unsigned int size;
	pthread_mutex_t mutex;
	pthread_cond_t popCondition;
} blockingQueue_t;

int initBlockingQueue(blockingQueue_t *p_queue);
int destroyBlockingQueue(blockingQueue_t *p_queue, int p_freeElements);
void enqueueBlockingQueue(blockingQueue_t *p_queue, void *p_element);
void* dequeueBlockingQueue(blockingQueue_t *p_queue);
void clearBlockingQueue(blockingQueue_t *p_queue, int p_freeElements);

unsigned int sizeBlockingQueue(blockingQueue_t *p_queue);
int isEmptyBlockingQueue(blockingQueue_t *p_queue);

#endif