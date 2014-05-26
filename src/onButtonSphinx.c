#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/cont_ad.h>

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

#define BUFFER_SIZE 4096

volatile int exitApp;
volatile int run;

pthread_t thread;
pthread_barrier_t startBarrier;
pthread_barrier_t endBarrier;

ps_decoder_t *ps;
ad_rec_t *ad;
cont_ad_t *c_ad;
cmd_ln_t *config;

struct sigaction sa;

int printResult()
{
    char const *hyp, *uttid;
    int32 score;

    hyp = ps_get_hyp(ps, &score, &uttid);
    if (hyp == NULL) {
        printf("Error getting result.\n");
        return 0;
    }

    printf("Result => %s, uttid: %s, score: %d\n", hyp, uttid, score);

    if (strcmp(hyp, "left") == 0)
        return 2;

    return 1;
}

int sphinxInit()
{
    config = cmd_ln_init(NULL, ps_args(), TRUE,
                         "-hmm", MODELDIR "/hmm/en_US/hub4wsj_sc_8k",
                         "-lm", "1048.lm",
                         "-dict", "1048.dic",
                         //"-lm", MODELDIR "/lm/en/turtle.DMP",
                         //"-dict", MODELDIR "/lm/en/turtle.dic",
                         NULL);

    if (config == NULL) {
        printf("Error getting cmd config.\n");
        return 0;
    }

    ps = ps_init(config);
    if (ps == NULL) {
        printf("Error calling init.\n");
        return 0;
    }

    return 1;
}

int sphinxClose()
{
    if (ps_free(ps) != 0) {
        printf("Error freeing pocketsphinx.\n");
        return 0;
    }

    return 1;
}

int recordInit()
{
    ad = ad_open();
    if (ad == NULL) {
        printf("Error opening recording device.\n");
        return 0;
    }

    c_ad = cont_ad_init(ad, ad_read);
    if (c_ad == NULL) {
        printf("Error initializing continues ad.\n");
        return 0;
    }

    if (ad_start_rec(ad) < 0) {
        printf("Error starting recording.\n");
        return 0;
    }

    if (cont_ad_calib(c_ad) < 0) {
        printf("Error calibrating continues ad.\n");
        return 0;
    }

    ad_stop_rec(ad);

    return 1;
}

int recordClose()
{
    cont_ad_close(c_ad);
    ad_close(ad);

    return 1;
}

int decode()
{
    int ret;
    int16 buf[BUFFER_SIZE];
    
    printf("Listening for input...\n");
    
    if (ad_start_rec(ad) < 0) {
        printf("Error starting recording.\n");
        return 0;
    }
    
    if (ps_start_utt(ps, NULL) < 0) {
        printf("Failed to start utterance.\n");
        return 0;
    }
    
    while(run)
    {
        ret = cont_ad_read(c_ad, buf, BUFFER_SIZE);

        if (ret < 0) {
            printf("Failed to record audio.\n");
            return 0;
        } else if(ret > 0) {
            // Valid speech data read.
            ret = ps_process_raw(ps, buf, 4096, 0, 0);
            if (ret < 0) {
                printf("Error decoding.\n");
                return 0;
            }
        } else {
            //no data
            usleep(50000);
        }
    }
    
    ad_stop_rec(ad);
    while (ad_read(ad, buf, BUFFER_SIZE) >= 0);
    cont_ad_reset(c_ad);

    ps_end_utt(ps);
    return 1;
}

void * listenThread(void *arg)
{
    printf("Listen Thread: init sphinx...\n");
    if(!sphinxInit())
        exit(-1);
    
    if(!recordInit())
        exit(-1);
    
    while(!exitApp)
    {
        if(run)
        {
            printf("Listen Thread: starting to decode...\n");
            pthread_barrier_wait(&startBarrier);
            if(!decode())
                exitApp = 1;
            printf("Listen Thread: decoding finished...\n");
            
            if (printResult() == 2)
                exitApp = 1;
            
            pthread_barrier_wait(&endBarrier);
        }
        
        usleep(50000);
    }
    
    recordClose();
    sphinxClose();
    
    printf("Listen thread terminated.\n");
    return NULL;
}

void startThread()
{
    printf("Starting listen thread...\n");
    exitApp = 0;
    run = 0;
    pthread_barrier_init(&startBarrier, NULL, 2);
    pthread_barrier_init(&endBarrier, NULL, 2);
    pthread_create(&thread, NULL, listenThread, NULL);
}

void joinThread() 
{
    void *res;
    printf("Joining listen thread...\n");
    pthread_join(thread, &res);
}

void sighandler(int sig)
{
    run = 0;
    exitApp = 1;
    exit(0);
}

void setSignalAction()
{
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = sighandler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

int main(int argc, char **argv)
{
    char buf[10];
    setSignalAction();
    startThread();

    while (!exitApp) {
        
        printf("Press return to start recording.\n");
        getchar();
        run = 1;
        pthread_barrier_wait(&startBarrier);
        
        printf("Press return to end recording.\n");
        getchar();
        run = 0;
        pthread_barrier_wait(&endBarrier);
        
        printf("Recording stopped.\n");
    }
    
    joinThread();

    return 0;
}


