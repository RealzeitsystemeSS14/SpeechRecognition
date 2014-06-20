#include <pthread.h>
#include "StringPool.h"
#include "Utils.h"

#define POOL_SIZE 5

static char pool[POOL_SIZE * STRING_SIZE];
static int inUse[POOL_SIZE];
static unsigned int inUseCount;

static pthread_mutex_t mutex;
static pthread_cond_t condition;

int initStringPool()
{
	int i, ret;
	pthread_mutexattr_t attr;
	
	inUseCount = 0;
	
	ret = initRTMutexAttr(&attr);
	if(ret != 0) {
		PRINT_ERR("Failed to int rt mutex attributes (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_mutex_init(&mutex, &attr);
	if(ret != 0) {
		PRINT_ERR("Failed to int mutex (%d).\n", ret);
		return ret;
	}
		
	ret = pthread_cond_init(&condition, NULL);
	if(ret != 0) {
		PRINT_ERR("Failed to int condition variable (%d).\n", ret);
		return ret;
	}
	
	for(i = 0; i < POOL_SIZE; ++i)
		inUse[i] = 0;
	
	pthread_mutexattr_destroy(&attr);
	
	return 0;
}

int destroyStringPool()
{
	int ret;
	
	ret = pthread_cond_destroy(&condition);
	if(ret != 0) {
		PRINT_ERR("Failed to destroy condition variable (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_mutex_destroy(&mutex);
	if(ret != 0) {
		PRINT_ERR("Failed to destroy mutex (%d).\n", ret);
		return ret;
	}
		
	return 0;
}

char* reserveString()
{
	pthread_mutex_lock(&mutex);
	// wait until a string is available
	while(inUseCount >= POOL_SIZE)
		pthread_cond_wait(&condition, &mutex);
	
	int i;
	char *result;
	for(i = 0; i < POOL_SIZE; ++i) {
		if(!inUse[i]) {
			result = pool + (i * STRING_SIZE);
			inUse[i] = 1;
			++inUseCount;
			break;
		}
	}
	
	pthread_mutex_unlock(&mutex);
		
	return result;
}

void releaseString(char *p_string)
{
	pthread_mutex_lock(&mutex);
	
	if(inUseCount != 0) {
		int i;
		for(i = 0; i < POOL_SIZE; ++i) {
			if(p_string == (pool + (i * STRING_SIZE))) {
				inUse[i] = 0;
				--inUseCount;
				break;
			}
		}
	}
	
	pthread_mutex_unlock(&mutex);
	pthread_cond_signal(&condition);
}