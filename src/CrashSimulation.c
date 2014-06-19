#include <time.h>
#include <stdlib.h>
#include "CrashSimulation.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define CAR_CRASHED(sim) (sim->car.position >= sim->distance)
#define CAR_STOPPED(sim) (sim->car.velocity == 0)
#define CAR_BRAKES(sim) (sim->car.brake)
#define MAX_START_STEP 60

int initSimulation(crashSimulation_t *p_simulation, unsigned int p_acceleration, unsigned int p_brakeAcceleration, unsigned int p_distance, unsigned int p_minStartVel, unsigned int p_maxStartVel)
{		
	srand(time(NULL));
	p_simulation->car.acceleration = p_acceleration;
	p_simulation->car.brakeAcceleration = p_brakeAcceleration;
	p_simulation->car.brake = 0;
	p_simulation->car.position = 0;
	p_simulation->distance = p_distance;
	p_simulation->minStartVel = p_minStartVel;
	p_simulation->maxStartVel = p_maxStartVel;
	if(randomSimulationStart(p_simulation) != 0)
		return -1;
	
	return 0;
}

int stepSimulation(crashSimulation_t *p_simulation)
{
	unsigned int toDec, toInc;
	int ret;
	
	if(!simulationHasStarted(p_simulation)) {
		++p_simulation->currentStep;
		return 0;
	}
	
	// check if car already crashed into the wall
	if(CAR_CRASHED(p_simulation)) {
		ret = -1;
	} else {
		if(CAR_BRAKES(p_simulation)) {
			if(CAR_STOPPED(p_simulation))
				ret = 1;
			else {
				// only if car has not crashed and has not stopped yet
				// simulate braking / slowing down
				p_simulation->car.position += p_simulation->car.velocity;
				
				toDec = MIN(p_simulation->car.brakeAcceleration, p_simulation->car.velocity);
				p_simulation->car.velocity -= toDec;
			}
		} else {
			// only if car has not crashed and does not brake
			// simulate accelerating
			toInc = MIN(p_simulation->car.velocity, p_simulation->distance - p_simulation->car.position);
			p_simulation->car.position += p_simulation->car.velocity;
			p_simulation->car.velocity += p_simulation->car.acceleration;
			ret = 0;
		}
	}
	
	return ret;
}

int randomSimulationStart(crashSimulation_t *p_simulation)
{
	p_simulation->currentStep = 0;
	p_simulation->startStep = abs(rand()) % MAX_START_STEP;
	p_simulation->car.velocity = (p_simulation->minStartVel + abs(rand())) % p_simulation->maxStartVel;
	return 0;
}

int simulationHasStarted(crashSimulation_t *p_simulation)
{
	return p_simulation->currentStep >= p_simulation->startStep;
}