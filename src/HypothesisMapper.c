#include <string.h>
#include <stdlib.h>
#include "HypothesisMapper.h"
#include "StringPool.h"
#include "Utils.h"
#include "TimeTaking.h"

#define STOP_CMD "stop"
#define START_CMD "run"
#define RESET_CMD "reset"
#define FLIP_CMD "flip"

#define POISON_PILL "poisonPill"
static char *poisonPill = POISON_PILL;

int initHypothesisMapper(hypothesisMapper_t *p_mapper, blockingQueue_t *p_hypQueue, rtSimulationThread_t *p_simulationThread)
{
	p_mapper->hypQueue = p_hypQueue;
	p_mapper->simulationThread = p_simulationThread;
	p_mapper->keepRunning = 0;
	
	return 0;
}

int loopHypothesisMapper(hypothesisMapper_t *p_mapper)
{
	char *hyp;
	
	PRINT_INFO("HypothesisMapper started.\n");
	
	p_mapper->keepRunning = 1;
	while(p_mapper->keepRunning) {
		hyp = dequeueBlockingQueue(p_mapper->hypQueue);
		// if poison pill jump to next iteration -> keepRunning should now be false
		if(hyp == poisonPill)
			continue;
		
		//TODO RESTART_TIME_TAKING(mapperExecutionTime);
		//change hypothesis to lower case, to make compare case insensitive
		toLowerCase(hyp);
		// check which command was called
		PRINT_INFO("Hypothesis received: %s.\n", hyp);
		if(strcmp(hyp, RESET_CMD) == 0)
			resetSimulation(p_mapper->simulationThread);
		else if(strcmp(hyp, START_CMD) == 0)
			startSimulation(p_mapper->simulationThread);
		else if(strcmp(hyp, STOP_CMD) == 0)
			stopSimulation(p_mapper->simulationThread);
		else if(strcmp(hyp, FLIP_CMD) == 0)
			flipPosition(p_mapper->simulationThread, BOT_POSITION);
		else
			PRINT_INFO("Received unknown hypothesis: %s.\n", hyp);
		
		//TODO STOP_TIME_TAKING(mapperExecutionTime);
		//TODO STOP_TIME_TAKING(totalReactionTime);
		releaseString(hyp);
	}
	
	PRINT_INFO("HypothesisMapper terminated.\n");
	
	return 0;
}

int stopHypothesisMapper(hypothesisMapper_t *p_mapper)
{
	p_mapper->keepRunning = 0;
	enqueueBlockingQueue(p_mapper->hypQueue, poisonPill);
	
	return 0;
}

