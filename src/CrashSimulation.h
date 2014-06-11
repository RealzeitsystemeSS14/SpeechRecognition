#ifndef CRASH_SIMULATION_H_
#define CRASH_SIMULATION_H_


typedef struct {
	unsigned int position;
	unsigned int velocity;
	unsigned int acceleration;
	unsigned int brakeAcceleration;
	
	int brake;
} crashCar_t;

/* A CrashSimulation simulates an accelerating car driving on a line (1 dimensional).
 * If the car brakes, it get negative acceleration until it sops completely.*/ 
typedef struct {
	crashCar_t car;
	unsigned int distance;
} crashSimulation_t;

int initSimulation(crashSimulation_t *p_simulation, unsigned int p_acceleration, unsigned int p_brakeAcceleration, unsigned int p_distance);

/* stepSimulation() simulates one timestep.
 * If the car is accelerating, its velocity and position get increseased.
 * If the car is braking, its velocity gets decreased and (if velocity > 0) its position gets increased.
 * It returns 1 if the car has successfully stopped, -1 if the car crashed and 0 if the simulation is still running. */
int stepSimulation(crashSimulation_t *p_simulation);

#endif