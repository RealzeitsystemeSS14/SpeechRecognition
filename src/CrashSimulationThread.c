#include <allegro.h>
#include "CrashSimulationThread.h"
#include "SimulationDrawer.h"
#include "Utils.h"
#include "RTScheduling.h"
#include "TimeTaking.h"

#define GUI_WIDTH 640
#define GUI_HEIGHT 480
#define DEF_DISTANCE 500000
#define DEF_ACCELERATION 100
#define DEF_BRAKE_ACCELERATION 100
#define SIMULATION_RATE 30
#define SIMULATION_INTERVAL_US (1000000 / SIMULATION_RATE)
#define DEF_START_VELOCITY (SIMULATION_RATE * DEF_ACCELERATION)

int initCrashSimulationThread(crashSimulationThread_t *p_thread, inputThread_t *p_inputThread)
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
	
	ret = initSimulation(&p_thread->simulation, DEF_ACCELERATION, DEF_BRAKE_ACCELERATION, DEF_DISTANCE, DEF_START_VELOCITY);
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

int destroyCrashSimulationThread(crashSimulationThread_t *p_thread)
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

static int stepSimulationThreadSafe(crashSimulationThread_t *p_thread)
{
	holdTimeTaking();
	pthread_mutex_lock(&p_thread->simulationMutex);
	resumeTimeTaking();
	int ret = stepSimulation(&p_thread->simulation);
	pthread_mutex_unlock(&p_thread->simulationMutex);
	return ret;
}

static void drawSimulationThreadSafe(crashSimulationThread_t *p_thread, int p_status)
{
	holdTimeTaking();
	pthread_mutex_lock(&p_thread->simulationMutex);
	resumeTimeTaking();
	int pos = p_thread->simulation.car.position;
	int dist = p_thread->simulation.distance;
	pthread_mutex_unlock(&p_thread->simulationMutex);
	
	drawSimulation(pos, dist, p_status, p_thread->inputThread->listenState);
}

static int initAllegroComponents(crashSimulationThread_t *p_thread)
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

static int destroyAllegroComponents(crashSimulationThread_t *p_thread)
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
	int ret = 0;
	
	crashSimulationThread_t *simulationThread = (crashSimulationThread_t*) arg;
	//allegro has to be initialized int thread
	//allegro creates thread -> shall inherit from this one
	ret = initAllegroComponents(simulationThread);
	if(ret != 0)
		exit(0);
		
	PRINT_INFO("SimulationThread started.\n");	
	simulationThread->running = 1;
	
	while(simulationThread->keepRunning) {
		restartTimeTaking();
		if(simulationThread->simulate) {
			// simulate the crash simulation for one timestep
			ret = stepSimulationThreadSafe(simulationThread);
		}
		drawSimulationThreadSafe(simulationThread, ret);
		stopTimeTaking();
		usleep(10000);
	}
	
	destroyAllegroComponents(simulationThread);
	simulationThread->running = 0;
	PRINT_INFO("SimulationThread terminated.\n");
	pthread_exit(&simulationThread->exitCode);
}

int startCrashSimulationThread(crashSimulationThread_t *p_thread)
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

int stopCrashSimulationThread(crashSimulationThread_t *p_thread)
{
	p_thread->keepRunning = 0;
	
	return 0;
}

int joinCrashSimulationThread(crashSimulationThread_t *p_thread)
{
	void* retVal;
	pthread_join(p_thread->thread, &retVal);
	return *((int*) retVal);
}

int startCrashSimulation(crashSimulationThread_t *p_thread)
{
	pthread_mutex_lock(&p_thread->simulationMutex);
	p_thread->simulation.car.brake = 0;
	pthread_mutex_unlock(&p_thread->simulationMutex);
	p_thread->simulate = 1;
	return 0;
}

int stopCrashSimulation(crashSimulationThread_t *p_thread)
{
	p_thread->simulate = 0;
	return 0;
}

int resetCrashSimulation(crashSimulationThread_t *p_thread)
{
	int ret;
	stopCrashSimulation(p_thread);
	pthread_mutex_lock(&p_thread->simulationMutex);
	p_thread->simulation.car.position = 0;
	p_thread->simulation.car.velocity = 0;
	p_thread->simulation.car.brake = 0;
	p_thread->simulation.car.velocity = p_thread->simulation.startVelocity;
	ret = randomSimulationStart(&p_thread->simulation);
	pthread_mutex_unlock(&p_thread->simulationMutex);
	
	return ret;
}

int brakeCar(crashSimulationThread_t *p_thread)
{
	pthread_mutex_lock(&p_thread->simulationMutex);
	if(simulationHasStarted(&p_thread->simulation))
		p_thread->simulation.car.brake = 1;
	pthread_mutex_unlock(&p_thread->simulationMutex);
	
	return 0;
}