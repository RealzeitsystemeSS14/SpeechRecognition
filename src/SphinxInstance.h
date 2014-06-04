#ifndef SPHINX_INSTANCE_H_
#define SPHINX_INSTANCE_H_

#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/cont_ad.h>

typedef struct {
	ps_decoder_t *psDecoder;
	ad_rec_t *audioDevice;
	cont_ad_t *contAudioDevice;
	cmd_ln_t *config;
} sphinxInstance_t;

int initSphinxInstance(sphinxInstance_t *p_sphinx, cmd_ln_t *p_config);
int closeSphinxInstance(sphinxInstance_t *p_sphinx);

int initSphinxRecord(sphinxInstance_t *p_sphinx);
int closeSphinxRecord(sphinxInstance_t *p_sphinx);

#endif