#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <sys/time.h>

#define PRINT_ERR(msg, ...) fprintf(stderr, "In %s (%d): " msg , __FILE__, __LINE__, ##__VA_ARGS__)
#define PRINT_INFO(msg, ...) printf(msg, ##__VA_ARGS__)

typedef struct {
	unsigned int targetRate;
	struct timeval lastTimeStamp;
} rate_t;

int initRate(rate_t* p_rate, unsigned int p_targetRate);
int sleepRate(rate_t* p_rate);

#endif