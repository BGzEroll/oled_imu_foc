#ifndef FOC_UTILS_H
#define FOC_UTILS_H

#include "stm32f4xx_hal.h"
#include "system/delay.h"
#include <math.h>

#define _constrain(amt, low, high)        ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#define _round(x)                         ((x) >= 0 ? (int32_t)((x) + 0.5f) : (int32_t)((x) - 0.5f))
#define _sqrt(x)                          foc_utils::sqrt_approx(x)

/* 常量定义 */
#define _2_SQRT3         1.15470053838f
#define _SQRT3           1.73205080757f
#define _1_SQRT3         0.57735026919f
#define _SQRT3_2         0.86602540378f
#define _SQRT2           1.41421356237f
#define _120_D2R         2.09439510239f
#define _PI              3.14159265359f
#define _PI_2            1.57079632679f
#define _PI_3            1.0471975512f
#define _2PI             6.28318530718f
#define _3PI_2           4.71238898038f
#define _PI_6            0.52359877559f
#define _RPM_TO_RADS     0.10471975512f

class foc_utils
{
    public:
        foc_utils() = delete;
        foc_utils(const foc_utils &) = delete;
        foc_utils &operator=(const foc_utils &) = delete;

    public:
        static inline uint32_t get_us_tick(){return delay::get_us_tick();}
        static inline void delay_ms(uint32_t ms){delay::ms(ms);}
        static float sin(float angle);
        static float cos(float angle);
        static float normalize_angle(float angle);
        static float electrical_angle(float shaft_angle, uint8_t pole_pairs);
        static float sqrt_approx(float x);
};

#endif
