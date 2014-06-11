/* The AudioBuffer is used to save audio data, to be enqeued in a BlockingQueue.
 * It transports the audio data between threads.
 * initAudioBuffer() return 0 on success.
 * addAudioBuffer() returns 0 on success, else the remaining bytes, which have not been written
 * into the AudioBuffer.*/

#ifndef AUDIO_BUFFER_H_
#define AUDIO_BUFFER_H_

#include <stdint.h>

#define AUDIO_BUFFER_MAX_SIZE 8192

typedef struct {
	int16_t buffer[AUDIO_BUFFER_MAX_SIZE];
	unsigned int size;
} audioBuffer_t;

int initAudioBuffer(audioBuffer_t *p_buffer);
unsigned int addAudioBuffer(audioBuffer_t *p_buffer, int16_t *p_buf, unsigned int p_count);
void clearAudioBuffer(audioBuffer_t *p_buffer);
int isFullAudioBuffer(audioBuffer_t *p_buffer);

#endif