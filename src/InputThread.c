#include <string.h>
#include "AudioBuffer.h"
#include "AudioBufferPool.h"
#include "InputThread.h"
#include "Utils.h"
#include "TimeTaking.h"

#define MAX_RETRIES 10
#define BUFFER_SIZE 1024
#define SAMPLE_RATE 48000

int initInputThread(inputThread_t *p_thread, blockingQueue_t *p_audioQueue)
{
	int ret;
	
	p_thread->audioQueue = p_audioQueue;
	p_thread->running = 0;
	p_thread->keepRunning = 0;
	p_thread->record = 0;
	p_thread->exitCode = 0;
	
	ret = pthread_mutex_init(&p_thread->recordMutex, NULL);
	if(ret != 0) {
		PRINT_ERR("Failed to init mutex (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_cond_init(&p_thread->recordCond, NULL);
	if(ret != 0) {
		PRINT_ERR("Failed to init condition variable (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_barrier_init(&p_thread->startBarrier, NULL, 2);
	if(ret != 0) {
		PRINT_ERR("Failed to init barrier (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_barrier_init(&p_thread->stopBarrier, NULL, 2);
	if(ret != 0) {
		PRINT_ERR("Failed to init barrier (%d).\n", ret);
		return ret;
	}
	
	// open audio device, used for recording audio data
	p_thread->audioDevice = ad_open();
    if (p_thread->audioDevice == NULL) {
		PRINT_ERR("Failed to open audio device.\n");
        return -1;
	}
	
	// init audio device as continous audio device
    p_thread->contAudioDevice = cont_ad_init(p_thread->audioDevice, ad_read);
    if (p_thread->contAudioDevice == NULL) {
		PRINT_ERR("Failed to init continuous audio device.\n");
        return -2;
	}
	
	// calibrate audio device
	ret = ad_start_rec(p_thread->audioDevice);
    if (ret < 0) {
		PRINT_ERR("Failed to start recording (%d).\n", ret);
        return ret;
	}
	
	ret = cont_ad_calib(p_thread->contAudioDevice);
    if (ret < 0) {
		PRINT_ERR("Failed to calibrate continuous audio device (%d).\n", ret);
        return ret;
	}

    ad_stop_rec(p_thread->audioDevice);
		
	return 0;
}

int destroyInputThread(inputThread_t *p_thread)
{
	int ret = 0;
	if(p_thread->running)
		ret = pthread_cancel(p_thread->thread);
	
	if(ret != 0) {
		PRINT_ERR("Failed to cancel thread (%d).\n", ret);
		return ret;
	}
		
	cont_ad_close(p_thread->contAudioDevice);
    ad_close(p_thread->audioDevice);
	
	ret = pthread_barrier_destroy(&p_thread->stopBarrier);
	if(ret != 0) {
		PRINT_ERR("Failed to destroy barrier (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_barrier_destroy(&p_thread->startBarrier);
	if(ret != 0) {
		PRINT_ERR("Failed to destroy barrier (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_cond_destroy(&p_thread->recordCond);
	if(ret != 0) {
		PRINT_ERR("Failed to destroy condition variable (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_mutex_destroy(&p_thread->recordMutex);
	if(ret != 0) {
		PRINT_ERR("Failed to destroy mutex (%d).\n", ret);
		return ret;
	}
	
	return 0;
}

static void signalStartRecording(inputThread_t *p_thread)
{
	pthread_barrier_wait(&p_thread->startBarrier);
	PRINT_INFO("Recording input...\n");
}

static void signalStopRecording(inputThread_t *p_thread)
{
	pthread_barrier_wait(&p_thread->stopBarrier);
	PRINT_INFO("Stopped recording.\n");
}

static int record(inputThread_t *p_thread)
{
    int ret = 0;
    int16 buf[BUFFER_SIZE];
	
	//get audioBuffer for audioQueue
	audioBuffer_t *resultBuf = reserveAudioBuffer();
	
	ret = initAudioBuffer(resultBuf);
	if(ret != 0) {
		PRINT_ERR("Failed to init resultBuf %d).\n", ret);
		return ret;
	}
	
    signalStartRecording(p_thread);
	//start recording from audiodevice
	ret = ad_start_rec(p_thread->audioDevice);
    if (ret < 0) {
		PRINT_ERR("Could not start recording audio (%d).\n", ret);
        return ret;
	}
	
	//check if not silent
	while (((ret = cont_ad_read(p_thread->contAudioDevice, buf, BUFFER_SIZE)) == 0) && p_thread->record) 
        usleep(1000);
		
	//add read audio data to audioBuffer
	addAudioBuffer(resultBuf, buf, ret);
	
	PRINT_INFO("Received audio.\n");
	
    while(p_thread->record) {
        ret = cont_ad_read(p_thread->contAudioDevice, buf, BUFFER_SIZE);

        if (ret < 0) {
			//something went wrong
            PRINT_ERR("Failed to record audio (%d).\n", ret);
            break;
        } else if(ret > 0) {
            // valid speech data read
            addAudioBuffer(resultBuf, buf, ret);
			if(isFullAudioBuffer(resultBuf))
				PRINT_INFO("AudioBuffer is full!\n");
        } else {
            //no data
            usleep(1000);
        }
    }
    
    ad_stop_rec(p_thread->audioDevice);
    while (ad_read(p_thread->audioDevice, buf, BUFFER_SIZE) >= 0)
		addAudioBuffer(resultBuf, buf, ret);
    cont_ad_reset(p_thread->contAudioDevice);
	
	signalStopRecording(p_thread);
	startTimeTaking(&globalTime);
	enqueueBlockingQueue(p_thread->audioQueue, (void*) resultBuf);
	
    return ret;
}

static void* runThread(void * arg)
{
	PRINT_INFO("InputThread started.\n");
	inputThread_t *sphinxThread = (inputThread_t*) arg;
	
	pthread_mutex_lock(&sphinxThread->recordMutex);
	sphinxThread->running = 1;
	sphinxThread->exitCode = 0;
	unsigned int retries = 0;
	
	while(sphinxThread->keepRunning) {
		
		// wait until recording should start
		while(!sphinxThread->record && sphinxThread->keepRunning)
			pthread_cond_wait(&sphinxThread->recordCond, &sphinxThread->recordMutex);
		
		// stop if woken up because stopThread() was called
		if(!sphinxThread->keepRunning)
			break;
		
		//TODO time taking
		startTimeTaking(&globalTime);
		startTimeTaking(&inputTime);
		sphinxThread->exitCode = record(sphinxThread);
		
		
		// if decoding failed retry MAX_RETRIES times
		if(sphinxThread->exitCode != 0)
			++retries;
		else
			retries = 0;
		//terminate input thread, beacause max number of retries reached	
		if(retries >= MAX_RETRIES) {
			PRINT_ERR("Recording failed. Retries: %d. Aborting.\n", retries);
			break;
		}
		stopTimeTaking(&inputTime);
	}
	
	sphinxThread->running = 0;
	pthread_mutex_unlock(&sphinxThread->recordMutex);
	
	PRINT_INFO("InputThread terminated.\n");
	pthread_exit(&sphinxThread->exitCode);
}

int startInputThread(inputThread_t *p_thread)
{
	int ret;
	
	p_thread->record = 0;
	p_thread->keepRunning = 1;
	ret = pthread_create(&p_thread->thread, NULL, runThread, p_thread);
	if(ret != 0) {
		PRINT_ERR("Failed to create Thread (%d).\n", ret);
		return ret;
	}
	
	return 0;
}

int stopInputThread(inputThread_t *p_thread)
{
	int ret;
	p_thread->keepRunning = 0;
	pthread_cond_signal(&p_thread->recordCond);
	return 0;
}

int joinInputThread(inputThread_t *p_thread)
{
	void *ret;
	pthread_join(p_thread->thread, &ret);
	return *((int*) ret);
}

int startRecording(inputThread_t *p_thread)
{
	if(p_thread->record) {
		PRINT_ERR("Cannot start recording, already recording.\n");
		return -1;
	}
	
	pthread_mutex_lock(&p_thread->recordMutex);
	p_thread->record = 1;
	pthread_mutex_unlock(&p_thread->recordMutex);
	// signal to start recording
	pthread_cond_signal(&p_thread->recordCond);
	// wait until recording has started
	pthread_barrier_wait(&p_thread->startBarrier);
	
	return 0;
}

int stopRecording(inputThread_t *p_thread)
{
	if(!p_thread->record) {
		PRINT_ERR("Cannot stop recording, not recording.\n");
		return -1;
	}
	
	p_thread->record = 0;
	// wait until recording has stopped
	pthread_barrier_wait(&p_thread->stopBarrier);
	
	return 0;
}