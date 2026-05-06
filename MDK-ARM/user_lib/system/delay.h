#ifndef DELAY_H
#define DELAY_H

#include "stm32f4xx_hal.h"

class delay
{
    public:
        delay() = delete;
        delay(const delay &) = delete;
        delay &operator=(const delay &) = delete;

    public:
        /** 
         * @brief 阻塞延迟 us
         * 
         * @param us 延时微秒数
         */
        static inline void us(uint32_t us)
        {
            init();

            uint32_t start = DWT->CYCCNT;
            uint32_t tick = us * delay::ticks_per_us;

            while((uint32_t)(DWT->CYCCNT - start) < tick);
        }

        /** 
         * @brief 阻塞延迟 ms
         * 
         * @param ms 延时毫秒数
         */
        static inline void ms(uint32_t ms)
        {
            HAL_Delay(ms);
        }

        /**
         * @brief 获取当前微秒 tick
         * 
         * @return 当前系统运行到现在的微秒数
         */
        static inline uint32_t get_us_tick()
        {
            init();

            return DWT->CYCCNT / delay::ticks_per_us;
        }

        /**
         * @brief 获取当前毫秒 tick
         * 
         * @return 当前系统运行到现在的毫秒数
         */
        static inline uint32_t get_ms_tick()
        {
            return HAL_GetTick();
        }

    private:
        /**
         * @brief 初始化 dwt
         */
        static inline void init()
        {
            if(!delay::is_init)
            {
                CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
                DWT->CYCCNT = 0;
                DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

                delay::is_init = true;
            }

            refresh_ticks_per_us();
        }

        /**
         * @brief 刷新微秒 tick 数
         */
        static inline void refresh_ticks_per_us()
        {
            if(delay::last_system_core_clock == SystemCoreClock){return;}
            delay::last_system_core_clock = SystemCoreClock;

            uint32_t ticks = SystemCoreClock / 1000000UL;
            if(ticks == 0){ticks = 1;}

            delay::ticks_per_us = ticks;
        }

    private:
        static bool is_init;
        static uint32_t last_system_core_clock;
        static uint32_t ticks_per_us;
};

#endif
