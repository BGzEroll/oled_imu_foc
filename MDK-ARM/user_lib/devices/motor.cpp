#include "motor.h"

#include "main.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim8;
extern ADC_HandleTypeDef hadc1;

static as5600 encoder_1(0, 0x36);
static INA240 current_sensor_1(&hadc1, ADC_INJECTED_RANK_1, ADC_INJECTED_RANK_2, 0.01f, 50.0f);
foc_motor motor_1(&htim1, MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, 7, 5.1f, 220.0f);

/**
 * @brief 初始化电机
 */
void motor_init()
{
    motor_1.link_encoder_sensor(&encoder_1);
    motor_1.link_current_sensor(&current_sensor_1);
    motor_1.encoder_sensor->velocity_filter_Tf = 0.0063662f;        // 25Hz 低通滤波
	motor_1.voltage_power_supply = 12.0f;
	motor_1.voltage_limit = 1.0f;
    motor_1.voltage_sensor_align = 3.0f;
	motor_1.controller_type = foc_controller::type::velocity_openloop;
    motor_1.torque_type = foc_controller::torque_type::voltage;
    encoder_1.init();
    current_sensor_1.init();
	motor_1.init();
}

/**
 * @brief 定时器中断回调函数
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM1)
    {
        motor_1.loop();
    }
}
