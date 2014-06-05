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
	
	//init decoder, used to interprete audio data
	p_thread->psDecoder = ps_init(p_config);
    if (p_thread->psDecoder == NULL) {
		PRINT_ERR("Failed to init Decoder (%d).\n", ret);
        return -2;
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
	
	//free decoder
	ret = ps_free(p_thread->psDecoder);
	if(ret != 0) {
		PRINT_ERR("Failed to free decoder (%d).\n", ret);
		return ret;
	}
	
	return 0;
}

static int interprete(interpreterThread_t * p_thread, audioBuffer_t *buffer, char **p_outHyp)
{
	int ret;
	char const *hyp, *uttid;
    int32 score;
	
	//start utterance, in which the data ist interpreted
	ret = ps_start_utt(p_thread->psDecoder, NULL);
    if (ret < 0) {
		PRINT_ERR("Failed to start utterance (%d).\n", ret);
		return ret;
	}
        
	//interprete the audio data
	ret = ps_process_raw(p_thread->psDecoder, buffer->buffer, buffer->size, 0, 0);
    if (ret < 0) {
		PRINT_ERR("Failed to process audio data (%d).\n", ret);
        return ret;
	}
		
	ps_end_utt(p_thread->psDecoder);
	
	//get result of interpretation
	hyp = ps_get_hyp(p_thread->psDecoder, &score, &uttid);
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
	audioBuffer_t *buffer;
	char *hyp;
	
	
	interpreterThread_t *interpreterThread = (interpreterThread_t*) arg;
	interpreterThread->running = 1;
	interpreterThread->exitCode = 0;
	
	while(interpreterThread->keepRunning) {
		buffer = (audioBuffer_t*) dequeueBlockingQueue(interpreterThread->audioQueue);
		if(buffer->size != 0) {
			interpreterThread->exitCode = interprete(interpreterThread, buffer, &hyp);
			if(interpreterThread->exitCode == 0)
				enqueueBlockingQueue(interpreterThread->hypQueue, (void*) hyp);
			else
				interpreterThread->keepRunning = 0;
		}
		free(buffer);
	}
	interpreterThread->running = 0;
	printf("InterpreterThread terminated.\n");
	
	pthread_exit(&interpreterThread->exitCode);
}

int startInterpreterThread(interpreterThread_t *p_thread)
{
	int ret;
	
	p_thread->keepRunning = 1;
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
	audioBuffer_t *poisonPill = malloc(sizeof(audioBuffer_t));
	ret = initAudioBuffer(poisonPill);
	if(ret != 0) {
		PRINT_ERR("Failed to init poison pill (%d).\n", ret);
		free(poisonPill);
		return ret;
	}
	enqueueBlockingQueue(p_thread->audioQueue, poisonPill);
	
	return 0;
}

int joinInterpreterThread(interpreterThread_t *p_thread)
{
	void *ret;
	pthread_join(p_thread->thread, &ret);
	return *((int*) ret);
}