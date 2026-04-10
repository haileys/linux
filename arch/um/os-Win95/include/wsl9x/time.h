#pragma once
#include "prelude.h"

/* returns time since boot in milliseconds */
uint32_t VMM_Get_System_Time(void);

// Retrieves the number of real time clock ticks that have elapsed since
// the current Windows session was started based on a unit of 0.8 usec
uint64_t VTD_Get_Real_Time(void);
static const uint32_t WIN9X_REAL_CLOCK_HZ = 1250000;
static const uint64_t WIN9X_NSEC_PER_REAL_CLOCK = 800;

// Returns wall time in millis since Jan 1 *1980*
uint64_t VTD_Get_Date_And_Time(void);
static const uint64_t WIN9X_WALL_CLOCK_EPOCH = 315532800;
