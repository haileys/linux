#pragma once
#include "prelude.h"

typedef struct Mutex* HMUTEX;

#define MUTEX_MUST_COMPLETE 0x01
HMUTEX VMM_CreateMutex(long boost, unsigned long flags);

long VMM_DestroyMutex(HMUTEX mutex);
void VMM_EnterMutex(HMUTEX mutex, unsigned long flags);
void VMM_LeaveMutex(HMUTEX mutex);

void VMM_SignalID(uint32_t id);
void VMM_BlockOnID(uint32_t id);
