#ifndef INPUT_THREAD_H_
#define INPUT_THREAD_H_

#include <pthread.h>
#include "BlockingQueue.h"
#include "SphinxInstance.h"

typedef struct {
	pthread_t thread;
	blockingQueue_t *audioQueue;
	sphinxInstance_t sphinx;
	
	int exitCode;
	volatile int running;
	volatile int record;
} inputThread_t;

int initInputThread(inputThread_t *p_thread, blockingQueue_t *p_audioQueue, cmd_ln_t *p_config);
int destroyInputThread(inputThread_t *p_thread);

int startInputThread(inputThread_t *p_thread);
int stopInputThread(inputThread_t *p_thread);
int joinInputThread(inputThread_t *p_thread);

void startRecording(inputThread_t *p_thread);
void stopRecording(inputThread_t *p_thread);

#endif
