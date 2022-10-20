#pragma once
#ifndef AUDIO_H
#define AUDIO_H
#include <opus/opusfile.h>
#include "common.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

const char* opusStrError(int);
bool audioInit(void);
void audioExit(void);
void audioCallback(void* const);
void audioThread(void* const);

#endif