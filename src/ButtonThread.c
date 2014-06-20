#include <allegro.h>
#include "ButtonThread.h"
#include "Utils.h"
#include "SimulationDrawer.h"
#include "RTScheduling.h"

int initButtonThread(buttonThread_t *p_thread, inputThread_t *p_inputThread, void (*p_closeCallback) (void))
{
	install_keyboard();
	
	p_thread->inputThread = p_inputThread;
	p_thread->running = 0;
	p_thread->exitCode = 0;
	p_thread->closeCallback = p_closeCallback;
	
	return 0;
}

int destroyButtonThread(buttonThread_t *p_thread)
{
	int ret;
	if(p_thread->running) {
		ret = stopButtonThread(p_thread);
		if(ret != 0) {
			PRINT_ERR("Failed to stop thread (%d).\n", ret);
			return ret;
		}
		
		joinButtonThread(p_thread);
	}
	
	remove_keyboard();
	
	return 0;
}

static void* runThread(void * arg)
{
	PRINT_INFO("ButtonThread started.\n");
	buttonThread_t *buttonThread = (buttonThread_t*) arg;
	buttonThread->running = 1;
	buttonThread->exitCode = 0;
	int displayTxt = 1;
	
	while(buttonThread->keepRunning) {
		if(displayTxt) {
			PRINT_INFO("Press SPACE to record data.\n");
			displayTxt = 0;
		}
		
		if(key[KEY_SPACE]) {
			
			startRecording(buttonThread->inputThread);
			setSpeechState(LISTENING_SPEECH_STATE);
			
			while(key[KEY_SPACE] && buttonThread->keepRunning)
				usleep(10000);
				
			if(!buttonThread->keepRunning)
				break;
				
			stopRecording(buttonThread->inputThread);
			setSpeechState(WAITING_SPEECH_STATE);
			displayTxt = 1;
		} else if(key[KEY_ESC]) {
			buttonThread->closeCallback();
			break;
		}else {
			usleep(50000);
		}
	}
	
	buttonThread->running = 0;
	PRINT_INFO("ButtonThread terminated.\n");
	pthread_exit(&buttonThread->exitCode);
}

int startButtonThread(buttonThread_t *p_thread)
{
	int ret;
	pthread_attr_t attr;
	
	if(p_thread->running) {
		PRINT_ERR("Thread is already running.\n");
		return -1;
	}
	
	ret = initRTThreadAttr(&attr, BUTTON_STACKSIZE, BUTTON_PRIORITY);
	if(ret != 0) {
		PRINT_ERR("Failed to init rt thread attributes (%d).\n", ret);
		return ret;
	}
	
	p_thread->keepRunning = 1;
	ret = pthread_create(&p_thread->thread, &attr, runThread, p_thread);
	if(ret != 0) {
		PRINT_ERR("Failed to create thread (%d).\n", ret);
		return ret;
	}
	
	pthread_attr_destroy(&attr);
	
	return 0;
}

int stopButtonThread(buttonThread_t *p_thread)
{
	int ret;
	p_thread->keepRunning = 0;
	
	return 0;
}

int joinButtonThread(buttonThread_t *p_thread)
{
	void *ret;
	pthread_join(p_thread->thread, &ret);
	return *((int*) ret);
}