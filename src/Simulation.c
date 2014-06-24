#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include "Simulation.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MIN_RESPAWN_TIME 5
#define MAX_WAIT 3

int initSimulation(rtSimulation_t *p_simulation, int p_distance, int p_minObstacleDistance)
{		
	srand(time(NULL));
	p_simulation->distance = p_distance;
	p_simulation->position = TOP_POSITION;
	p_simulation->minObstacleDistance = p_minObstacleDistance;
	restartSimulation(p_simulation);
	
	return 0;
}

static int stepDiff(rtSimulation_t *p_simulation)
{
	int diff = p_simulation->currentStep - p_simulation->lastRespawn;
	if(p_simulation->lastRespawn > p_simulation->currentStep)
		diff = diff + INT_MAX + 1;
	
	return diff;
}

static void moveObstacles(rtSimulation_t *p_simulation, int p_position)
{
	int *positions, *respawnTimes;
	int i;
	
	if(p_position == TOP_POSITION) {
		positions = p_simulation->topPositions;
		respawnTimes = p_simulation->topRespawnTimes;
	} else {
		positions = p_simulation->botPositions;
		respawnTimes = p_simulation->botRespawnTimes;
	}
	
	for(i = 0; i < OBSTACLE_COUNT; ++i) {
		if(positions[i] >= p_simulation->distance) {
			// is out of field
			positions[i] = -1;
			respawnTimes[i] = MIN_RESPAWN_TIME + (abs(rand()) % MAX_WAIT);
		}
		
		if(positions[i] < 0) {
			// position is negative, so we have to wait to respawn
			if(respawnTimes[i] == 0) {
				int diff = stepDiff(p_simulation);
				if(diff >= p_simulation->minObstacleDistance) {
					// we respawn after respawn time
					positions[i] = 0;
					p_simulation->lastRespawn = p_simulation->currentStep;
				} else {
					respawnTimes[i] = p_simulation->minObstacleDistance - diff;
				}
			}
			if(respawnTimes[i] > 0)
				--respawnTimes[i];
		} else {
			// position is not negative, so move obstacle
			++positions[i];
		}
	}
}

int stepSimulation(rtSimulation_t *p_simulation)
{
	++p_simulation->currentStep;
	
	// move obstacles
	moveObstacles(p_simulation, TOP_POSITION);
	moveObstacles(p_simulation, BOT_POSITION);
	
	int i;
	// check if we hit an obstacle
	for(i = 0; i < OBSTACLE_COUNT; ++i) {
		if(p_simulation->position == TOP_POSITION) {
			if(p_simulation->topPositions[i] == p_simulation->distance)
				return 1;
		} else {
			if(p_simulation->botPositions[i] == p_simulation->distance)
				return 1;
		}
	}
	
	return 0;
}

void restartSimulation(rtSimulation_t *p_simulation)
{
	int i;
	p_simulation->currentStep = 0;
	p_simulation->lastRespawn = 0;
	for(i = 0; i < OBSTACLE_COUNT; ++i) {
		p_simulation->topPositions[i] = p_simulation->distance + 1;
		p_simulation->botPositions[i] = p_simulation->distance + 1;
	}
}