#ifndef TIME_TAKING_H
#define TIME_TAKING_H

#include "Utils.h"

void initTimeTaking();
void restartTimeTaking();
void holdTimeTaking();
void resumeTimeTaking();
void stopTimeTaking();
void saveTimeToFile(char *p_file);

#endif