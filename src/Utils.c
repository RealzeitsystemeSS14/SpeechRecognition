#include "Utils.h"

#define USEC_PER_SEC 1000000
#define MSEC_PER_SEC 1000
#define USEC_PER_MSEC 1000

int startWatch(stopWatch_t* p_watch)
{
	return gettimeofday(p_watch, NULL);
}

int stopWatch(stopWatch_t* p_watch)
{
	int ret;
	struct timeval end;
	ret = gettimeofday(&end, NULL);
	if(ret != 0)
		return ret;
	
	p_watch->tv_sec = end.tv_sec - p_watch->tv_sec;
	
	if(end.tv_usec < p_watch->tv_usec) {
		p_watch->tv_sec -= 1;
		p_watch->tv_usec = USEC_PER_SEC - p_watch->tv_usec + end.tv_usec ;
	} else
		p_watch->tv_usec = end.tv_usec - p_watch->tv_usec;
	
	return 0;
}

float getWatchSec(stopWatch_t* p_watch)
{
	float result = p_watch->tv_sec;
	result += ((float) (p_watch->tv_usec / USEC_PER_MSEC)) / MSEC_PER_SEC;
	return result;
}
unsigned int getWatchMSec(stopWatch_t* p_watch)
{
	return ((unsigned int) p_watch->tv_sec) * MSEC_PER_SEC + p_watch->tv_usec / USEC_PER_MSEC;
}

useconds_t getWatchUSec(stopWatch_t* p_watch)
{
	return ((useconds_t) p_watch->tv_sec) * USEC_PER_SEC + p_watch->tv_usec;
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