#ifndef BUTTON_THREAD_H_
#define BUTTON_THREAD_H_

#include <pthread.h>
#include "InputThread.h"

typedef struct {
	pthread_t thread;
	inputThread_t *inputThread;
	
	volatile int running;
} buttonThread_t;

int initButtonThread(buttonThread_t *p_thread, inputThread_t *p_inputThread);
int destroyButtonThread(buttonThread_t *p_thread);

int startButtonThread(buttonThread_t *p_thread);
int stopButtonThread(buttonThread_t *p_thread);

#endif