#include "InterpreterThread.h"
#include "AudioBuffer.h"
#include "Utils.h"

int initInterpreterThread(interpreterThread_t *p_thread, blockingQueue_t *p_audioQueue, blockingQueue_t *p_hypQueue, cmd_ln_t *p_config)
{
	int ret;
	
	p_thread->audioQueue = p_audioQueue;
	p_thread->hypQueue = p_hypQueue;
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

int destroyInterpreterThread(interpreterThread_t *p_thread)
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
		PRINT_ERR("Failed to close SphinxInstance (%d).\n", ret);
		return ret;
	}
}

static int interprete(interpreterThread_t * p_thread, audioBuffer_t *buffer, char **p_outHyp)
{
	int ret;
	char const *hyp, *uttid;
    int32 score;
	
	ret = ps_start_utt(p_thread->sphinx.psDecoder, NULL);
    if (ret < 0) {
		PRINT_ERR("Failed to start utterance (%d).\n", ret);
		return ret;
	}
        
	ret = ps_process_raw(p_thread->sphinx.psDecoder, buffer->buffer, buffer->size, 0, 0);
    if (ret < 0) {
		PRINT_ERR("Failed to process audio data (%d).\n", ret);
        return ret;
	}
		
	ps_end_utt(p_thread->sphinx.psDecoder);
	
	hyp = ps_get_hyp(p_thread->sphinx.psDecoder, &score, &uttid);
    if (hyp == NULL) {
        PRINT_ERR("Failed to get hypothesis.\n");
        return -1;
    }
	
	*p_outHyp = malloc(sizeof(char) * strlen(hyp) + 1);
	strcpy(*p_outHyp, hyp);
	
	return 0;
} 

static void* runThread(void * arg)
{
	printf("InterpreterThread started.\n");
	interpreterThread_t *interpreterThread = (interpreterThread_t*) arg;
	audioBuffer_t *buffer;
	char *hyp;
	int ret;
	
	while(interpreterThread->keepRunning) {
		buffer = (audioBuffer_t*) dequeueBlockingQueue(interpreterThread->audioQueue);
		if(buffer->size != 0) {
			ret = interprete(interpreterThread, buffer, &hyp);
			if(ret == 0)
				enqueueBlockingQueue(interpreterThread->hypQueue, (void*) hyp);
			else {
				interpreterThread->keepRunning = 0;
				interpreterThread->exitCode = -1;
			}
		}
		free(buffer);
	}
	
	ret = closeSphinxRecord(&interpreterThread->sphinx);
	if(ret != 0)
		PRINT_ERR("Failed to close SphinxRecord (%d).\n", ret);
	
	printf("InterpreterThread terminated.\n");
	
	return &interpreterThread->exitCode;
}

int startInterpreterThread(interpreterThread_t *p_thread)
{
	int ret;
	
	ret = initSphinxRecord(&p_thread->sphinx);
	if(ret != 0) {
		PRINT_ERR("Failed to init SphinxRecord (%d).\n", ret);
		return ret;
	}
	
	p_thread->keepRunning = 1;
	p_thread->running = 1;
	ret = pthread_create(&p_thread->thread, NULL, runThread, p_thread);
	if(ret != 0) {
		PRINT_ERR("Failed to create thread (%d).\n", ret);
		return ret;
	}
	
	return 0;
}

int stopInterpreterThread(interpreterThread_t *p_thread)
{
	int ret;
	p_thread->keepRunning = 0;
	p_thread->running = 0;
	audioBuffer_t *poisonPill = malloc(sizeof(audioBuffer_t));
	ret = initAudioBuffer(poisonPill);
	if(ret != 0) {
		PRINT_ERR("Failed to init poison pill (%d).\n", ret);
		return ret;
	}
	enqueueBlockingQueue(p_thread->audioQueue, poisonPill);
	
	return 0;
}

int joinInterpreterThread(interpreterThread_t *p_thread)
{
	void *retVal;
	int ret;
	pthread_join(p_thread->thread, &retVal);
		
	return p_thread->exitCode;
}