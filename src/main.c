#include <signal.h>
#include "InputThread.h"
#include "InterpreterThread.h"
#include "Utils.h"

struct sigaction sa;

volatile int run;

inputThread_t inputThread;
interpreterThread_t interpreterThread;
blockingQueue_t audioQueue;
blockingQueue_t hypQueue;

void sighandler(int sig)
{
    run = 0;
	char *poisonPill = malloc(sizeof(char) * 11);
	strcpy(poisonPill, "poisonPill");
	enqueueBlockingQueue(&hypQueue, poisonPill);
	stopInputThread(&inputThread);
	stopInterpreterThread(&interpreterThread);
}

void setSignalAction()
{
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = sighandler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

int main(int argc, char** argv)
{
	run = 1;
	char *hyp;
	cmd_ln_t *config;
	
	printf("Getting Config...\n");

    config = cmd_ln_init(NULL, ps_args(), TRUE,
                         "-hmm", MODELDIR "/hmm/en_US/hub4wsj_sc_8k",
                         "-lm", "1048.lm",
                         "-dict", "1048.dic",
						 "-logfn", "/dev/null",
                         //"-lm", MODELDIR "/lm/en/turtle.DMP",
                         //"-dict", MODELDIR "/lm/en/turtle.dic",
                         NULL);

    if (config == NULL) {
        PRINT_ERR("Error getting cmd config.\n");
        return -1;
    }
	
	printf("Init HypQueue...");
	if(initBlockingQueue(&hypQueue) != 0)
		return -1;
	printf(" [Done]\n");
	printf("Init AudioQueue...");
	if(initBlockingQueue(&audioQueue) != 0)
		return -2;
	printf(" [Done]\n");
	
	printf("Init InputThread...");
	if(initInputThread(&inputThread, &audioQueue) != 0)
		return -3;
	printf(" [Done]\n");
	printf("Init InterpreterThread...");
	if(initInterpreterThread(&interpreterThread, &audioQueue, &hypQueue, config) != 0)
		return -4;
	printf(" [Done]\n");
	
	printf("Start InputThread...\n");
	if(startInputThread(&inputThread) != 0)
		return -5;
	printf("Start InterpreterThread...\n");
	if(startInterpreterThread(&interpreterThread) != 0)
		return -6;
	
	sleep(1);
	while(run) {
		printf("Press return to record data.\n");
		getchar();
		startRecording(&inputThread);
		
		printf("Press RETURN to end recording.\n");
		getchar();
		stopRecording(&inputThread);
		
		printf("Waiting for hypothesis...\n");
		hyp = (char*) dequeueBlockingQueue(&hypQueue);
		printf("Received hypothesis: %s.\n", hyp);
		free(hyp);
	}
	
	joinInterpreterThread(&interpreterThread);
	joinInputThread(&inputThread);
	
	destroyInterpreterThread(&interpreterThread);
	destroyInputThread(&inputThread);
	
	destroyBlockingQueue(&audioQueue, 1);
	destroyBlockingQueue(&hypQueue, 1);
	
	return 0;
}