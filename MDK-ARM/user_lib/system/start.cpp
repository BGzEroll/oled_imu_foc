#include "system/event.h"
#include "system/delay.h"
#include "devices/led_dev.h"

#define DEBUG_TEST
#ifdef DEBUG_TEST

#include <math.h>
#include "third_party/segger_rtt/SEGGER_RTT.h"

static void test_proc(uint32_t tick)
{
	static float phase = 0.0f;
	static uint32_t cnt = 0;

	phase += (float)tick * 0.01f;
	if(phase >= 6.283185307f)
	{
		phase -= 6.283185307f;
	}

	int sin_x1000 = (int)(sinf(phase) * 1000.0f);
	int cos_x1000 = (int)(cosf(phase) * 1000.0f);

	static uint32_t check_tick_cnt[2] = {0};
	check_tick_cnt[1] = check_tick_cnt[0];
	check_tick_cnt[0] = delay::get_ms_tick();

	SEGGER_RTT_printf(0, "%d,%d,%d,%d\n", cnt++, check_tick_cnt[0] - check_tick_cnt[1], sin_x1000, cos_x1000);
}

static void test_init(void)
{
	SEGGER_RTT_Init();
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
