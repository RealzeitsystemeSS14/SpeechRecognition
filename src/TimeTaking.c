#include "TimeTaking.h"
#include "Utils.h"

struct timeElement inputTime;
struct timeElement interpreterTime;
struct timeElement simulationTime;
struct timeElement globalTime;
struct timeElement mapperTime;

int initTimeTaking()
{
	inputTime.maxMsec = 0;
	interpreterTime.maxMsec = 0;
	simulationTime.maxMsec = 0;
	globalTime.maxMsec = 0;
	
	return 0;
}

int startTimeTaking(struct timeElement *p_element)
{
	return startWatch(&p_element->watch);
}

int stopTimeTaking(struct timeElement *p_element)
{
	int ret = stopWatch(&p_element->watch);
	if(ret != 0)
		return ret;
	useconds_t tmp = getWatchMSec(&p_element->watch);
	if( tmp > p_element->maxMsec)
		p_element->maxMsec = tmp;
	
	return 0;
}

int saveTimesToFile(char *p_file)
{
	FILE *f = fopen(p_file, "w");
	if (f == NULL) {
		PRINT_ERR("Unable to open file: %s.\n", p_file);
		return -1;
	}
	
	fprintf(f, "Input: %dms\n",inputTime.maxMsec);
	fprintf(f, "Interpreter: %dms\n",interpreterTime.maxMsec);
	fprintf(f, "Simulation: %dms\n",simulationTime.maxMsec);
	fprintf(f, "Global: %dms\n",globalTime.maxMsec);
	fprintf(f, "Mapper: %dms\n", mapperTime.maxMsec);
	
	fclose(f);
	
	return 0;
}