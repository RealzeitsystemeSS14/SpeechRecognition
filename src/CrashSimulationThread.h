#ifndef CRASH_SIMULATION_THREAD_H_
#define CRASH_SIMULATION_THREAD_H_

#include <pthread.h>

typedef struct {
	unsigned int position;
	unsigned int velocity;
	unsigned int acceleration;
	unsigned int brakeAcceleration;
	
	int brake;
} crashCar_t;

/* A CrashSimulation simulates an accelerating car driving on a line (1 dimensional).
 * If the car brakes, it get negative acceleration until it sops completely.*/ 
typedef struct {
	crashCar_t car;
	unsigned int distance;
	
	pthread_mutex_t mutex;
} crashSimulation_t;

int initSimulation(crashSimulation_t *p_simulation, unsigned int p_velocity, unsigned int p_acceleration, unsigned int p_brakeAcceleration, unsigned int p_distance);
int destroySimulation(crashSimulation_t *p_simulation);

int brakeCar(crashSimulation_t *p_simulation);

/* stepSimulation() simulates one timestep.
 * If the car is accelerating, its velocity and position get increseased.
 * If the car is braking, its velocity gets decreased and (if velocity > 0) its position gets increased.
 * It returns 1 if the car has successfully stopped, -1 if the car crashed and 0 if the simulation is still running. */
int stepSimulation(crashSimulation_t *p_simulation);
int resetSimulation(crashSimulation_t *p_simulation);
int setSimulationParamter(crashSimulation_t *p_simulation, unsigned int p_velocity, unsigned int p_acceleration, unsigned int p_brakeAcceleration, unsigned int p_distance);
unsigned int getCarPosition(crashSimulation_t *p_simulation);
unsigned int getSimulationDistance(crashSimulation_t *p_simulation);

typedef struct {
	pthread_t thread;
	crashSimulation_t simulation;
	
	int exitCode;
	volatile int simulate;
	volatile int keepRunning;
	volatile int running;
} crashSimulationThread_t;

int initCrashSimulationThread(crashSimulationThread_t *p_thread);
int destroyCrashSimulationThread(crashSimulationThread_t *p_thread);

int startCrashSimulationThread(crashSimulationThread_t *p_thread);
int stopCrashSimulationThread(crashSimulationThread_t *p_thread);
int joinCrashSimulationThread(crashSimulationThread_t *p_thread);

int startSimulation(crashSimulationThread_t *p_thread);
int stopSimultaion(crashSimulationThread_t *p_thread);

#endif