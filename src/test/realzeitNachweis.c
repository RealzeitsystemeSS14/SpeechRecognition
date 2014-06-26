#include "iniparser.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define T_E_MAX "tEMax"
#define T_D_MAX "tDMax"
#define T_P_MIN "tPMin"
#define T_B "tB"
#define PRIORITY "prio"
#define MAX_KEY_SIZE 100

#define INT_TO_BOOL(a) ((a) != 0  ? "TRUE" : "FALSE")

#define MIN(a,b) ((a) < (b) ? (a) : (b))

typedef struct {
	double tEMax, tDMax, tPMin, tB;
	int prio;
	char secname[100];
	double hinreichend, notwendig;
} iniTask;

static dictionary *file;
static int taskCount;
static iniTask *tasks;

void parseFile()
{
	char key[MAX_KEY_SIZE];
	int i;
	taskCount = iniparser_getnsec(file);
	tasks = (iniTask*) malloc(sizeof(iniTask) * taskCount);
	
	for(i = 0; i < taskCount; ++i) {
		sprintf(tasks[i].secname, "%s", iniparser_getsecname(file, i));
		
		sprintf(key, "%s:%s", tasks[i].secname, T_E_MAX);
		tasks[i].tEMax = iniparser_getdouble(file, key, 0);
		
		sprintf(key, "%s:%s", tasks[i].secname, T_D_MAX);
		tasks[i].tDMax = iniparser_getdouble(file, key, 0);
		
		sprintf(key, "%s:%s", tasks[i].secname, T_P_MIN);
		tasks[i].tPMin = iniparser_getdouble(file, key, 0);
		
		sprintf(key, "%s:%s", tasks[i].secname, T_B);
		tasks[i].tB = iniparser_getdouble(file, key, 0);
		
		sprintf(key, "%s:%s", tasks[i].secname, PRIORITY);
		tasks[i].prio = iniparser_getint(file, key, 0);
	}
}

double hinreichenderTestOR()
{
	int i;
	double sum = 0;
	
	for(i = 0; i < taskCount; ++i)
		sum += tasks[i].tEMax / MIN(tasks[i].tDMax, tasks[i].tPMin);
	
	return sum;
	
}

void hinreichenderTestMR()
{
	int i;
	double tUsageSum = hinreichenderTestOR();
	for(i = 0; i < taskCount; ++i)
		tasks[i].hinreichend = (tasks[i].tB / MIN(tasks[i].tDMax, tasks[i].tPMin)) + tUsageSum;
}

double getHinreichendGrenze()
{
	return ((double) taskCount) * (pow(2, 1.0 / (double) (taskCount)) - 1);
}

void notwendigerTestMR()
{
	int i,j;
	
	for(i = 0; i < taskCount; ++i) {
		double tl = 0;
		double tl1;
		
		// Anfangswert ist die Summe tEMax aller gleich- / höherprioren Tasks
		for(j = 0; j < taskCount; ++j) {
			if(tasks[j].prio >= tasks[i].prio)
				tl += tasks[j].tEMax;
		}
		
		while(1)  {
			// formel aus skript
			tl1 = ceil(tl / tasks[i].tPMin) * tasks[i].tB;
			for(j = 0; j < taskCount; ++j) {
				if(tasks[j].prio >= tasks[i].prio)
					tl1 += ceil(tl / tasks[j].tPMin) * tasks[j].tEMax;
			}
			
			if(tl == tl1)
				break;
			else 
				tl = tl1;
		}
		
		tasks[i].notwendig = tl;
	}
}

void printResults()
{
	int i;
	double hinreichendGrenze = getHinreichendGrenze();
	for(i = 0; i < taskCount; ++i) {
		printf("%s:\n", tasks[i].secname);
		printf("-- Hinreichend: %.4f <= %.4f %s\n", tasks[i].hinreichend, hinreichendGrenze, INT_TO_BOOL(tasks[i].hinreichend <= hinreichendGrenze));
		printf("-- Notwendig:   %.4f <= %.4f %s\n", tasks[i].notwendig, tasks[i].tDMax, INT_TO_BOOL(tasks[i].notwendig <= tasks[i].tDMax));
	}
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		fprintf(stderr, "Gib die scheiß Datei an!\n");
		return -1;
	}
	
	file = iniparser_load(argv[1]);
	
	parseFile();
	hinreichenderTestMR();
	notwendigerTestMR();
	printResults();
	
	free(tasks);
	iniparser_freedict(file);
	
	return 0;
}