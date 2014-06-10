#include "ButtonThread.h"
#include "Utils.h"

int initButtonThread(buttonThread_t *p_thread, inputThread_t *p_inputThread)
{
	p_thread->inputThread = p_inputThread;
	p_thread->running = 0;
	p_thread->exitCode = 0;
	
	return 0;
}

int destroyButtonThread(buttonThread_t *p_thread)
{
	int ret;
	ret = stopButtonThread(p_thread, 1);
	if(ret != 0) {
		PRINT_ERR("Failed to cancel thread (%d).\n", ret);
		return ret;
	}
	
	return 0;
}

static void* runThread(void * arg)
{
	PRINT_INFO("ButtonThread started.\n");
	buttonThread_t *buttonThread = (buttonThread_t*) arg;
	buttonThread->running = 1;
	buttonThread->exitCode = 0;
	
	while(buttonThread->keepRunning) {
		sleep(1);
		PRINT_INFO("Press RETURN to record data.\n");
		getchar();
		if(!buttonThread->keepRunning)
			break;
		startRecording(buttonThread->inputThread);
		
		PRINT_INFO("Press RETURN to end recording.\n");
		getchar();
		stopRecording(buttonThread->inputThread);
	}
	
	buttonThread->running = 0;
	PRINT_INFO("ButtonThread terminated.\n");
	pthread_exit(&buttonThread->exitCode);
}

int startButtonThread(buttonThread_t *p_thread)
{
	int ret;
	
	if(p_thread->running) {
		PRINT_ERR("Thread is already running.\n");
		return -1;
	}
	
	p_thread->keepRunning = 1;
	ret = pthread_create(&p_thread->thread, NULL, runThread, p_thread);
	if(ret != 0) {
		PRINT_ERR("Failed to create thread (%d).\n", ret);
		return ret;
	}
	
	return 0;
}

int stopButtonThread(buttonThread_t *p_thread, int p_force)
{
	int ret;
	if(p_force) {
		ret = pthread_cancel(p_thread->thread);
		p_thread->keepRunning = 0;
		if(ret != 0) {
			PRINT_ERR("Failed to cancel thread (%d).\n", ret);
			return ret;
		}
		p_thread->running = 0;
	} else {
		p_thread->keepRunning = 0;
	}
	
	return 0;
}

int joinButtonThread(buttonThread_t *p_thread)
{
	void *ret;
	pthread_join(p_thread->thread, &ret);
	return *((int*) ret);
}