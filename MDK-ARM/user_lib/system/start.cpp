#include "system/event.h"
#include "system/delay.h"
#include "devices/led_dev.h"
#include "devices/mpu6050_dev.h"
#include "devices/ssd1306_dev.h"
#include "devices/motor.h"

#define DEBUG_TEST
#ifdef DEBUG_TEST

#include <string.h>
#include <stdio.h>
#include "drivers/bus/uart_bus.h"
#include "third_party/segger_rtt/SEGGER_RTT.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc3;

static uart_bus uart_0(0);

static volatile uint16_t adc3_value = 0;
static volatile uint16_t adc1_value[2] = {0};

static void test_proc(uint32_t tick)
{
	motor_1.move(3.0f);

	static uint32_t oled_show_cnt = 0;
	if((oled_show_cnt += tick) >= 50 && (oled_show_cnt = 0, 1))
	{
		// oled.show_string(1, 1, "AX:");
		// oled.show_signed_num(1, 4, (int32_t)(mpu6050_dev.acc[0] * 1000), 4);
		// oled.show_string(2, 1, "AY:");
		// oled.show_signed_num(2, 4, (int32_t)(mpu6050_dev.acc[1] * 1000), 4);
		// oled.show_string(3, 1, "AZ:");
		// oled.show_signed_num(3, 4, (int32_t)(mpu6050_dev.acc[2] * 1000), 4);
		// oled.show_string(4, 1, "UNIT:mg");

		oled.show_string(4, 1, "Encoder:");
		oled.show_signed_num(4, 9, (int32_t)(motor_1.encoder_sensor->get_angle() * 1000.0f), 4);

		oled.show_string(1, 1, "ADC1_1:");
		oled.show_signed_num(1, 8, (int32_t)((float)adc1_value[0] / 4095.0f * 3.3f * 2.0f * 1000.0f), 5);

		oled.show_string(2, 1, "ADC1_2:");
		oled.show_signed_num(2, 8, (int32_t)((float)adc1_value[1] / 4095.0f * 3.3f * 2.0f * 1000.0f), 5);

		oled.show_string(3, 1, "ADC3:");
		oled.show_signed_num(3, 8, (int32_t)(((float)adc3_value / 4095.0f) * 3.3f * 11.0f * 1000.0f), 5);

		oled.flush();
	}

	// SEGGER_RTT_printf(0, "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
	// 	mpu6050_dev.acc[0], mpu6050_dev.acc[1], mpu6050_dev.acc[2],
	// 	mpu6050_dev.gyro[0], mpu6050_dev.gyro[1], mpu6050_dev.gyro[2],
	// 	mpu6050_dev.angle[0], mpu6050_dev.angle[1], mpu6050_dev.angle[2]
	// );

	// static uint32_t uart_print_cnt = 0;
	// if((uart_print_cnt += tick) >= 1000 && (uart_print_cnt = 0, 1))
	// {
	// 	uart_0.write_bytes((uint8_t *)"Hello, World!\n", 14);
	// }
}

static void test_init(void)
{
	uart_0.init();
	HAL_ADC_Start_DMA(&hadc3, (uint32_t *)&adc3_value, 1);
	HAL_ADCEx_InjectedStart_IT(&hadc1);
}

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1) {
        adc1_value[0] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1); // IN0
        adc1_value[1] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2); // IN1
    }
}

#endif

/**
 * @brief 事件列表
 */
static void event_list(void)
{
#ifdef DEBUG_TEST
	static event test_task(1, test_proc);
	test_task.start();
#endif

	static event green_led_task(1000, [](){green_led.toggle();});
	green_led_task.start();

	static event blue_led_task(50, blue_led_proc);
	blue_led_task.start();

	static event mpu6050_task(1, [](){mpu6050_dev.update();});
	// mpu6050_task.start();

	static event motor_task(1, [](){motor_1.update();});
	motor_task.start();
}

/**
 * @brief 初始化所有模块
 */
extern "C" void start_init_all(void)
{
	delay::ms(1000);

#ifdef DEBUG_TEST
	test_init();
#endif

	SEGGER_RTT_Init();
	oled.init();
	// mpu6050_dev.init();
	motor_init();
	
	event_list();
}

/**
 * @brief 调度器循环
 */
extern "C" void start_loop(void)
{
	event::loop();
}
