#include "ButtonThread.h"
#include "Utils.h"

int initButtonThread(buttonThread_t *p_thread, inputThread_t *p_inputThread)
{
	p_thread->inputThread = p_inputThread;
	p_thread->running = 0;
}

int destroyButtonThread(buttonThread_t *p_thread)
{
	int ret;
	if(p_thread->running) {
		ret = pthread_cancel(p_thread->thread);
		if(ret != 0) {
			PRINT_ERR("Failed to cancel thread (%d).\n", ret);
			return ret;
		}
	}
	
	p_thread->running = 0;
	
	return 0;
}

static void* runThread(void * arg)
{
	buttonThread_t *buttonThread = (buttonThread_t*) arg;
	while(1) {
		printf("Press RETURN to record data.\n");
		getchar();
		startRecording(buttonThread->inputThread);
		
		printf("Press RETURN to end recording.\n");
		getchar();
		stopRecording(buttonThread->inputThread);
	}
}

int startButtonThread(buttonThread_t *p_thread)
{
	int ret;
	
	p_thread->running = 1;
	ret = pthread_create(&p_thread->thread, NULL, runThread, p_thread);
	if(ret != 0) {
		PRINT_ERR("Failed to create thread (%d).\n", ret);
		return ret;
	}
	
	return 0;
}

int stopButtonThread(buttonThread_t *p_thread)
{
	int ret;
	ret = pthread_cancel(p_thread->thread);
	if(ret != 0) {
		PRINT_ERR("Failed to cancel thread (%d).\n", ret);
		return ret;
	}
	
	p_thread->running = 0;
	
	return 0;
}