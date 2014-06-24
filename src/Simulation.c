#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include "Simulation.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MIN_RESPAWN_TIME 5
#define MAX_WAIT 3
#define SPAWN_OFFSET(s) ((2 * s->distance) / 3)

int initSimulation(rtSimulation_t *p_simulation, int p_distance)
{		
	srand(time(NULL));
	p_simulation->distance = p_distance;
	p_simulation->position = TOP_POSITION;
	restartSimulation(p_simulation);
	
	return 0;
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
			respawnTimes[i] = SPAWN_OFFSET(p_simulation);
		}
		
		if(positions[i] < 0) {
			// position is negative, so we have to wait to respawn
			if(respawnTimes[i] == 0) {
				// we respawn after respawn time
				positions[i] = 0;
			}
			if(respawnTimes[i] > 0)
				--respawnTimes[i];
		}
		if(positions[i] >= 0) {
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
	for(i = 0; i < OBSTACLE_COUNT; ++i) {
		p_simulation->topPositions[i] = -1;
		p_simulation->topRespawnTimes[i] = ((2 * i) - 1) * SPAWN_OFFSET(p_simulation);
		p_simulation->botPositions[i] = -1;
		p_simulation->botRespawnTimes[i] = (2 * i) * SPAWN_OFFSET(p_simulation);
	}
}