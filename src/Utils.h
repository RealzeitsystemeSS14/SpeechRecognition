#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <libgen.h>

#define PRINT_ERR(msg, ...) fprintf(stderr, "[ERROR] In %s (%d): " msg , basename(__FILE__), __LINE__, ##__VA_ARGS__)
#define PRINT_INFO(msg, ...) printf(msg, ##__VA_ARGS__)

typedef	struct {
	struct timeval lastStamp;
	struct timeval measured;
}  stopWatch_t;

int startWatch(stopWatch_t* p_watch);
int stopWatch(stopWatch_t* p_watch);
void resetWatch(stopWatch_t* p_watch);

float getWatchSec(stopWatch_t* p_watch);
unsigned int getWatchMSec(stopWatch_t* p_watch);
useconds_t getWatchUSec(stopWatch_t* p_watch);

typedef struct {
	unsigned int targetRate;
	unsigned int lastDiffUS;
	stopWatch_t watch;
} rate_t;

int initRate(rate_t *p_rate, unsigned int p_targetRate);
unsigned int lastDiffUS(rate_t *p_rate);
int sleepRate(rate_t *p_rate);
void toLowerCase(char *p_str);

#endif