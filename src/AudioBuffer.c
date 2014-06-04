#include "AudioBuffer.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))

int initAudioBuffer(audioBuffer_t *p_buffer)
{
	p_buffer->size = 0;
	return 0;
}

unsigned int addAudioBuffer(audioBuffer_t *p_buffer, int16_t *p_buf, unsigned int p_count)
{
	int i;
	unsigned int toCopy = MIN(AUDIO_BUFFER_MAX_SIZE - p_buffer->size, p_count);
	for(i = 0; i < toCopy; ++i) {
		p_buffer->buffer[p_buffer->size + i] = p_buf[i];
	}
	p_buffer->size += toCopy;
	
	return toCopy;
}

void clearAudioBuffer(audioBuffer_t *p_buffer)
{
	p_buffer->size = 0;
}