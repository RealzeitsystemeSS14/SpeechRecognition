#include <signal.h>
#include "InputThread.h"
#include "InterpreterThread.h"
#include "ButtonThread.h"
#include "CrashSimulationThread.h"
#include "HypothesisMapper.h"
#include "Utils.h"

#define HYP_QUEUE_SIZE 10
#define AUDIO_QUEUE_SIZE 10

struct sigaction sa;

blockingQueue_t audioQueue;
blockingQueue_t hypQueue;

inputThread_t inputThread;
interpreterThread_t interpreterThread;
buttonThread_t buttonThread;
crashSimulationThread_t simulationThread;
hypothesisMapper_t hypMapper;

void sighandler(int sig)
{
	//TODO
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

static int init()
{
	cmd_ln_t *config;
	
	err_set_logfp(fopen("/dev/null", "w"));
	PRINT_INFO("Getting Config...\n");

    config = cmd_ln_init(NULL, ps_args(), TRUE,
                         "-hmm", MODELDIR "/hmm/en_US/hub4wsj_sc_8k",
                         "-lm", "0583.lm",
                         "-dict", "0583.dic",
                         NULL);

    if (config == NULL) {
        PRINT_ERR("Error getting cmd config.\n");
        return -1;
    }
	
	PRINT_INFO("Init HypQueue...");
	fflush(stdout);
	if(initBlockingQueue(&hypQueue, HYP_QUEUE_SIZE) != 0)
		return -1;
	PRINT_INFO(" [Done]\n");
	
	PRINT_INFO("Init AudioQueue...");
	fflush(stdout);
	if(initBlockingQueue(&audioQueue, AUDIO_QUEUE_SIZE) != 0)
		return -2;
	PRINT_INFO(" [Done]\n");
	
	PRINT_INFO("Init InputThread...");
	fflush(stdout);
	if(initInputThread(&inputThread, &audioQueue) != 0)
		return -3;
	PRINT_INFO(" [Done]\n");
	
	PRINT_INFO("Init InterpreterThread...");
	fflush(stdout);
	if(initInterpreterThread(&interpreterThread, &audioQueue, &hypQueue, config) != 0)
		return -4;
	PRINT_INFO(" [Done]\n");
	
	PRINT_INFO("Init ButtonThread...");
	fflush(stdout);
	if(initButtonThread(&buttonThread, &inputThread) != 0)
		return -4;
	PRINT_INFO(" [Done]\n");
	
	PRINT_INFO("Init SimulationThread...");
	fflush(stdout);
	if(initCrashSimulationThread(&simulationThread) != 0)
		return -5;
	PRINT_INFO(" [Done]\n");
	
	PRINT_INFO("Init HypothesisMapper...");
	fflush(stdout);
	if(initHypothesisMapper(&hypMapper, &hypQueue, &simulationThread) != 0)
		return -6;
	PRINT_INFO(" [Done]\n");
	
	return 0;
}

static int start()
{
	PRINT_INFO("Start InputThread...\n");
	if(startInputThread(&inputThread) != 0)
		return -20;
	PRINT_INFO("Start InterpreterThread...\n");
	if(startInterpreterThread(&interpreterThread) != 0)
		return -21;
	PRINT_INFO("Start ButtonThread...\n");
	if(startButtonThread(&buttonThread) != 0)
		return -22;
	PRINT_INFO("Start SimulationThread...\n");
	if(startCrashSimulationThread(&simulationThread) != 0)
		return -23;
		
	return 0;
}

static void join()
{
	joinCrashSimulationThread(&simulationThread);
	joinButtonThread(&buttonThread);
	joinInterpreterThread(&interpreterThread);
	joinInputThread(&inputThread);
}

static void destroy()
{
	destroyCrashSimulationThread(&simulationThread);
	destroyButtonThread(&buttonThread);
	destroyInterpreterThread(&interpreterThread);
	destroyInputThread(&inputThread);
	
	destroyBlockingQueue(&audioQueue, 1);
	destroyBlockingQueue(&hypQueue, 1);
}

int main(int argc, char** argv)
{
	int ret;
	
	ret = init();
	if(ret != 0)
		return ret;
	
	ret = start();
	if(ret != 0)
		return ret;
	
	
	loopHypothesisMapper(&hypMapper);
	
	join();
	destroy();
	
	return 0;
}