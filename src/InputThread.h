/* The InputThread records incoming audio data. Recording can be controlled
 * with startRecording() and stopRecording().
 * The received data (after stopRecording()), is queued in the given BlockingQueue.
 * All functions return 0 on success. */
 
#ifndef INPUT_THREAD_H_
#define INPUT_THREAD_H_

#include <pthread.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/cont_ad.h>
#include "BlockingQueue.h"
#include "Utils.h"

typedef struct {
	pthread_t thread;
	blockingQueue_t *audioQueue;
	ad_rec_t *audioDevice;
	cont_ad_t *contAudioDevice;
	
	pthread_cond_t recordCond;
	pthread_mutex_t recordMutex;
	pthread_barrier_t startBarrier;
	pthread_barrier_t stopBarrier;
	
	int exitCode;
	volatile int running;
	volatile int keepRunning;
	volatile int record;
} inputThread_t;

int initInputThread(inputThread_t *p_thread, blockingQueue_t *p_audioQueue);
int destroyInputThread(inputThread_t *p_thread);

int startInputThread(inputThread_t *p_thread);
int stopInputThread(inputThread_t *p_thread);
int joinInputThread(inputThread_t *p_thread);

int startRecording(inputThread_t *p_thread);
int stopRecording(inputThread_t *p_thread);

#endif
