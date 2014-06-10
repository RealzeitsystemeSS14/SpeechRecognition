#include <unistd.h>
#include "Utils.h"

#define USEC_PER_SEC 1000000

int initRate(rate_t* p_rate, unsigned int p_targetRate)
{
	int ret;
	p_rate->targetRate = p_targetRate;
	ret = gettimeofday(&p_rate->lastTimeStamp, NULL);
	if(ret != 0)
		return ret;
		
	return 0;
}

int sleepRate(rate_t* p_rate) 
{
	struct timeval diff;
	useconds_t diffUsec, intervalUsec;
	int ret;
	
	ret = gettimeofday(&diff, NULL);
	if(ret != 0)
		return ret;
		
	diff.tv_sec = diff.tv_sec - p_rate->lastTimeStamp.tv_sec;
	diff.tv_usec = diff.tv_usec - p_rate->lastTimeStamp.tv_usec;
	diffUsec = ((useconds_t) diff.tv_sec) * USEC_PER_SEC + diff.tv_usec;
	intervalUsec = USEC_PER_SEC / ((useconds_t) p_rate->targetRate);
	
	if(diffUsec < intervalUsec) {
		ret = usleep(intervalUsec - diffUsec);
		if(ret != 0)
			return ret;
	}
	
	return 0;
}