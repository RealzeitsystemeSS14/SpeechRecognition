#include "TimeTaking.h"

#define TIME_TAKING_NAME "Sim"
static stopWatch_t watch;
static unsigned int max;

void initTimeTaking()
{
	max = 0;
}

void restartTimeTaking()
{
	resetWatch(&watch);
	if(startWatch(&watch) != 0)
		PRINT_ERR("Failed to start watch for time taking.\n");
}

void holdTimeTaking()
{
	if(stopWatch(&watch) != 0)
		PRINT_ERR("Failed to stop watch for time taking.\n");
}
void resumeTimeTaking()
{
	if(startWatch(&watch) != 0)
		PRINT_ERR("Failed to start watch for time taking.\n");
}

void stopTimeTaking()
{
	if(stopWatch(&watch) != 0)
		PRINT_ERR("Failed to stop watch for time taking.\n");
	unsigned int tmp = getWatchMSec(&watch);
	if(tmp > max)
		max = tmp;
}

void saveTimeToFile(char *p_file)
{
	FILE *f = fopen(p_file, "w");
	if (f == NULL) {
		PRINT_ERR("Unable to open file: %s.\n", p_file);
		return;
	}
	
	fprintf(f, "### Time taking results\n");
	fprintf(f, "%s: %dms\n",TIME_TAKING_NAME, max);
	
	fclose(f);
	
}