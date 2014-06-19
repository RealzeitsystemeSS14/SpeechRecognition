/* The HypothesisMapper handles hypothesises, which it gets from the hypQueue.
 * Depending on the received hypothesis it calls different functions to react.
 * All function return 0 on success. */
 
#include "BlockingQueue.h"
#include "CrashSimulationThread.h"

typedef struct {
	crashSimulationThread_t *simulationThread;
	blockingQueue_t *hypQueue;
	
	volatile int keepRunning;
} hypothesisMapper_t;

int initHypothesisMapper(hypothesisMapper_t *p_mapper, blockingQueue_t *p_hypQueue, crashSimulationThread_t *p_simulationThread);
int loopHypothesisMapper(hypothesisMapper_t *p_mapper);
int stopHypothesisMapper(hypothesisMapper_t *p_mapper);