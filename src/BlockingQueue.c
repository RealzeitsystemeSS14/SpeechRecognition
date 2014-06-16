#include <stdlib.h>
#include "BlockingQueue.h"

#define IS_EMPTY(queue) (queue->size == 0)
#define IS_FULL(queue) (queue->size == queue->maxSize)

int initBlockingQueue(blockingQueue_t *p_queue)
{
	int ret;
	p_queue->maxSize = BLOCKING_QUEUE_SIZE;
	if(p_queue->elements == NULL) 
		return -1;
		
	p_queue->size = 0;
	
	ret = pthread_mutex_init(&p_queue->mutex, NULL);
	if(ret != 0)
		return ret;
		
	ret = pthread_cond_init(&p_queue->popCondition, NULL);
	if(ret != 0)
		return ret;
	
	ret = pthread_cond_init(&p_queue->pushCondition, NULL);
	if(ret != 0)
		return ret;
	
	return 0;
}

int destroyBlockingQueue(blockingQueue_t *p_queue, int p_freeElements)
{
	int ret;
	ret = pthread_cond_destroy(&p_queue->pushCondition);
	if(ret != 0)
		return ret;
	
	ret = pthread_cond_destroy(&p_queue->popCondition);
	if(ret != 0)
		return ret;
	
	ret = pthread_mutex_destroy(&p_queue->mutex);
	if(ret != 0)
		return ret;
		
	clearBlockingQueue(p_queue, p_freeElements);
	
	return 0;
}

void enqueueBlockingQueue(blockingQueue_t *p_queue, void *p_element)
{
	pthread_mutex_lock(&p_queue->mutex);
	
	// block if queue is full
	while(IS_FULL(p_queue))
		pthread_cond_wait(&p_queue->pushCondition, &p_queue->mutex);
	
	p_queue->elements[p_queue->size] = p_element;
	++p_queue->size;
	
	pthread_mutex_unlock(&p_queue->mutex);
	pthread_cond_signal(&p_queue->popCondition);
}

void* dequeueBlockingQueue(blockingQueue_t* p_queue)
{
	pthread_mutex_lock(&p_queue->mutex);
	
	// block if queue is empty
	while(IS_EMPTY(p_queue))
		pthread_cond_wait(&p_queue->popCondition, &p_queue->mutex);
	
	void *result = p_queue->elements[0];
	--p_queue->size;
	
	int i;
	for(i = 0; i < p_queue->size; ++i)
		p_queue->elements[i] = p_queue->elements[i + 1];
	
	pthread_mutex_unlock(&p_queue->mutex);
	
	return result;
}

void clearBlockingQueue(blockingQueue_t* p_queue, int p_freeElements)
{
	
	pthread_mutex_lock(&p_queue->mutex);
	
	if(p_freeElements) {
		int i;
		for(i = 0; i < p_queue->size; ++i)
			free(p_queue->elements[i]);
	}
	
	p_queue->size = 0;
	
	pthread_mutex_unlock(&p_queue->mutex);
}

unsigned int sizeBlockingQueue(blockingQueue_t* p_queue)
{
	pthread_mutex_lock(&p_queue->mutex);
	unsigned int result = p_queue->size;
	pthread_mutex_unlock(&p_queue->mutex);
	
	return result;
}