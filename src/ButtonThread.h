/* ButtonThread reacts to button presses, and starts / stops the 
 * recording of audio data according to the situation of the InputThread.
 * All functions return 0 on success. */

#ifndef BUTTON_THREAD_H_
#define BUTTON_THREAD_H_

#include <pthread.h>
#include "InputThread.h"

typedef struct {
	pthread_t thread;
	inputThread_t *inputThread;
	
	int exitCode;
	volatile int running;
	volatile int keepRunning;
} buttonThread_t;

int initButtonThread(buttonThread_t *p_thread, inputThread_t *p_inputThread);
int destroyButtonThread(buttonThread_t *p_thread);

int startButtonThread(buttonThread_t *p_thread);
int stopButtonThread(buttonThread_t *p_thread);
int joinButtonThread(buttonThread_t *p_thread);

#endif