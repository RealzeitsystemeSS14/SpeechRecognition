#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/cont_ad.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 4096

ps_decoder_t *ps;
ad_rec_t *ad;
cont_ad_t *c_ad;
cmd_ln_t *config;

static const arg_t cont_args_def[] = {
    POCKETSPHINX_OPTIONS,
    { "-df",
      ARG_STRING,
      NULL,
      "Device file of audio device." },
    CMDLN_EMPTY_OPTION
};

int decode()
{
    int ret;
    int16 buf[BUFFER_SIZE];
    int32 ts;

    printf("Start listening ...\n");

    if (ad_start_rec(ad) < 0) {
        printf("Error starting recording.\n");
        return 0;
    }

    // Check if not silent
    while ((ret = cont_ad_read(c_ad, buf, BUFFER_SIZE)) == 0)
        usleep(1000);

    if (ret < 0) {
        printf("Failed to record audio.\n");
        return 0;
    }

    ts = c_ad->read_ts;

    // Audio recorded
    printf("New audio recorded!\n");

    if (ps_start_utt(ps, NULL) < 0) {
        printf("Failed to start utterance.\n");
        return 0;
    }

    ret = ps_process_raw(ps, buf, 4096, 0, 0);
    if (ret < 0) {
        printf("Error decoding.\n");
        return 0;
    }

    // Now check if silence data is read.
    while (1) {
        ret = cont_ad_read(c_ad, buf, BUFFER_SIZE);

        if (ret < 0) {
            printf("Failed to record audio.\n");
            return 0;
        }

        if (ret == 0) {
            // Check if 1 second is elapsed.
            if ((c_ad->read_ts - ts) >= DEFAULT_SAMPLES_PER_SEC/4)
                break;
            else {
                usleep(1000);
                continue;
            }
        }

        // Valid speech data read.

        ts = c_ad->read_ts;

        ret = ps_process_raw(ps, buf, 4096, 0, 0);
        if (ret < 0) {
            printf("Error decoding.\n");
            return 0;
        }
    }

    // Flush input device.
    ad_stop_rec(ad);
    while (ad_read(ad, buf, BUFFER_SIZE) >= 0);
    cont_ad_reset(c_ad);

    ps_end_utt(ps);
    return 1;
}

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
    config = cmd_ln_init(NULL, cont_args_def, TRUE,
                         "-hmm", MODELDIR "/hmm/en_US/hub4wsj_sc_8k",
                         "-lm", "1048.lm",
                         "-dict", "1048.dic",
                         "-df", "/dev/dsp1",
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
    ad = ad_open_dev(cmd_ln_str_r(config, "-df"), (int)cmd_ln_float32_r(config, "-samprate"));
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
    //ad_stop_rec(ad); // TODO: Should be made in decode function.
    ad_close(ad);

    return 1;
}

int main(int argc, char **argv)
{
    sphinxInit();
    recordInit();

    while (1) {
        decode();

        if (printResult() == 2)
            break;
    }

    recordClose();
    sphinxClose();

    return 0;
}
