#ifndef LED_H
#define LED_H

#include "stm32f4xx_hal.h"

class led
{
    public:
        led(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState on_level);

    public:
        void on();
        void off();
        void toggle();

    private:
        GPIO_TypeDef *port;
        uint16_t pin;
        GPIO_PinState on_level;
};

#endif
