/* The CrashSimulationThread runs at a fixes rate and simulates a simple reaction game.
 * The player has to evade moving obstacles.
 * This thread handles the simulation logic as well as redrawing the GUI.
 * All functions return 0 on success.*/

#ifndef SIMULATION_THREAD_H_
#define SIMULATION_THREAD_H_

#include <pthread.h>
#include "Simulation.h"
#include "InputThread.h"

typedef struct {
	pthread_t thread;
	rtSimulation_t simulation;
	pthread_mutex_t simulationMutex;
	pthread_barrier_t startBarrier;
	
	inputThread_t *inputThread;
	
	int exitCode;
	volatile int simulate;
	volatile int keepRunning;
	volatile int running;
} rtSimulationThread_t;

int initCrashSimulationThread(rtSimulationThread_t *p_thread, inputThread_t *p_inputThread);
int destroyCrashSimulationThread(rtSimulationThread_t *p_thread);

int startCrashSimulationThread(rtSimulationThread_t *p_thread);
int stopCrashSimulationThread(rtSimulationThread_t *p_thread);
int joinCrashSimulationThread(rtSimulationThread_t *p_thread);

/* The simulation has to be manipulated over the RTSimulationThread,
 * so all calls are handled threadsafe. */
int startSimulation(rtSimulationThread_t *p_thread);
int stopSimulation(rtSimulationThread_t *p_thread);
int resetSimulation(rtSimulationThread_t *p_thread);
int flipPosition(rtSimulationThread_t *p_thread, int p_position);

#endif