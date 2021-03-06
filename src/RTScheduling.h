/* This component provides several intialization functions to
 * create realtime mutexes and threads.
 * All functions return 0 on success. */

#ifndef RT_SCHEDULING_H
#define RT_SCHEDULING_H

#include <pthread.h>

#define INPUT_PRIORITY 96
#define INTERPRETER_PRIORITY 97
#define SIMULATION_PRIORITY 98
#define MAPPER_PRIORITY 99

// stack must at least be 16384 (1 page)
// stack must be multiple of system page size
#define INPUT_STACKSIZE (2 * 16384)
#define INTERPRETER_STACKSIZE (2 * 16384)
#define SIMULATION_STACKSIZE (4 * 16384)
#define BUTTON_STACKSIZE (16384)

int initRTCurrentThread(int p_priority);
int initRTThreadAttr(pthread_attr_t *p_attributes, unsigned int p_stackSize, int p_priority);
int initRTMutexAttr(pthread_mutexattr_t *p_attributes);

#endif