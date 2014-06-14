#ifndef SIMULATION_DRAWER_H_
#define SIMULATION_DRAWER_H_

#include "CrashSimulationThread.h"

#define WAITING_SPEECH_STATE 0
#define LISTENING_SPEECH_STATE 1

int initSimulationDrawer(int p_width, int p_height);
int drawSimulation(unsigned int p_carPosition, unsigned int p_distance, int p_carCrashed);
int destroySimulationDrawer();
int setSpeechState(int p_speechState);

#endif