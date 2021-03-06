#include "TimeTaking.h"

struct timeElement inputExecutionTime;
struct timeElement interpreterExecutionTime;
struct timeElement simulationExecutionTime;
struct timeElement mapperExecutionTime;

struct timeElement inputReactionTime;
struct timeElement interpreterReactionTime;
struct timeElement simulationReactionTime;
struct timeElement mapperReactionTime;
struct timeElement totalReactionTime;

int initTimeTaking()
{
	inputExecutionTime.max = 0;
	interpreterExecutionTime.max = 0;
	simulationExecutionTime.max = 0;
	mapperExecutionTime.max = 0;
	
	inputReactionTime.max = 0;
	interpreterReactionTime.max = 0;
	simulationReactionTime.max = 0;
	mapperReactionTime.max = 0;
	totalReactionTime.max = 0;
	
	return 0;
}

int stopTimeTaking(struct timeElement *p_element)
{
	int ret = stopWatch(&p_element->watch);
	if(ret != 0)
		return ret;
	unsigned int tmp = getWatchMSec(&p_element->watch);
	if( tmp > p_element->max)
		p_element->max = tmp;
	
	return 0;
}

int saveTimesToFile(char *p_file)
{
	FILE *f = fopen(p_file, "w");
	if (f == NULL) {
		PRINT_ERR("Unable to open file: %s.\n", p_file);
		return -1;
	}
	
	fprintf(f, "### Measure Results ###\n");
	fprintf(f, "--- Execution Times ---\n");
	fprintf(f, "Input: %dms\n",inputExecutionTime.max);
	fprintf(f, "Interpreter: %dms\n",interpreterExecutionTime.max);
	fprintf(f, "Simulation: %dms\n",simulationExecutionTime.max);
	fprintf(f, "Mapper: %dms\n", mapperExecutionTime.max);
	fprintf(f, "--- Reaction Times ---\n");
	fprintf(f, "Input: %dms\n",inputReactionTime.max);
	fprintf(f, "Interpreter: %dms\n",interpreterReactionTime.max);
	fprintf(f, "Simulation: %dms\n",simulationReactionTime.max);
	fprintf(f, "Mapper: %dms\n", mapperReactionTime.max);
	fprintf(f, "Total: %dms\n", totalReactionTime.max);
	
	fclose(f);
	
	return 0;
}