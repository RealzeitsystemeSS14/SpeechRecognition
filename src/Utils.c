#include "Utils.h"

#define USEC_PER_SEC 1000000
#define MSEC_PER_SEC 1000
#define USEC_PER_MSEC 1000

static void getTimevalDiff(struct timeval *p_begin, struct timeval *p_end, struct timeval *p_result)
{
	// copy begin to result
	*p_result = *p_begin;
 
	if (p_end->tv_usec < p_result->tv_usec) {
		int nsec = (p_result->tv_usec - p_end->tv_usec) / USEC_PER_SEC + 1;
		p_result->tv_usec -= USEC_PER_SEC * nsec;
		p_result->tv_sec += nsec;
	}
	if ((p_end->tv_usec - p_result->tv_usec) > USEC_PER_SEC) {
		int nsec = (p_end->tv_usec - p_result->tv_usec) / USEC_PER_SEC;
		p_result->tv_usec += USEC_PER_SEC * nsec;
		p_result->tv_sec -= nsec;
	}
	
	p_result->tv_sec = p_end->tv_sec - p_result->tv_sec;
	p_result->tv_usec = p_end->tv_usec - p_result->tv_usec;
}

int startWatch(stopWatch_t* p_watch)
{
	return gettimeofday(&p_watch->lastStamp, NULL);
}

int stopWatch(stopWatch_t* p_watch)
{
	int ret;
	struct timeval end, diff;
	ret = gettimeofday(&end, NULL);
	if(ret != 0)
		return ret;
		
	getTimevalDiff(&p_watch->lastStamp, &end, &diff);
	
	// add diff to total measurement
	p_watch->measured.tv_sec += diff.tv_sec;
	p_watch->measured.tv_usec += diff.tv_usec;
	
	return 0;
}

void resetWatch(stopWatch_t* p_watch)
{
	p_watch->measured.tv_sec = 0;
	p_watch->measured.tv_usec = 0;
}

float getWatchSec(stopWatch_t* p_watch)
{
	float result = p_watch->measured.tv_sec;
	result += ((float) (p_watch->measured.tv_usec / USEC_PER_MSEC)) / MSEC_PER_SEC;
	return result;
}
unsigned int getWatchMSec(stopWatch_t* p_watch)
{
	return ((unsigned int) p_watch->measured.tv_sec) * MSEC_PER_SEC + p_watch->measured.tv_usec / USEC_PER_MSEC;
}

unsigned int getWatchUSec(stopWatch_t* p_watch)
{
	return ((unsigned int) ((unsigned int)p_watch->measured.tv_sec) * USEC_PER_SEC + p_watch->measured.tv_usec);
}

int initRate(rate_t* p_rate, unsigned int p_targetRate)
{
	int ret;
	p_rate->targetRate = p_targetRate;
	ret = startWatch(&p_rate->watch);
	if(ret != 0)
		return ret;
		
	return 0;
}

int sleepRate(rate_t* p_rate) 
{
	useconds_t diffUsec, intervalUsec;
	int ret;
	
	ret = stopWatch(&p_rate->watch);
	if(ret != 0)
		return ret;
		
	diffUsec = getWatchUSec(&p_rate->watch);
	intervalUsec = USEC_PER_SEC / p_rate->targetRate;
	
	if(diffUsec < intervalUsec) {
		ret = usleep(intervalUsec - diffUsec);
		if(ret != 0)
			return ret;
	}
	
	resetWatch(&p_rate->watch);
	ret = startWatch(&p_rate->watch);
	if(ret != 0)
		return ret;
	
	return 0;
}

void toLowerCase(char *p_str)
{
	int i;
	for(i = 0; p_str[i]; i++){
		p_str[i] = tolower(p_str[i]);
	}
}