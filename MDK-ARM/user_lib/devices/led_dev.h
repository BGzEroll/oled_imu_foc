#ifndef LED_DEV_H
#define LED_DEV_H

#include "drivers/led.h"
#include "main.h"

extern led green_led;
extern led blue_led;

void green_led_proc(uint32_t tick);
void blue_led_proc(uint32_t tick);

#endif
