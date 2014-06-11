#include "CrashSimulationThread.h"
#include "SimulationDrawer.h"
#include "Utils.h"

#define DRAWER_WIDTH 640
#define DRAWER_HEIGHT 480
#define DEF_DISTANCE 500000
#define DEF_ACCELERATION 100
#define DEF_BRAKE_ACCELERATION 100
#define SIMULATION_RATE 30

int initCrashSimulationThread(crashSimulationThread_t *p_thread)
{
	int ret;
	
	ret = pthread_mutex_init(&p_thread->simulationMutex, NULL);
	if(ret != 0) {
		PRINT_ERR("Failed to init mutex (%d).\n", ret);
		return ret;
	}
	
	ret = initSimulation(&p_thread->simulation, DEF_ACCELERATION, DEF_BRAKE_ACCELERATION, DEF_DISTANCE);
	if(ret != 0) {
		PRINT_ERR("Failed to init simulation (%d).\n", ret);
		return ret;
	}
	p_thread->simulate = 0;
	p_thread->keepRunning = 0;
	p_thread->running = 0;
	p_thread->exitCode = 0;
	
	ret = initSimulationDrawer(DRAWER_WIDTH, DRAWER_HEIGHT);
	if(ret != 0) {
		PRINT_ERR("Failed to init SimulationDrawer (%d).\n", ret);
		return ret;
	}
	
	return 0;
}

int destroyCrashSimulationThread(crashSimulationThread_t *p_thread)
{
	int ret;
	
	ret = destroySimulationDrawer();
	if(ret != 0) {
		PRINT_ERR("Failed to destroy SimulationDrawer (%d).\n", ret);
		return ret;
	}
	
	if(p_thread->running) {
		ret = pthread_cancel(p_thread->thread);
		if(ret != 0) {
			PRINT_ERR("Failed to cancel thread (%d).\n", ret);
			return ret;
		}
	}
	
	ret = pthread_mutex_destroy(&p_thread->simulationMutex);
	if(ret != 0) {
		PRINT_ERR("Failed to destroy mutex (%d).\n", ret);
		return ret;
	}
		
	return 0;
}

static int stepSimulationThreadSafe(crashSimulationThread_t *p_thread)
{
	pthread_mutex_lock(&p_thread->simulationMutex);
	int ret = stepSimulation(&p_thread->simulation);
	pthread_mutex_unlock(&p_thread->simulationMutex);
	return ret;
}

static void drawSimulationThreadSafe(crashSimulationThread_t *p_thread, int p_status)
{
	pthread_mutex_lock(&p_thread->simulationMutex);
	int pos = p_thread->simulation.car.position;
	int dist = p_thread->simulation.distance;
	pthread_mutex_unlock(&p_thread->simulationMutex);
	
	drawSimulation(pos, dist, p_status);
}

static void* runThread(void *arg)
{
	PRINT_INFO("SimulationThread started.\n");
	rate_t loopRate;
	int ret;
	
	crashSimulationThread_t *simulationThread = (crashSimulationThread_t*) arg;
	simulationThread->running = 1;
	simulationThread->exitCode = initRate(&loopRate, SIMULATION_RATE);
	if(simulationThread->exitCode != 0) {
		PRINT_ERR("Failed to init rate (%d).\n", simulationThread->exitCode);
		simulationThread->keepRunning = 0;
	}
	
	while(simulationThread->keepRunning) {
		if(simulationThread->simulate) {
			// simulate the crash simulation for one timestep
			
			ret = stepSimulationThreadSafe(simulationThread);
		}
		drawSimulationThreadSafe(simulationThread, ret);
		
		simulationThread->exitCode = sleepRate(&loopRate);
		if(simulationThread->exitCode != 0) {
			PRINT_ERR("Failed to sleep (%d).\n", simulationThread->exitCode);
			simulationThread->keepRunning = 0;
		}
	}
	
	simulationThread->running = 0;
	PRINT_INFO("SimulationThread terminated.\n");
	pthread_exit(&simulationThread->exitCode);
}

int startCrashSimulationThread(crashSimulationThread_t *p_thread)
{
	int ret;
	p_thread->keepRunning = 1;
	ret = pthread_create(&p_thread->thread, NULL, runThread, p_thread);
	if(ret != 0)
		return ret;
		
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

int startSimulation(crashSimulationThread_t *p_thread)
{
	p_thread->simulate = 1;
	return 0;
}

int stopSimultaion(crashSimulationThread_t *p_thread)
{
	p_thread->simulate = 0;
	return 0;
}

int resetSimulation(crashSimulationThread_t *p_thread)
{
	stopSimultaion(p_thread);
	pthread_mutex_lock(&p_thread->simulationMutex);
	p_thread->simulation.car.position = 0;
	p_thread->simulation.car.velocity = 0;
	p_thread->simulation.car.brake = 0;
	pthread_mutex_unlock(&p_thread->simulationMutex);
	
	return 0;
}

int brakeCar(crashSimulationThread_t *p_thread)
{
	pthread_mutex_lock(&p_thread->simulationMutex);
	p_thread->simulation.car.brake = 1;
	pthread_mutex_unlock(&p_thread->simulationMutex);
	
	return 0;
}