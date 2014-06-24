#include <allegro.h>
#include "SimulationThread.h"
#include "SimulationDrawer.h"
#include "Utils.h"
#include "RTScheduling.h"
#include "TimeTaking.h"

#define GUI_WIDTH 640
#define GUI_HEIGHT 480
#define DEF_DISTANCE 12
#define SIMULATION_RATE 5
#define SIMULATION_INTERVAL_US (1000000 / SIMULATION_RATE)
#define DEF_START_VELOCITY (SIMULATION_RATE * DEF_ACCELERATION)

int initCrashSimulationThread(rtSimulationThread_t *p_thread, inputThread_t *p_inputThread)
{
	int ret;
	pthread_mutexattr_t attr;
	
	ret = initRTMutexAttr(&attr);
	if(ret != 0) {
		PRINT_ERR("Failed to init rt mutex attributes (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_mutex_init(&p_thread->simulationMutex, &attr);
	if(ret != 0) {
		PRINT_ERR("Failed to init mutex (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_barrier_init(&p_thread->startBarrier, NULL, 2);
	if(ret != 0) {
		PRINT_ERR("Failed to init barrier (%d).\n", ret);
		return ret;
	}
	
	ret = initSimulation(&p_thread->simulation, DEF_DISTANCE);
	if(ret != 0) {
		PRINT_ERR("Failed to init simulation (%d).\n", ret);
		return ret;
	}
	p_thread->simulate = 0;
	p_thread->keepRunning = 0;
	p_thread->running = 0;
	p_thread->exitCode = 0;
	p_thread->inputThread = p_inputThread;
	
	pthread_mutexattr_destroy(&attr);
	
	return 0;
}

int destroyCrashSimulationThread(rtSimulationThread_t *p_thread)
{
	int ret;
	
	if(p_thread->running) {
		ret = stopCrashSimulationThread(p_thread);
		if(ret != 0) {
			PRINT_ERR("Failed to stop thread (%d).\n", ret);
			return ret;
		}
		
		joinCrashSimulationThread(p_thread);
	}
	
	ret = pthread_mutex_destroy(&p_thread->simulationMutex);
	if(ret != 0) {
		PRINT_ERR("Failed to destroy mutex (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_barrier_destroy(&p_thread->startBarrier);
		
	return 0;
}

static int stepSimulationThreadSafe(rtSimulationThread_t *p_thread)
{
	HOLD_TIME_TAKING(simulationExecutionTime);
	pthread_mutex_lock(&p_thread->simulationMutex);
	RESUME_TIME_TAKING(simulationExecutionTime);
	int ret = stepSimulation(&p_thread->simulation);
	pthread_mutex_unlock(&p_thread->simulationMutex);
	return ret;
}

static void drawSimulationThreadSafe(rtSimulationThread_t *p_thread, int p_status)
{
	int topObstalces[OBSTACLE_COUNT];
	int botObstalces[OBSTACLE_COUNT];
	int pos, dist, i;
	HOLD_TIME_TAKING(simulationExecutionTime);
	pthread_mutex_lock(&p_thread->simulationMutex);
	RESUME_TIME_TAKING(simulationExecutionTime);
	pos = p_thread->simulation.position;
	dist = p_thread->simulation.distance;
	for(i = 0; i < OBSTACLE_COUNT; ++i) {
		topObstalces[i] = p_thread->simulation.topPositions[i];
		botObstalces[i] = p_thread->simulation.botPositions[i];
	}
	pthread_mutex_unlock(&p_thread->simulationMutex);
	
	drawSimulation(pos, dist, topObstalces, botObstalces, p_status, p_thread->inputThread->listenState);
}

static int initAllegroComponents(rtSimulationThread_t *p_thread)
{
	int ret;
	PRINT_INFO("Init allegro...\n");
	allegro_init();
	ret = initSimulationDrawer(GUI_WIDTH, GUI_HEIGHT);
	if(ret != 0)
		PRINT_ERR("Failed to init SimulationDrawer (%d).\n", ret);
	
	pthread_barrier_wait(&p_thread->startBarrier);
	
	return ret;
}

static int destroyAllegroComponents(rtSimulationThread_t *p_thread)
{
	int ret;
	ret = destroySimulationDrawer();
	if(ret != 0)
		PRINT_ERR("Failed to destroy SimulationDrawer (%d).\n", ret);
	
	allegro_exit();
	return ret;
}

static void* runThread(void *arg)
{
	rate_t loopRate;
	int ret = 0;
	
	rtSimulationThread_t *simulationThread = (rtSimulationThread_t*) arg;
	//allegro has to be initialized int thread
	//allegro creates thread -> shall inherit from this one
	ret = initAllegroComponents(simulationThread);
	if(ret != 0)
		exit(0);
		
	PRINT_INFO("SimulationThread started.\n");	
	simulationThread->running = 1;
	simulationThread->exitCode = initRate(&loopRate, SIMULATION_RATE);
	if(simulationThread->exitCode != 0) {
		PRINT_ERR("Failed to init rate (%d).\n", simulationThread->exitCode);
		simulationThread->keepRunning = 0;
	}
	
	while(simulationThread->keepRunning) {
		RESTART_TIME_TAKING(simulationExecutionTime);
		if(simulationThread->simulate) {
			// simulate the crash simulation for one timestep
			ret = stepSimulationThreadSafe(simulationThread);
			if(ret != 0)
				stopSimulation(simulationThread);
		}
		drawSimulationThreadSafe(simulationThread, ret);
		STOP_TIME_TAKING(simulationExecutionTime);
		
		simulationThread->exitCode = sleepRate(&loopRate);
		if(simulationThread->exitCode != 0) {
			PRINT_ERR("Failed to sleep (%d).\n", simulationThread->exitCode);
			simulationThread->keepRunning = 0;
		}
	}
	
	destroyAllegroComponents(simulationThread);
	simulationThread->running = 0;
	PRINT_INFO("SimulationThread terminated.\n");
	pthread_exit(&simulationThread->exitCode);
}

int startCrashSimulationThread(rtSimulationThread_t *p_thread)
{
	int ret;
	pthread_attr_t attr;
	
	ret = initRTThreadAttr(&attr, SIMULATION_STACKSIZE, SIMULATION_PRIORITY);
	if(ret != 0) {
		PRINT_ERR("Failed to init rt thread attributes (%d).\n", ret);
		return ret;
	}
	
	p_thread->keepRunning = 1;
	ret = pthread_create(&p_thread->thread, &attr, runThread, p_thread);
	if(ret != 0) {
		PRINT_ERR("Failed to create simulation thread (%d).\n", ret);
		return ret;
	}
	
	pthread_attr_destroy(&attr);
	
	pthread_barrier_wait(&p_thread->startBarrier);
		
	return 0;
}

int stopCrashSimulationThread(rtSimulationThread_t *p_thread)
{
	p_thread->keepRunning = 0;
	
	return 0;
}

int joinCrashSimulationThread(rtSimulationThread_t *p_thread)
{
	void* retVal;
	pthread_join(p_thread->thread, &retVal);
	return *((int*) retVal);
}

int startSimulation(rtSimulationThread_t *p_thread)
{
	p_thread->simulate = 1;
	return 0;
}

int stopSimulation(rtSimulationThread_t *p_thread)
{
	p_thread->simulate = 0;
	return 0;
}

int resetSimulation(rtSimulationThread_t *p_thread)
{
	int i;
	stopSimulation(p_thread);
	pthread_mutex_lock(&p_thread->simulationMutex);
	restartSimulation(&p_thread->simulation);
	pthread_mutex_unlock(&p_thread->simulationMutex);
	
	return 0;
}

int flipPosition(rtSimulationThread_t *p_thread, int p_position)
{
	pthread_mutex_lock(&p_thread->simulationMutex);
	if(p_thread->simulate) {
		if(p_thread->simulation.position == TOP_POSITION)
			p_thread->simulation.position = BOT_POSITION;
		else
			p_thread->simulation.position = TOP_POSITION;
	}
	pthread_mutex_unlock(&p_thread->simulationMutex);
	
	return 0;
}