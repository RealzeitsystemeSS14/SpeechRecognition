#include <time.h>
#include <stdlib.h>
#include "Simulation.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX_WAIT 3

int initSimulation(rtSimulation_t *p_simulation, int p_distance)
{		
	srand(time(NULL));
	p_simulation->distance = p_distance;
	p_simulation->position = TOP_POSITION;
	return 0;
}

int stepSimulation(rtSimulation_t *p_simulation)
{
	int i;
	// move obstacles
	for(i = 0; i < OBSTACLE_COUNT; ++i) {
		if(p_simulation->topPosition[i] >= p_simulation->distance) {
			p_simulation->topPosition[i] = -1;
			p_simulation->topWait[i] = abs(rand()) % MAX_WAIT;
		}
		
		if(p_simulation->topPosition[i] < 0) {
			// position is negative, so we have to wait to respawn
			if(p_simulation->topWait[i] == 0) {
				// we respawn after waiting time
				p_simulation->topPosition[i] = 0;
			} else
				--p_simulation->topWait[i];
		} else {
			// position is not negative, so move obstacle
			++p_simulation->topPosition[i];
		}
			
		if(p_simulation->botPosition[i] >= p_simulation->distance) {
			p_simulation->botPosition[i] = -1;
			p_simulation->botWait[i] = abs(rand()) % MAX_WAIT;
		}
		
		if(p_simulation->botPosition[i] < 0) {
			// position is negative, so we have to wait to respawn
			if(p_simulation->botWait[i] == 0) {
				// we respawn after waiting time
				p_simulation->botPosition[i] = 0;
			} else
				--p_simulation->botWait[i];
		} else {
			// position is not negative, so move obstacle
			++p_simulation->botPosition[i];
		}
	}
	
	// check if we hit an obstacle
	for(i = 0; i < OBSTACLE_COUNT; ++i) {
		if(p_simulation->position == TOP_POSITION) {
			if(p_simulation->topPosition[i] >= p_simulation->distance)
				return 1;
		} else {
			if(p_simulation->botPosition[i] >= p_simulation->distance)
				return 1;
		}
	}
	
	return 0;
}