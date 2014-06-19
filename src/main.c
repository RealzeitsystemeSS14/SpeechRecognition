#include <signal.h>
#include <allegro.h>
#include "InputThread.h"
#include "InterpreterThread.h"
#include "ButtonThread.h"
#include "CrashSimulationThread.h"
#include "HypothesisMapper.h"
#include "TimeTaking.h"
#include "Utils.h"

struct sigaction sa;

static blockingQueue_t audioQueue;
static blockingQueue_t hypQueue;

static inputThread_t inputThread;
static interpreterThread_t interpreterThread;
static buttonThread_t buttonThread;
static crashSimulationThread_t simulationThread;
static hypothesisMapper_t hypMapper;

static volatile int initialized = 0;
static cmd_ln_t *config;

void sighandler(int sig)
{
	PRINT_INFO("Shutting down...\n");
	if(initialized)
		stopHypothesisMapper(&hypMapper);
	else {
		PRINT_INFO("[Shutdown]\n");
		exit(0);
	}
}

void setSignalAction()
{
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = sighandler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
}

static void closeWindow()
{
	sighandler(0);
}

static int init()
{
	PRINT_INFO("Initializing allegro...\n");
	allegro_init();
	install_timer();
	set_close_button_callback(closeWindow);
	// have to be after allegro init because allegro initializes its own sighandler
	setSignalAction();
	
	err_set_logfp(fopen("/dev/null", "w"));
	PRINT_INFO("Getting Config...\n");

    config = cmd_ln_init(NULL, ps_args(), TRUE,
                         "-hmm", MODELDIR "/hmm/en_US/hub4wsj_sc_8k",
                         "-lm", "6706.lm",
                         "-dict", "6706.dic",
						 "-ds", "2",
						 "-topn", "2",
						 "-maxwpf", "5",
						 "-maxhmmpf", "3000",
						 "-kdmaxdepth", "5",
						 "-kdmaxbbi", "16",
                         NULL);

    if (config == NULL) {
        PRINT_ERR("Error getting cmd config.\n");
        return -1;
    }
	
	PRINT_INFO("Init AudioBufferPool...\n");
	if(initAudioBufferPool() != 0)
		return -1;
	
	PRINT_INFO("Init StringPool...\n");
	if(initStringPool() != 0)
		return -1;
	
	PRINT_INFO("Init HypQueue...\n");
	if(initBlockingQueue(&hypQueue) != 0)
		return -1;
	
	PRINT_INFO("Init AudioQueue...\n");
	if(initBlockingQueue(&audioQueue) != 0)
		return -1;
	
	PRINT_INFO("Init InterpreterThread...\n");
	if(initInterpreterThread(&interpreterThread, &audioQueue, &hypQueue, config) != 0)
		return -1;
	
	PRINT_INFO("Init InputThread...\n");
	if(initInputThread(&inputThread, &audioQueue) != 0)
		return -1;
	
	PRINT_INFO("Init ButtonThread...\n");
	if(initButtonThread(&buttonThread, &inputThread, closeWindow) != 0)
		return -1;
	
	PRINT_INFO("Init SimulationThread...\n");
	if(initCrashSimulationThread(&simulationThread) != 0)
		return -1;
	
	PRINT_INFO("Init HypothesisMapper...\n");
	if(initHypothesisMapper(&hypMapper, &hypQueue, &simulationThread) != 0)
		return -1;
		
	PRINT_INFO("[Initialized]\n");
	
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
static void stop()
{
	PRINT_INFO("Stopping threads...\n");
	stopCrashSimulationThread(&simulationThread);
	stopButtonThread(&buttonThread);
	stopInterpreterThread(&interpreterThread);
	stopInputThread(&inputThread);
}

static void join()
{
	PRINT_INFO("Joining threads...\n");
	joinCrashSimulationThread(&simulationThread);
	joinButtonThread(&buttonThread);
	joinInterpreterThread(&interpreterThread);
	joinInputThread(&inputThread);
	PRINT_INFO("[Joined threads]\n");
}

static void save() 
{
	PRINT_INFO("Saving results...\n");
	saveTimesToFile("times.txt");
	PRINT_INFO("[Saved]\n");
}

static void destroy()
{
	destroyCrashSimulationThread(&simulationThread);
	destroyButtonThread(&buttonThread);
	destroyInterpreterThread(&interpreterThread);
	destroyInputThread(&inputThread);
	
	destroyBlockingQueue(&audioQueue, 0);
	destroyBlockingQueue(&hypQueue, 0);
	
	cmd_ln_free_r(config);
	
	remove_timer();
	allegro_exit();
	
	PRINT_INFO("[Destroyed]\n");
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
	
	initialized = 1;
	loopHypothesisMapper(&hypMapper);
	
	stop();
	join();
	save();
	destroy();
	
	PRINT_INFO("[Shutdown]\n");
	
	return 0;
}