#include "CrashSimulation.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define CAR_CRASHED(sim) (sim->car.position < sim->distance)
#define CAR_STOPPED(sim) (sim->car.velocity == 0)
#define CAR_BRAKES(sim) (p_simulation->car.brake)

int initSimulation(crashSimulation_t *p_simulation, unsigned int p_velocity, unsigned int p_acceleration, unsigned int p_brakeAcceleration, unsigned int p_distance)
{		
	p_simulation->car.velocity = p_velocity;
	p_simulation->car.acceleration = p_acceleration;
	p_simulation->car.brakeAcceleration = p_brakeAcceleration;
	p_simulation->car.brake = 0;
	p_simulation->distance = p_distance;
	
	return 0;
}

int stepSimulation(crashSimulation_t *p_simulation)
{
	unsigned int toDec;
	int ret;
	
	// check if car already crashed into the wall
	if(CAR_CRASHED(p_simulation)) {
		if(CAR_BRAKES(p_simulation)) {
			if(CAR_STOPPED(p_simulation))
				ret = 1;
			else {
				// only if car has not crashed and has not stopped yet
				// simulate braking / slowing down
				p_simulation->car.position += p_simulation->car.velocity;
				
				toDec = MIN(p_simulation->car.brakeAcceleration, p_simulation->car.velocity);
				p_simulation->car.velocity -= p_simulation->car.brakeAcceleration;
			}
		} else {
			// only if car has not crashed and does not brake
			// simulate accelerating
			p_simulation->car.position += p_simulation->car.velocity;
			p_simulation->car.velocity += p_simulation->car.acceleration;
			ret = 0;
		}
	} else
		ret = -1;
	
	return ret;
}