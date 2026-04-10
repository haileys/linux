#pragma once
#include "prelude.h"

typedef struct VMM_TCB* VMM_THREAD_HANDLE;

void VMMTerminateThread(VMM_THREAD_HANDLE thread);
