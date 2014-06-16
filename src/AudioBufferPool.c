#include <pthread.h>
#include "AudioBufferPool.h"
#include "Utils.h"

#define POOL_SIZE 5

static audioBuffer_t pool[POOL_SIZE];
static int inUse[POOL_SIZE];
static unsigned int inUseCount;

static pthread_mutex_t mutex;
static pthread_cond_t condition;

int initAudioBufferPool()
{
	int i, ret;
	
	inUseCount = 0;
	
	ret = pthread_mutex_init(&mutex, NULL);
	if(ret != 0) {
		PRINT_ERR("Failed to init mutex (%d).\n", ret);
		return ret;
	}
		
	ret = pthread_cond_init(&condition, NULL);
	if(ret != 0) {
		PRINT_ERR("Failed to init condition variable (%d).\n", ret);
		return ret;
	}
	
	for(i = 0; i < POOL_SIZE; ++i) {
		inUse[i] = 0;
		ret = initAudioBuffer(&pool[i]);
		if(ret != 0) {
			PRINT_ERR("Failed to init audioBuffer (%d).\n", ret);
			return ret;
		}
	}
	
	return 0;
}

int destroyAudioBufferPool()
{
	int ret;
	
	ret = pthread_cond_destroy(&condition);
	if(ret != 0) {
		PRINT_ERR("Failed to destroy condition variable (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_mutex_destroy(&mutex);
	if(ret != 0){
		PRINT_ERR("Failed to destroy mutex (%d).\n", ret);
		return ret;
	}
		
	return 0;
}

audioBuffer_t* reserveAudioBuffer()
{
	pthread_mutex_lock(&mutex);
	// wait until there is a buffer available
	while(inUseCount >= POOL_SIZE)
		pthread_cond_wait(&condition, &mutex);
	
	int i;
	audioBuffer_t* result = NULL;
	for(i = 0; i < POOL_SIZE; ++i) {
		if(!inUse[i]) {
			inUse[i] = 1;
			result = pool + i;
			inUseCount++;
			break;
		}
	}
	
	pthread_mutex_unlock(&mutex);
	
	return result;
}

void releaseAudioBuffer(audioBuffer_t *p_audioBuffer)
{
	pthread_mutex_lock(&mutex);
	
	if(inUseCount != 0) {
		int i;
		for(i = 0; i < POOL_SIZE; ++i) {
			if((pool + i) == p_audioBuffer) {
				inUse[i] = 0;
				inUseCount--;
				break;
			}
		}
	}
	
	pthread_mutex_unlock(&mutex);
	pthread_cond_signal(&condition);
}