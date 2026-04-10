#pragma once
#include "prelude.h"

// Retrieves the number of real time clock ticks that have elapsed since
// the current Windows session was started based on a unit of 0.8 usec
uint64_t VTD_Get_Real_Time(void);
static const uint32_t WIN9X_REAL_CLOCK_HZ = 1250000;
static const uint64_t WIN9X_NSEC_PER_REAL_CLOCK = 800;

// Returns wall time in millis since Jan 1 *1980*
uint64_t VTD_Get_Date_And_Time(void);
static const uint64_t WIN9X_WALL_CLOCK_EPOCH = 315532800;

typedef struct VMM_TimeOut* VMM_TIMEOUT_HANDLE;

// Define timeout callback as fastcall - this tells GCC to use ECX and EDX
// for the first two arguments, which is exactly the calling convention
// required for this callback.
#define _vmm_timer_callback __attribute__((fastcall))
typedef void _vmm_timer_callback(*VMM_TimeOutCallback)(uint32_t millis_late, void* ctx);

VMM_TIMEOUT_HANDLE VMM_Set_Async_Time_Out(uint32_t millis, void* ctx, VMM_TimeOutCallback callback);

void VMM_Cancel_Time_Out(VMM_TIMEOUT_HANDLE handle);
