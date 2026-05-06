#include "led_dev.h"

led green_led(ONBORAD_GREEN_LED_GPIO_Port, ONBORAD_GREEN_LED_Pin, GPIO_PIN_RESET);
led blue_led(ONBORAD_BLUE_LED_GPIO_Port, ONBORAD_BLUE_LED_Pin, GPIO_PIN_RESET);

/**
 * @brief 绿色 led 实例闪烁函数
 */
void green_led_blink(uint32_t tick)
{
    static uint8_t step = 0;
    static uint32_t step_tick = 0;

    step_tick += tick;
    switch(step)
    {
        case 0:
            green_led.on();
            if(step_tick >= tick)
            {
                step_tick = 0;
                step++;
            }
            break;

        case 1:
            green_led.off();
            if(step_tick >= 1000 - tick)
            {
                step_tick = 0;
                step = 0;
            }
            break;
    }
}

/**
 * @brief 绿色 led 实例进程函数
 */
void green_led_proc(uint32_t tick)
{
    green_led_blink(tick);
}

/**
 * @brief 蓝色 led 实例进程函数
 */
void blue_led_proc(uint32_t tick)
{
    blue_led.toggle();
}
