#include "CrashSimulationThread.h"
#include "SimulationDrawer.h"
#include "Utils.h"
#include "TimeTaking.h"
#include "RTScheduling.h"

#define GUI_WIDTH 640
#define GUI_HEIGHT 480
#define DEF_DISTANCE 500000
#define DEF_ACCELERATION 100
#define DEF_BRAKE_ACCELERATION 100
#define SIMULATION_RATE 30
#define MIN_START_VEL (2 * SIMULATION_RATE * DEF_ACCELERATION)
#define MAX_START_VEL (2 * SIMULATION_RATE * DEF_ACCELERATION)

int initCrashSimulationThread(crashSimulationThread_t *p_thread)
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
	
	ret = initSimulation(&p_thread->simulation, DEF_ACCELERATION, DEF_BRAKE_ACCELERATION, DEF_DISTANCE, MIN_START_VEL, MAX_START_VEL);
	if(ret != 0) {
		PRINT_ERR("Failed to init simulation (%d).\n", ret);
		return ret;
	}
	p_thread->simulate = 0;
	p_thread->keepRunning = 0;
	p_thread->running = 0;
	p_thread->exitCode = 0;
	
	ret = initSimulationDrawer(GUI_WIDTH, GUI_HEIGHT);
	if(ret != 0) {
		PRINT_ERR("Failed to init SimulationDrawer (%d).\n", ret);
		return ret;
	}
	
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
	
	ret = destroySimulationDrawer();
	if(ret != 0) {
		PRINT_ERR("Failed to destroy SimulationDrawer (%d).\n", ret);
		return ret;
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
	HOLD_TIME_TAKING(simulationExecutionTime);
	pthread_mutex_lock(&p_thread->simulationMutex);
	RESUME_TIME_TAKING(simulationExecutionTime);
	RESTART_TIME_TAKING(simulationStepCSTime);
	int ret = stepSimulation(&p_thread->simulation);
	STOP_TIME_TAKING(simulationStepCSTime);
	pthread_mutex_unlock(&p_thread->simulationMutex);
	return ret;
}

static void drawSimulationThreadSafe(crashSimulationThread_t *p_thread, int p_status)
{
	HOLD_TIME_TAKING(simulationExecutionTime);
	pthread_mutex_lock(&p_thread->simulationMutex);
	RESUME_TIME_TAKING(simulationExecutionTime);
	int pos = p_thread->simulation.car.position;
	int dist = p_thread->simulation.distance;
	pthread_mutex_unlock(&p_thread->simulationMutex);
	
	drawSimulation(pos, dist, p_status);
}

static void* runThread(void *arg)
{
	PRINT_INFO("SimulationThread started.\n");
	rate_t loopRate;
	int ret = 0;
	
	crashSimulationThread_t *simulationThread = (crashSimulationThread_t*) arg;
	simulationThread->running = 1;
	simulationThread->exitCode = initRate(&loopRate, SIMULATION_RATE);
	if(simulationThread->exitCode != 0) {
		PRINT_ERR("Failed to init rate (%d).\n", simulationThread->exitCode);
		simulationThread->keepRunning = 0;
	}
	
	while(simulationThread->keepRunning) {
		// take time for simulation task
		RESTART_TIME_TAKING(simulationExecutionTime);
		if(simulationThread->simulate) {
			// simulate the crash simulation for one timestep
			ret = stepSimulationThreadSafe(simulationThread);
		}
		drawSimulationThreadSafe(simulationThread, ret);
		// stop watch before sleeping
		STOP_TIME_TAKING(simulationExecutionTime);
		
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
	pthread_attr_t attr;
	
	ret = initRTThreadAttr(&attr, SIMULATION_STACKSIZE, SIMULATION_PRIORITY);
	if(ret != 0) {
		PRINT_ERR("Failed to init rt thread attributes (%d).\n", ret);
		return ret;
	}
	
	p_thread->keepRunning = 1;
	ret = pthread_create(&p_thread->thread, NULL, runThread, p_thread);
	if(ret != 0) {
		PRINT_ERR("Failed to create simulation thread (%d).\n", ret);
		return ret;
	}
		
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
	HOLD_TIME_TAKING(mapperExecutionTime);
	pthread_mutex_lock(&p_thread->simulationMutex);
	RESUME_TIME_TAKING(mapperExecutionTime);
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
	HOLD_TIME_TAKING(mapperExecutionTime);
	pthread_mutex_lock(&p_thread->simulationMutex);
	RESUME_TIME_TAKING(mapperExecutionTime);
	p_thread->simulation.car.position = 0;
	p_thread->simulation.car.velocity = 0;
	p_thread->simulation.car.brake = 0;
	ret = randomSimulationStart(&p_thread->simulation);
	pthread_mutex_unlock(&p_thread->simulationMutex);
	
	return ret;
}

int brakeCar(crashSimulationThread_t *p_thread)
{
	HOLD_TIME_TAKING(mapperExecutionTime);
	pthread_mutex_lock(&p_thread->simulationMutex);
	RESUME_TIME_TAKING(mapperExecutionTime);
	if(simulationHasStarted(&p_thread->simulation))
		p_thread->simulation.car.brake = 1;
	pthread_mutex_unlock(&p_thread->simulationMutex);
	
	return 0;
}