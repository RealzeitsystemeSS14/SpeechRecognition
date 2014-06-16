/* The AudioBufferPool reserves memory at its loading time.
 * Its AudioBuffer elements can be accessed with the function getAudioBuffer().
 * The pool is fixed size, hence if all elements are reserved the caller of
 * getAudioBuffer() will block.
 * Use releaseAudioBuffer() to mark a AudioBuffer as free again. */

#ifndef AUDIO_BUFFER_POOL_H
#define AUDIO_BUFFER_POOL_H

#include "AudioBuffer.h"

int initAudioBufferPool();
int destroyAudioBufferPool();

/* This function can block if no buffer is available */
audioBuffer_t* reserveAudioBuffer();
void releaseAudioBuffer(audioBuffer_t *p_audioBuffer);

#endif