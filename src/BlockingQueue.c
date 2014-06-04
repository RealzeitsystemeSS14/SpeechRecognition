#include <stdlib.h>
#include "BlockingQueue.h"

int initBlockingQueue(blockingQueue_t *p_queue)
{
	int ret;
	p_queue->first = NULL;
	p_queue->last = NULL;
	p_queue->size = 0;
	
	ret =pthread_mutex_init(&p_queue->mutex, NULL);
	if(ret != 0)
		return ret;
		
	ret = pthread_cond_init(&p_queue->popCondition, NULL);
	if(ret != 0)
		return ret;
	
	return 0;
}

int destroyBlockingQueue(blockingQueue_t *p_queue, int p_freeElements)
{
	int ret;
	clearBlockingQueue(p_queue, p_freeElements);
	ret = pthread_cond_destroy(&p_queue->popCondition);
	if(ret != 0)
		return ret;
	
	ret = pthread_mutex_destroy(&p_queue->mutex);
	if(ret != 0)
		return ret;
		
	return 0;
}

void enqueueBlockingQueue(blockingQueue_t *p_queue, void *p_element)
{
	pthread_mutex_lock(&p_queue->mutex);
	
	blockingQueueElement_t* element = malloc(sizeof(blockingQueueElement_t));
	element->data = p_element;
	if(p_queue->size == 0) {
		element->prev = NULL;
		element->next = NULL;
		p_queue->first = element;
		p_queue->last = element;
	} else {
		p_queue->last->next = element;
		element->prev = p_queue->last;
		element->next = NULL;
	}
	
	++p_queue->size;

	pthread_mutex_unlock(&p_queue->mutex);
	pthread_cond_signal(&p_queue->popCondition);
}

void* dequeueBlockingQueue(blockingQueue_t* p_queue)
{
	pthread_mutex_lock(&p_queue->mutex);
	
	//wait until queue has content
	while(isEmptyBlockingQueue(p_queue))
		pthread_cond_wait(&p_queue->popCondition, &p_queue->mutex);
	
	blockingQueueElement_t *resultElement = p_queue->first->data;
	void *result = resultElement->data;
	
	//check if there are other elements in queue
	if(p_queue->size > 1) {
		p_queue->first = p_queue->first->next;
		p_queue->first->prev = NULL;
	} else {
		p_queue->first = NULL;
		p_queue->last = NULL;
	}
	--p_queue->size;
	free(resultElement);
	pthread_mutex_unlock(&p_queue->mutex);
	
	return result;
}

void clearBlockingQueue(blockingQueue_t* p_queue, int p_freeElements)
{
	blockingQueueElement_t* next, *current;
	
	pthread_mutex_lock(&p_queue->mutex);
	
	next = p_queue->first;
	while(next != NULL) {
		current = next;
		next = current->next;
		if(p_freeElements)
			free(current->data);
		free(current);
	}
	
	p_queue->first = NULL;
	p_queue->last = NULL;
	p_queue->size = 0;
	
	pthread_mutex_unlock(&p_queue->mutex);
}

unsigned int sizeBlockingQueue(blockingQueue_t* p_queue)
{
	return p_queue->size;
}

int isEmptyBlockingQueue(blockingQueue_t *p_queue)
{
	return p_queue->size == 0;
}