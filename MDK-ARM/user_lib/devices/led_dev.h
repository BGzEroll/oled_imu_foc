#ifndef LED_DEV_H
#define LED_DEV_H

#include "drivers/led.h"

extern led green_led;
extern led blue_led;

void green_led_proc(uint32_t tick);
void blue_led_proc(uint32_t tick);

#endif
