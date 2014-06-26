/* The InputThread records incoming audio data. Recording is started when first audio
 * smaples are received. After a certain time of silence or if the AudioBuffer is full,
 * the recording is stopped.
 * The received data is queued in the audioQueue.
 * The audio device which is initialized by sphinx, uses its own internal thread to poll
 * data from the hardware device. Hence the audio device strcut has to be initialized from
 * within the InputThread, so it inherits the correct thread attributes.
 * All functions return 0 on success. */
 
#ifndef INPUT_THREAD_H_
#define INPUT_THREAD_H_

#include <pthread.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/cont_ad.h>
#include "BlockingQueue.h"
#include "Utils.h"

#define INPUT_BUFFER_SIZE 1024
#define INPUT_WAITING 0
#define INPUT_PROCESSING 1
#define INPUT_LISTENING 2

typedef struct {
	pthread_t thread;
	blockingQueue_t *audioQueue;
	ad_rec_t *audioDevice;
	cont_ad_t *contAudioDevice;
	
	pthread_barrier_t startBarrier;
	
	int16 inputBuffer[INPUT_BUFFER_SIZE];
	
	int exitCode;
	volatile int running;
	volatile int keepRunning;
	volatile int listenState;
} inputThread_t;

int initInputThread(inputThread_t *p_thread, blockingQueue_t *p_audioQueue);
int destroyInputThread(inputThread_t *p_thread);

int startInputThread(inputThread_t *p_thread);
int stopInputThread(inputThread_t *p_thread);
int joinInputThread(inputThread_t *p_thread);

#endif
