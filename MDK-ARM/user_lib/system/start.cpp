#include "system/event.h"
#include "system/delay.h"
#include "devices/led_dev.h"
#include "devices/mpu6050_dev.h"

#define DEBUG_TEST
#ifdef DEBUG_TEST

#include <math.h>
#include "third_party/segger_rtt/SEGGER_RTT.h"

static void test_proc(uint32_t tick)
{
	SEGGER_RTT_printf(0, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		(int)(mpu6050_dev.acc[0] * 1000), (int)(mpu6050_dev.acc[1] * 1000), (int)(mpu6050_dev.acc[2] * 1000),
		(int)(mpu6050_dev.gyro[0] * 1000), (int)(mpu6050_dev.gyro[1] * 1000), (int)(mpu6050_dev.gyro[2] * 1000),
		(int)(mpu6050_dev.angle[0] * 1000), (int)(mpu6050_dev.angle[1] * 1000), (int)(mpu6050_dev.angle[2] * 1000)
	);
}

static void test_init(void)
{
	SEGGER_RTT_Init();
	mpu6050_dev.init(1);
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

	static event green_led_task(50, green_led_proc);
	green_led_task.start();

	static event blue_led_task(500, [](){blue_led.toggle();});
	blue_led_task.start();

	static event mpu6050_task(1, [](){mpu6050_dev.update();});
	mpu6050_task.start();
}

/**
 * @brief 初始化所有模块
 */
void start_init_all(void)
{
	delay::ms(1000);

#ifdef DEBUG_TEST
	test_init();
#endif
	
	event_list();
}

/**
 * @brief 调度器循环
 */
void start_loop(void)
{
	event::loop();
}
