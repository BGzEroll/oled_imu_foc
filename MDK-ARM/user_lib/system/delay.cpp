#include "delay.h"

bool delay::is_init = false;
uint32_t delay::last_system_core_clock = 0;
uint32_t delay::ticks_per_us = 0;
