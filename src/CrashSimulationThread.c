#include "CrashSimulationThread.h"
#include "SimulationDrawer.h"
#include "Utils.h"

#define DRAWER_WIDTH 640
#define DRAWER_HEIGHT 480
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define CAR_CRASHED(sim) (sim->car.position < sim->distance)
#define CAR_STOPPED(sim) (sim->car.velocity == 0)
#define CAR_BRAKES(sim) (p_simulation->car.brake)
 
int initSimulation(crashSimulation_t *p_simulation, unsigned int p_velocity, unsigned int p_acceleration, unsigned int p_brakeAcceleration, unsigned int p_distance)
{
	int ret;
	ret = pthread_mutex_init(&p_simulation->mutex, NULL);
	if(ret != 0) {
		PRINT_ERR("Failed to init mutex (%d).\n", ret);
		return ret;
	}
		
	p_simulation->car.velocity = p_velocity;
	p_simulation->car.acceleration = p_acceleration;
	p_simulation->car.brakeAcceleration = p_brakeAcceleration;
	p_simulation->car.brake = 0;
	p_simulation->distance = p_distance;
	
	ret = initSimulationDrawer(DRAWER_WIDTH, DRAWER_HEIGHT);
	if(ret != 0) {
		PRINT_ERR("Failed to init SimulationDrawer (%d).\n", ret);
		return ret;
	}
		
	
	return 0;
}

int destroySimulation(crashSimulation_t *p_simulation)
{
	int ret;
	ret = pthread_mutex_destroy(&p_simulation->mutex);
	if(ret != 0) {
		PRINT_ERR("Failed to destroy mutex (%d).\n", ret);
		return ret;
	}
	
	return 0;
}

int brakeCar(crashSimulation_t *p_simulation)
{
	pthread_mutex_lock(&p_simulation->mutex);
	p_simulation->car.brake = 1;
	pthread_mutex_unlock(&p_simulation->mutex);
	
	return 0;
}

int resetSimulation(crashSimulation_t *p_simulation)
{
	pthread_mutex_lock(&p_simulation->mutex);
	p_simulation->car.position = 0;
	p_simulation->car.brake = 0;
	pthread_mutex_unlock(&p_simulation->mutex);
	
	return 0;
}

int stepSimulation(crashSimulation_t *p_simulation)
{
	unsigned int toDec;
	int ret;
	
	pthread_mutex_lock(&p_simulation->mutex);
	// check if car already crashed into the wall
	if(CAR_CRASHED(p_simulation)) {
		if(CAR_BRAKES(p_simulation)) {
			if(CAR_STOPPED(p_simulation))
				ret = 1;
			else {
				// only if car has not crashed and has not stopped yet
				// simulate braking / slowing down
				p_simulation->car.position += p_simulation->car.velocity;
				
				toDec = MIN(p_simulation->car.brakeAcceleration, p_simulation->car.velocity);
				p_simulation->car.velocity -= p_simulation->car.brakeAcceleration;
			}
		} else {
			// only if car has not crashed and does not brake
			// simulate accelerating
			p_simulation->car.position += p_simulation->car.velocity;
			p_simulation->car.velocity += p_simulation->car.acceleration;
			ret = 0;
		}
	} else
		ret = -1;
	pthread_mutex_unlock(&p_simulation->mutex);
	
	return ret;
}

int setSimulationParamter(crashSimulation_t *p_simulation, unsigned int p_velocity, unsigned int p_acceleration, unsigned int p_brakeAcceleration, unsigned int p_distance)
{
	pthread_mutex_lock(&p_simulation->mutex);
	p_simulation->car.velocity = p_velocity;
	p_simulation->car.acceleration = p_acceleration;
	p_simulation->car.brakeAcceleration = p_brakeAcceleration;
	p_simulation->distance = p_distance;
	pthread_mutex_unlock(&p_simulation->mutex);
	
	return 0;
}

unsigned int getCarPosition(crashSimulation_t *p_simulation)
{
	unsigned int result;
	pthread_mutex_lock(&p_simulation->mutex);
	result = p_simulation->car.position;
	pthread_mutex_unlock(&p_simulation->mutex);
	return result;
}
unsigned int getSimulationDistance(crashSimulation_t *p_simulation)
{
	unsigned int result;
	pthread_mutex_lock(&p_simulation->mutex);
	result = p_simulation->distance;
	pthread_mutex_unlock(&p_simulation->mutex);
	return result;
}

/*###################################################################
 *############################ THREAD ###############################
 *###################################################################*/

#define DEF_DISTANCE 500000
#define DEF_ACCELERATION 100
#define DEF_BRAKE_ACCELERATION 100
#define DEF_VELOCITY 0
#define SIMULATION_RATE 30

int initCrashSimulationThread(crashSimulationThread_t *p_thread)
{
	int ret;
	ret = initSimulation(&p_thread->simulation, DEF_VELOCITY, DEF_ACCELERATION, DEF_BRAKE_ACCELERATION, DEF_DISTANCE);
	if(ret != 0)
		return ret;
	p_thread->simulate = 0;
	p_thread->keepRunning = 0;
	p_thread->running = 0;
	p_thread->exitCode = 0;
	
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
	
	ret = destroySimulation(&p_thread->simulation);
	if(ret != 0) {
		PRINT_ERR("Failed to destroy CrashSimulation (%d).\n", ret);
		return ret;
	}
		
	return 0;
}

static void* runThread(void *arg)
{
	PRINT_INFO("SimulationThread started.\n");
	rate_t loopRate;
	int ret;
	
	crashSimulationThread_t *simulationThread = (crashSimulationThread_t*) arg;
	simulationThread->running = 1;
	simulationThread->exitCode = initRate(&loopRate, SIMULATION_RATE);
	if(simulationThread->exitCode != 0)
		simulationThread->keepRunning = 0;
	
	while(simulationThread->keepRunning) {
		if(simulationThread->simulate) {
			// simulate the crash simulation for one timestep
			ret = stepSimulation(&simulationThread->simulation);
		}
		drawSimulation(simulationThread->simulation.car.position, simulationThread->simulation.distance, ret);
		
		simulationThread->exitCode = sleepRate(&loopRate);
		if(simulationThread->exitCode != 0)
			simulationThread->keepRunning = 0;
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