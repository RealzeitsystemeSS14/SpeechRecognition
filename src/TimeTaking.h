#ifndef TIME_TAKING_H
#define TIME_TAKING_H

#include "Utils.h"

struct timeElement {
	stopWatch_t watch;
	useconds_t maxMsec;
};

extern struct timeElement inputTime;
extern struct timeElement interpreterTime;
extern struct timeElement simulationTime;
extern struct timeElement globalTime;
extern struct timeElement mapperTime;

int startTimeTaking(struct timeElement *p_element);
int stopTimeTaking(struct timeElement *p_element);

int saveTimesToFile(char *p_file);

#endif