#ifndef SIMULATION_DRAWER_H_
#define SIMULATION_DRAWER_H_

#include "CrashSimulationThread.h"

int initSimulationDrawer(unsigned int p_width, unsigned int p_height);
int drawSimulation(unsigned int p_carPosition, unsigned int p_distance, int p_carCrashed);
int destroySimulationDrawer();

#endif