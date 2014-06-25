#include <string.h>
#include "AudioBuffer.h"
#include "AudioBufferPool.h"
#include "InputThread.h"
#include "Utils.h"
#include "TimeTaking.h"
#include "RTScheduling.h"

#define MAX_RETRIES 10
#define SAMPLE_RATE DEFAULT_SAMPLES_PER_SEC
#define MS_TO_SAMPLES(ms) (((SAMPLE_RATE * 1000) / ms) / 1000)
#define SILENCE_MS 200

int initInputThread(inputThread_t *p_thread, blockingQueue_t *p_audioQueue)
{
	int ret;
	
	p_thread->audioQueue = p_audioQueue;
	p_thread->running = 0;
	p_thread->keepRunning = 0;
	p_thread->exitCode = 0;
	p_thread->listenState = INPUT_WAITING;
	
	ret = pthread_barrier_init(&p_thread->startBarrier, NULL, 2);
	if(ret != 0) {
		PRINT_ERR("Failed to int barrier (%d).\n", ret);
        return ret;
	}
		
	return 0;
}

int destroyInputThread(inputThread_t *p_thread)
{
	int ret;
	int result = 0;
	if(p_thread->running) {
		ret = stopInputThread(p_thread);
		if(ret != 0) {
			PRINT_ERR("Failed to stop thread (%d).\n", ret);
			return ret;
		}
			
		joinInputThread(p_thread);
	}
	
	ret = pthread_barrier_init(&p_thread->startBarrier, NULL, 2);
	if(ret != 0) {
		PRINT_ERR("Failed to destroy barrier (%d).\n", ret);
        return ret;
	}
	
	return result;
}

static int32 sphinxTimestampDiff(int32 p_begin, int32 p_end)
{
	int32 result = p_end - p_begin;
	if(p_end < p_begin)
		result = result + MAX_INT32 + 1;
	return result;
}

static int record(inputThread_t *p_thread)
{
    int ret = 0;
	int32 ts;
	//get audioBuffer for audioQueue
	audioBuffer_t *resultBuf = reserveAudioBuffer();
	
	ret = initAudioBuffer(resultBuf);
	if(ret != 0) {
		PRINT_ERR("Failed to init resultBuf %d).\n", ret);
		return ret;
	}
	
	//start recording from audiodevice
	p_thread->listenState = INPUT_LISTENING;
	ret = ad_start_rec(p_thread->audioDevice);
    if (ret < 0) {
		PRINT_ERR("Could not start recording audio (%d).\n", ret);
        return ret;
	}
	
	//TODO HOLD_TIME_TAKING(inputExecutionTime);
	//check if not silent
	while (((ret = cont_ad_read(p_thread->contAudioDevice, p_thread->inputBuffer, INPUT_BUFFER_SIZE)) == 0) && p_thread->keepRunning ) 
        usleep(10000);
	//TODO RESUME_TIME_TAKING(inputExecutionTime);
	//TODO RESTART_TIME_TAKING(totalReactionTime);
	ts = p_thread->contAudioDevice->read_ts;
	p_thread->listenState = INPUT_PROCESSING;	
	//add read audio data to audioBuffer
	addAudioBuffer(resultBuf, p_thread->inputBuffer, ret);
	
	
    while(p_thread->keepRunning) {
        ret = cont_ad_read(p_thread->contAudioDevice, p_thread->inputBuffer, INPUT_BUFFER_SIZE);

        if (ret < 0) {
			//something went wrong
            PRINT_ERR("Failed to record audio (%d).\n", ret);
            break;
        } else if(ret > 0) {
            // valid speech data read
			// get new timestamp
			ts = p_thread->contAudioDevice->read_ts;
            addAudioBuffer(resultBuf, p_thread->inputBuffer, ret);
			if(isFullAudioBuffer(resultBuf)) {
				PRINT_INFO("AudioBuffer is full!\n");
				break;
			}
        } else {
            //no data
			if(sphinxTimestampDiff(ts, p_thread->contAudioDevice->read_ts) >= MS_TO_SAMPLES(SILENCE_MS))
				break;
			else
				usleep(10000);
        }
    }
    
    ad_stop_rec(p_thread->audioDevice);
	p_thread->listenState = INPUT_WAITING;
    while (ad_read(p_thread->audioDevice, p_thread->inputBuffer, INPUT_BUFFER_SIZE) >= 0);
    cont_ad_reset(p_thread->contAudioDevice);
	
	//TODO HOLD_TIME_TAKING(inputExecutionTime);
	// enqueuing can also block
	enqueueBlockingQueue(p_thread->audioQueue, (void*) resultBuf);
	//TODO RESUME_TIME_TAKING(inputExecutionTime);
	
    return ret;
}

static int openAudioDevice(inputThread_t *p_thread)
{
	int ret;
	
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
	
	pthread_barrier_wait(&p_thread->startBarrier);
	
	return 0;
}

static void closeAudioDevice(inputThread_t *p_thread) 
{
	cont_ad_close(p_thread->contAudioDevice);
    ad_close(p_thread->audioDevice);
}

static void* runThread(void * arg)
{
	inputThread_t *sphinxThread = (inputThread_t*) arg;
	
	// sphinx creates own thread for audio device
	// sphinx thread shall inherit attributes of input thread
	if(openAudioDevice(sphinxThread) != 0)
		exit(-1);
	
	PRINT_INFO("InputThread started.\n");
	
	sphinxThread->running = 1;
	sphinxThread->exitCode = 0;
	unsigned int retries = 0;
	
	while(sphinxThread->keepRunning) {
		
		//TODO RESTART_TIME_TAKING(inputExecutionTime);
		sphinxThread->exitCode = record(sphinxThread);
		//TODO STOP_TIME_TAKING(inputExecutionTime);
		
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
	}
	
	closeAudioDevice(sphinxThread);
	sphinxThread->running = 0;
	PRINT_INFO("InputThread terminated.\n");
	pthread_exit(&sphinxThread->exitCode);
}

int startInputThread(inputThread_t *p_thread)
{
	int ret;
	pthread_attr_t attr;
	
	ret = initRTThreadAttr(&attr, INPUT_STACKSIZE, INPUT_PRIORITY);
	if(ret != 0) {
		PRINT_ERR("Failed to init rt pthread attributes (%d).\n", ret);
		return ret;
	}
	
	p_thread->keepRunning = 1;
	ret = pthread_create(&p_thread->thread, &attr, runThread, p_thread);
	if(ret != 0) {
		PRINT_ERR("Failed to create Thread (%d).\n", ret);
		return ret;
	}
	
	pthread_attr_destroy(&attr);
	
	pthread_barrier_wait(&p_thread->startBarrier);
	
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
	return *((int*) ret);
}