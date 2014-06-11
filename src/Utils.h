#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#define PRINT_ERR(msg, ...) fprintf(stderr, "In %s (%d): " msg , __FILE__, __LINE__, ##__VA_ARGS__)
#define PRINT_INFO(msg, ...) printf(msg, ##__VA_ARGS__)

typedef	struct timeval stopWatch_t;

int startWatch(stopWatch_t* p_watch);
int stopWatch(stopWatch_t* p_watch);
float getWatchSec(stopWatch_t* p_watch);
unsigned int getWatchMSec(stopWatch_t* p_watch);
useconds_t getWatchUSec(stopWatch_t* p_watch);

typedef struct {
	unsigned int targetRate;
	stopWatch_t watch;
} rate_t;

int initRate(rate_t* p_rate, unsigned int p_targetRate);
int sleepRate(rate_t* p_rate);
void toLowerCase(char *p_str);

#endif