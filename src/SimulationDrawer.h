/* The simulation drawer visualizes the simualtion on the
 * screen. Allegro is used to draw in a window.
 * All functions return 0 on success. */

#ifndef SIMULATION_DRAWER_H_
#define SIMULATION_DRAWER_H_

#include "SimulationThread.h"

int initSimulationDrawer(int p_width, int p_height);
int drawSimulation(int p_position, int p_distance, int *p_topObstacles, int *p_botObstacles, int p_state, int p_listenState);
int destroySimulationDrawer();

#endif