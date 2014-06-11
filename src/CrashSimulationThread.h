#ifndef CRASH_SIMULATION_THREAD_H_
#define CRASH_SIMULATION_THREAD_H_

#include <pthread.h>
#include "CrashSimulation.h"

typedef struct {
	pthread_t thread;
	crashSimulation_t simulation;
	pthread_mutex_t simulationMutex;
	
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
int resetSimulation(crashSimulationThread_t *p_thread);
int brakeCar(crashSimulationThread_t *p_thread);

#endif