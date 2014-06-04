#include "SphinxInstance.h"

int initSphinxInstance(sphinxInstance_t *p_sphinx, cmd_ln_t *p_config)
{
    p_sphinx->config = p_config;
	p_sphinx->psDecoder = ps_init(p_sphinx->config);
    if (p_sphinx->psDecoder == NULL)
        return -2;

    return 0;
}

int closeSphinxInstance(sphinxInstance_t *p_sphinx)
{
	int ret = ps_free(p_sphinx->psDecoder);
    if (ret != 0)
        return ret;

    return 0;
}

int initSphinxRecord(sphinxInstance_t *p_sphinx)
{
	int ret;
    p_sphinx->audioDevice = ad_open();
    if (p_sphinx->audioDevice == NULL)
        return -1;

    p_sphinx->contAudioDevice = cont_ad_init(p_sphinx->audioDevice, ad_read);
    if (p_sphinx->contAudioDevice == NULL)
        return -2;

	ret = ad_start_rec(p_sphinx->audioDevice);
    if (ret < 0)
        return ret;
	
	ret = cont_ad_calib(p_sphinx->contAudioDevice);
    if (ret < 0)
        return ret;

    ad_stop_rec(p_sphinx->audioDevice);

    return 0;
}

int closeSphinxRecord(sphinxInstance_t *p_sphinx)
{
    cont_ad_close(p_sphinx->contAudioDevice);
    ad_close(p_sphinx->audioDevice);

    return 0;
}