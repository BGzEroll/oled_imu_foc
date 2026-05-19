#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f4xx_hal.h"
#include "drivers/foc/foc_deps.h"

extern foc_motor motor_1;

void motor_init();

#endif
