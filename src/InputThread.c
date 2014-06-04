#include <string.h>
#include "AudioBuffer.h"
#include "InputThread.h"
#include "Utils.h"

#define BUFFER_SIZE 4096

static int record(inputThread_t *p_thread)
{
    int ret;
    int16 buf[BUFFER_SIZE];
	//create audioBuffer for audioQueue
	audioBuffer_t *resultBuf = malloc(sizeof(audioBuffer_t));
    
	if(resultBuf == NULL) {
		PRINT_ERR("Could not malloc resultBuf.\n");
		return -1;
	}
	
    printf("Listening for input...");
	//start recording from audiodevice
	ret = ad_start_rec(p_thread->sphinx.audioDevice);
    if (ret < 0) {
		PRINT_ERR("Could not start recording audio (%d).\n", ret);
        return ret;
	}
    
	//check if not silent
	while ((ret = cont_ad_read(p_thread->sphinx.contAudioDevice, buf, BUFFER_SIZE)) == 0)
        usleep(1000);
	
	//add read audio data to audioBuffer
	addAudioBuffer(resultBuf, buf, ret);
	
	/*ret = ps_start_utt(p_thread->sphinx.psDecoder, NULL);
    if (ret < 0)
        return ret;
	
	ret = ps_process_raw(p_thread->sphinx.psDecoder, buf, BUFFER_SIZE, 0, 0);
    if (ret < 0)
        return ret;*/
    
    do
    {
        ret = cont_ad_read(p_thread->sphinx.contAudioDevice, buf, BUFFER_SIZE);

        if (ret < 0) {
			//something went wrong
            PRINT_ERR("Failed to record audio (%d).\n", ret);
            return ret;
        } else if(ret > 0) {
            // Valid speech data read
            addAudioBuffer(resultBuf, buf, ret);
        } else {
            //no data
            usleep(1000);
        }
    } while(p_thread->record);
    
    ad_stop_rec(p_thread->sphinx.audioDevice);
    while (ad_read(p_thread->sphinx.audioDevice, buf, BUFFER_SIZE) >= 0);
    cont_ad_reset(p_thread->sphinx.contAudioDevice);
    /*ps_end_utt(p_thread->sphinx.psDecoder);*/
	
	enqueueBlockingQueue(p_thread->audioQueue, (void*) resultBuf);
	
	printf("[Finished]\n");
	
    return 0;
}

static void* runThread(void * arg)
{
	printf("InputThread started.\n");
	inputThread_t *sphinxThread = (inputThread_t*) arg;
	
	while(1) {
		if(sphinxThread->record) {
			sphinxThread->exitCode = record(sphinxThread);
			if(sphinxThread->exitCode != 0)
				break;
		}
	}
	printf("InputThread terminated.\n");
	return &sphinxThread->exitCode;
}

int initInputThread(inputThread_t *p_thread, blockingQueue_t *p_audioQueue, cmd_ln_t *p_config)
{
	int ret;
	
	p_thread->audioQueue = p_audioQueue;
	p_thread->running = 0;
	p_thread->record = 0;
	p_thread->exitCode = 0;
	
	ret = initSphinxInstance(&p_thread->sphinx, p_config);
	if(ret != 0) {
		PRINT_ERR("Failed to init SphinxInstance (%d).\n", ret);
		return ret;
	}
		
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
		
	ret = closeSphinxInstance(&p_thread->sphinx);
	if(ret != 0) {
		PRINT_ERR("InputThread: could not close SphinxInstance (%d).\n", ret);
		return ret;
	}
		
}

int startInputThread(inputThread_t *p_thread)
{
	int ret;
	
	ret = initSphinxRecord(&p_thread->sphinx);
	if(ret != 0) {
		PRINT_ERR("Failed to InitRecording (%d).\n", ret);
		return ret;
	}
	
	p_thread->running = 1;
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
	ret = pthread_cancel(p_thread->thread);
	if(ret != 0) {
		PRINT_ERR("Failed to cancel thread (%d).\n", ret);
		return ret;
	}
	
	p_thread->running = 0;
	
	ret = closeSphinxRecord(&p_thread->sphinx);
	if(ret != 0) {
		PRINT_ERR("Failed to close SphinxRecord (%d).\n", ret);
		return ret;
	}
		
	return 0;
}

int joinInputThread(inputThread_t *p_thread)
{
	void *ret;
	pthread_join(p_thread->thread, &ret);
	return p_thread->exitCode;
}

void startRecording(inputThread_t *p_thread)
{
	p_thread->record = 1;
}

void stopRecording(inputThread_t *p_thread)
{
	p_thread->record = 0;
}