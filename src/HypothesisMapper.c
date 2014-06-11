#include <string.h>
#include <stdlib.h>
#include "HypothesisMapper.h"
#include "Utils.h"

#define STOP_CMD "stop"
#define START_CMD "start"
#define RESET_CMD "reset"

#define POISON_PILL "poisonPill"
#define POISON_PILL_SIZE (strlen(POISON_PILL) + 1)

int initHypothesisMapper(hypothesisMapper_t *p_mapper, blockingQueue_t *p_hypQueue, crashSimulationThread_t *p_simulationThread)
{
	p_mapper->hypQueue = p_hypQueue;
	p_mapper->simulationThread = p_simulationThread;
	p_mapper->keepRunning = 0;
	
	return 0;
}

int loopHypothesisMapper(hypothesisMapper_t *p_mapper)
{
	char *hyp;
	p_mapper->keepRunning = 1;
	while(p_mapper->keepRunning) {
		hyp = dequeueBlockingQueue(p_mapper->hypQueue);
		//change hypothesis to lower case, to make compare case insensitive
		toLowerCase(hyp);
		// check which command was calles
		if(strcmp(hyp, START_CMD) == 0)
			startSimulation(p_mapper->simulationThread);
		else if(strcmp(hyp, STOP_CMD) == 0)
			brakeCar(p_mapper->simulationThread);
		else if(strcmp(hyp, RESET_CMD) == 0) {
			resetSimulation(p_mapper->simulationThread);
		} else
			PRINT_INFO("Received unknown hypothesis: %s.\n", hyp);
		
		free(hyp);
	}
}

int stopHypothesisMapper(hypothesisMapper_t *p_mapper)
{
	//create poison pill for hypQueue, else thread might be blocking forever
	char *poisonPill = malloc(sizeof(char) * POISON_PILL_SIZE);
	if(poisonPill == NULL)
		return -1;
	strcpy(poisonPill, POISON_PILL);
	
	p_mapper->keepRunning = 0;
	enqueueBlockingQueue(p_mapper->hypQueue, poisonPill);
	
	return 0;
}

