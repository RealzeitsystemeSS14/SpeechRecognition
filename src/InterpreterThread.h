/* The InterpreterThread takes audio data from the given audioQueue
 * and interpretes it. A String hypothesis is created and queued in the given
 * hypQueue.
 * All functions return 0 on success.*/

#ifndef INTERPRETER_THREAD_H_
#define INTERPRETER_THREAD_H_

#include <pthread.h>
#include <pocketsphinx.h>
#include "BlockingQueue.h"

typedef struct {
	pthread_t thread;
	blockingQueue_t *audioQueue;
	blockingQueue_t *hypQueue;
	ps_decoder_t *psDecoder;
	
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