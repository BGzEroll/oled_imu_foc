#include "led.h"

/**
 * @brief 构造函数
 */
led::led(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState on_level)
    : port(port), pin(pin), on_level(on_level){}

/**
 * @brief 点亮 led
 */
void led::on()
{
    HAL_GPIO_WritePin(port, pin, on_level);
}

/**
 * @brief 熄灭 led
 */
void led::off()
{
    GPIO_PinState level = (on_level == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    HAL_GPIO_WritePin(port, pin, level);
}

/**
 * @brief 切换 led 状态
 */
void led::toggle()
{
    HAL_GPIO_TogglePin(port, pin);
}
