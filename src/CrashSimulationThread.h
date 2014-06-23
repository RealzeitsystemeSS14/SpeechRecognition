/* The CrashSimulationThread runs at a fixe rate and simulates a crash test with a car.
 * This thread handles the simulation logic as well as redrawing the GUI.
 * All functions return 0 on success.*/

#ifndef CRASH_SIMULATION_THREAD_H_
#define CRASH_SIMULATION_THREAD_H_

#include <pthread.h>
#include "CrashSimulation.h"
#include "InputThread.h"

typedef struct {
	pthread_t thread;
	crashSimulation_t simulation;
	pthread_mutex_t simulationMutex;
	pthread_barrier_t startBarrier;
	
	inputThread_t *inputThread;
	
	int exitCode;
	volatile int simulate;
	volatile int keepRunning;
	volatile int running;
} crashSimulationThread_t;

int initCrashSimulationThread(crashSimulationThread_t *p_thread, inputThread_t *p_inputThread);
int destroyCrashSimulationThread(crashSimulationThread_t *p_thread);

int startCrashSimulationThread(crashSimulationThread_t *p_thread);
int stopCrashSimulationThread(crashSimulationThread_t *p_thread);
int joinCrashSimulationThread(crashSimulationThread_t *p_thread);

/* The simulation has to be manipulated over the CrashSimulationThread,
 * so all calls are handled threadsafe. */
int startCrashSimulation(crashSimulationThread_t *p_thread);
int stopCrashSimulation(crashSimulationThread_t *p_thread);
int resetCrashSimulation(crashSimulationThread_t *p_thread);
int brakeCar(crashSimulationThread_t *p_thread);

#endif