#ifndef INPUT_THREAD_H_
#define INPUT_THREAD_H_

#include <pthread.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/cont_ad.h>
#include "BlockingQueue.h"

typedef struct {
	pthread_t thread;
	blockingQueue_t *audioQueue;
	ad_rec_t *audioDevice;
	cont_ad_t *contAudioDevice;
	
	pthread_cond_t startRecordCond;
	pthread_mutex_t startRecordMutex;
	pthread_cond_t stopRecordCond;
	pthread_mutex_t stopRecordMutex;
	
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
