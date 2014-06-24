/* The Simulation represents a game, where obstacles appear on
 * top and bot lanes. A player has to evade these obstacles.
 * stepSimulation() returns 0, if the player did not hit any obstacle. */

#ifndef SIMULATION_H_
#define SIMULATION_H_

#define OBSTACLE_COUNT 2
#define TOP_POSITION 1
#define BOT_POSITION 2
 
typedef struct {
	int distance;
	int position;
	
	int topPositions[OBSTACLE_COUNT];
	int topRespawnTimes[OBSTACLE_COUNT];
	int botPositions[OBSTACLE_COUNT];
	int botRespawnTimes[OBSTACLE_COUNT];
	
	int lastRespawnTime;
	int currentStep;
} rtSimulation_t;

int initSimulation(rtSimulation_t *p_simulation, int p_distance);

/* stepSimulation() simulates one timestep.
 *  */
int stepSimulation(rtSimulation_t *p_simulation);
void restartSimulation(rtSimulation_t *p_simulation);

#endif