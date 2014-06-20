#ifndef TIME_TAKING_H
#define TIME_TAKING_H

#include "Utils.h"

#ifdef TAKE_TIME
	#define RESTART_TIME_TAKING(w) resetWatch(&w.watch); if(startWatch(&w.watch) != 0) {PRINT_ERR("Failed to start watch.\n");}
	#define HOLD_TIME_TAKING(w) if(stopWatch(&w.watch) != 0) {PRINT_ERR("Failed to stop watch.\n");}
	#define RESUME_TIME_TAKING(w) if(startWatch(&w.watch) != 0) {PRINT_ERR("Failed to start watch.\n");}
	#define STOP_TIME_TAKING(w) stopTimeTaking(&w)
	#define SAVE_TIMES_TO_FILE(f) saveTimesToFile(f)
#else
	#define RESTART_TIME_TAKING(w)
	#define HOLD_TIME_TAKING(w)
	#define RESUME_TIME_TAKING(w)
	#define STOP_TIME_TAKING(w)
	#define SAVE_TIMES_TO_FILE(f)
#endif

struct timeElement {
	stopWatch_t watch;
	unsigned int max;
};

extern struct timeElement inputExecutionTime;
extern struct timeElement interpreterExecutionTime;
extern struct timeElement simulationExecutionTime;
extern struct timeElement simulationStepCSTime;
extern struct timeElement mapperExecutionTime;

int initTimeTaking();
int stopTimeTaking(struct timeElement *p_element);
int saveTimesToFile(char *p_file);

#endif