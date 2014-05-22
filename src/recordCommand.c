#include <pocketsphinx/pocketsphinx.h>
#include <sphinxbase/cont_ad.h>
#include <stdio.h>
#include <signal.h>

#define FATAL_ERROR(msg, code) fprintf(stderr, msg + "\n"); exit(code)
#define AUDIO_DEVICE ""
#define SAMPLING_RATE 1
#define BUFFER_SIZE 4096

static ps_decoder_t *psDecoder;
static cont_ad_t *continousAudoDevice;
static ad_rec_t *audioDevice;;
static int run;

int init();
int processCommands();
int32 waitForNextUtterance();
void record();

static void sighandler(int signo)
{
    printf("Aborting voice recognition...\n");
    run = 0;
}

int main(int arc, char **argv)
{
    run = 1;
    int ret = 0;
    
    ret = init();
    if(ret != 0)
        return ret;
    
    signal(SIGINT, &sighandler);
    
    ret = processCommands();
    if(ret != 0)
        return ret;
    
    ps_free(psDecoder);
    
    return 0;
}

int init()
{
    cmd_ln_t *config;
    
    printf("Initializing audio device...\n");
    
    config = cmd_ln_init(NULL, ps_args(), TRUE, 
                             "-hmm", MODELDIR "/hmm/en_US/hub4wsj_sc_8k",
                             "-lm", MODELDIR "/lm/en/turtle.DMP",
                             "-dict", MODELDIR "/lm/en/turtle.dic",
                             NULL);
    if(config == NULL) {
        fprintf(stderr, "Could not init sphinx config.\n");
        return 1;
    }
    
    psDecoder = ps_init(config);
    if(psDecoder == NULL) {
        fprintf(stderr, "Could not init psDecoder.\n");
        return 2;
    }
    
    audioDevice = ad_open();
    if(audioDevice == NULL) {
        fprintf(stderr, "Could not open Audio Device.\n");
        return 3;
    }
    continousAudoDevice = cont_ad_init(audioDevice, ad_read);
    if(continousAudoDevice == NULL) {
        fprintf(stderr, "Could not open Audio Device.\n");
        return 4;
    }
    
    if(ad_start_rec(audioDevice) < 0) {
        fprintf(stderr,"Failed to start recording.\n");
        return 5;
    }
    
    if(cont_ad_calib(continousAudoDevice) < 0) {
        fprintf(stderr,"Failed to calibrate voice activity detection.\n");
        return 6;
    }
    
    return 0;
}

int processCommands()
{
    int32 samples;
    int16 audioBuf[BUFFER_SIZE];
    char const *uttid;
    char const *hyp;
    
    while(run)
    {
        printf("Waiting for utterance...\n");
        samples = waitForNextUtterance();
        if(samples < 0)
            return -1;
        
        if(ps_start_utt(psDecoder, NULL) < 0) {
            fprintf(stderr, "Failed to start next utterance\n");
            return -1;
        }
        ps_process_raw(psDecoder, audioBuf, samples, FALSE, FALSE);
        
        printf("Recording...\n");
        fflush(stdout);
        record();
        
        ad_stop_rec(audioDevice);
        while(ad_read(audioDevice, audioBuf, BUFFER_SIZE) >= 0);
        cont_ad_reset(continousAudoDevice);
        ps_end_utt(psDecoder);
        
        hyp = ps_get_hyp(psDecoder, NULL, &uttid);
        printf("Heard: %s\n", hyp);
        
        if (ad_start_rec(audioDevice) < 0) {
            fprintf(stderr, "Failed to start audio device.\n");
            return -1;
        }
    }
    
    return 0;
}

int32 waitForNextUtterance()
{
    int32 result;
    int16 audioBuf[BUFFER_SIZE];
    result = cont_ad_read(continousAudoDevice, audioBuf, BUFFER_SIZE);
    
    while(result == 0)
    {
        usleep(10000);
        result = cont_ad_read(continousAudoDevice, audioBuf, BUFFER_SIZE);
    }
    
    if(result < 0) {
        fprintf(stderr, "Failed to read audio by waiting for next utterance.\n");
    }
    
    return result;
}

void record()
{
    int32 samples, timeStamp, rem;
    int16 audioBuf[BUFFER_SIZE];
    
    timeStamp = continousAudoDevice->read_ts;
    while(run)
    {
        samples = cont_ad_read(continousAudoDevice, audioBuf, BUFFER_SIZE);
        if (samples == 0) {
            if ((continousAudoDevice->read_ts - timeStamp) > DEFAULT_SAMPLES_PER_SEC)
                break;
        } else {
            timeStamp = continousAudoDevice->read_ts;
        }
        
        rem = ps_process_raw(psDecoder, audioBuf, samples, FALSE, FALSE);
        
        if ((rem == 0) && (samples == 0))
            usleep(20000);
    }
}
