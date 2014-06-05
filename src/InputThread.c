#include <string.h>
#include "AudioBuffer.h"
#include "InputThread.h"
#include "Utils.h"

#define BUFFER_SIZE 4096

static void signalStartRecording(inputThread_t *p_thread)
{
	pthread_mutex_lock(&p_thread->startRecordMutex);
	printf("Listening for input...\n");
	pthread_cond_signal(&p_thread->startRecordCond);
	pthread_mutex_unlock(&p_thread->startRecordMutex);
}

static void signalStopRecording(inputThread_t *p_thread)
{
	pthread_mutex_lock(&p_thread->stopRecordMutex);
	printf("Stopped listening.\n");
	pthread_cond_signal(&p_thread->stopRecordCond);
	pthread_mutex_unlock(&p_thread->stopRecordMutex);
}

static int record(inputThread_t *p_thread)
{
    int ret;
    int16 buf[BUFFER_SIZE];
	
	//create audioBuffer for audioQueue
	audioBuffer_t *resultBuf = malloc(sizeof(audioBuffer_t));
	if(resultBuf == NULL) {
		PRINT_ERR("Failed to malloc resultBuf.\n");
		return -1;
	}
	
	ret = initAudioBuffer(resultBuf);
	if(ret != 0) {
		PRINT_ERR("Failed to init resultBuf %d).\n", ret);
		return ret;
	}
    
	//start recording from audiodevice
	ret = ad_start_rec(p_thread->audioDevice);
    if (ret < 0) {
		PRINT_ERR("Could not start recording audio (%d).\n", ret);
        return ret;
	}
    
	signalStartRecording(p_thread);
	//check if not silent
	while ((ret = cont_ad_read(p_thread->contAudioDevice, buf, BUFFER_SIZE)) == 0 && p_thread->record)
        usleep(1000);
	
	//add read audio data to audioBuffer
	addAudioBuffer(resultBuf, buf, ret);
    
    while(p_thread->record) {
        ret = cont_ad_read(p_thread->contAudioDevice, buf, BUFFER_SIZE);

        if (ret < 0) {
			//something went wrong
            PRINT_ERR("Failed to record audio (%d).\n", ret);
            return ret;
        } else if(ret > 0) {
            // valid speech data read
            addAudioBuffer(resultBuf, buf, ret);
        } else {
            //no data
            usleep(1000);
        }
    }
    
    ad_stop_rec(p_thread->audioDevice);
    while (ad_read(p_thread->audioDevice, buf, BUFFER_SIZE) >= 0);
    cont_ad_reset(p_thread->contAudioDevice);
	
	signalStopRecording(p_thread);
	enqueueBlockingQueue(p_thread->audioQueue, (void*) resultBuf);
	
    return 0;
}

static void* runThread(void * arg)
{
	printf("InputThread started.\n");
	inputThread_t *sphinxThread = (inputThread_t*) arg;
	sphinxThread->running = 1;
	
	while(sphinxThread->keepRunning) {
		if(sphinxThread->record) {
			sphinxThread->exitCode = record(sphinxThread);
			if(sphinxThread->exitCode != 0)
				break;
		} else {
			usleep(1000);
		}
	}
	
	sphinxThread->running = 0;
	printf("InputThread terminated.\n");
	return &sphinxThread->exitCode;
}

int initInputThread(inputThread_t *p_thread, blockingQueue_t *p_audioQueue)
{
	int ret;
	
	p_thread->audioQueue = p_audioQueue;
	p_thread->running = 0;
	p_thread->keepRunning = 0;
	p_thread->record = 0;
	p_thread->exitCode = 0;
	
	//initialize audio device, used for recording audio data
	p_thread->audioDevice = ad_open();
    if (p_thread->audioDevice == NULL) {
		PRINT_ERR("Failed to open audio device.");
        return -1;
	}

    p_thread->contAudioDevice = cont_ad_init(p_thread->audioDevice, ad_read);
    if (p_thread->contAudioDevice == NULL) {
		PRINT_ERR("Failed to init continous audio device.");
        return -2;
	}

	ret = ad_start_rec(p_thread->audioDevice);
    if (ret < 0) {
		PRINT_ERR("Failed to start recording (%d).", ret);
        return ret;
	}
	
	ret = cont_ad_calib(p_thread->contAudioDevice);
    if (ret < 0) {
		PRINT_ERR("Failed to calibrate continous audio device (%d).", ret);
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
	return 0;
}

int joinInputThread(inputThread_t *p_thread)
{
	void *ret;
	pthread_join(p_thread->thread, &ret);
	return p_thread->exitCode;
}

int startRecording(inputThread_t *p_thread)
{
	if(p_thread->record) {
		
		return -1;
	}
	
	pthread_mutex_lock(&p_thread->startRecordMutex);
	p_thread->record = 1;
	// wait for recording to be started, else stop could be called too fast
	pthread_cond_wait(&p_thread->startRecordCond, &p_thread->startRecordMutex);
	pthread_mutex_unlock(&p_thread->startRecordMutex);
	
	return 0;
}

int stopRecording(inputThread_t *p_thread)
{
	if(!p_thread->record) {
		return -1;
	}
	
	pthread_mutex_lock(&p_thread->stopRecordMutex);
	p_thread->record = 0;
	pthread_cond_wait(&p_thread->stopRecordCond, &p_thread->stopRecordMutex);
	pthread_mutex_unlock(&p_thread->stopRecordMutex);
	
	return 0;
}