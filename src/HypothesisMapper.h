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