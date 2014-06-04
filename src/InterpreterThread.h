#ifndef INTERPRETER_THREAD_H_
#define INTERPRETER_THREAD_H_

#include <pthread.h>
#include "BlockingQueue.h"
#include "SphinxInstance.h"

typedef struct {
	pthread_t thread;
	blockingQueue_t *audioQueue;
	blockingQueue_t *hypQueue;
	sphinxInstance_t sphinx;
	
	int exitCode;
	volatile int running;
	volatile int record;
	volatile int keepRunning;
} interpreterThread_t;

int initInterpreterThread(interpreterThread_t *p_thread, blockingQueue_t *p_audioQueue, blockingQueue_t *p_hypQueue, cmd_ln_t *p_config);
int destroyInterpreterThread(interpreterThread_t *p_thread);

int startInterpreterThread(interpreterThread_t *p_thread);
int stopInterpreterThread(interpreterThread_t *p_thread);
int joinInterpreterThread(interpreterThread_t *p_thread);

#endif