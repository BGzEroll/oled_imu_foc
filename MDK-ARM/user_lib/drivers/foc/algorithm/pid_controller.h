#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include "stm32f4xx_hal.h"

class pid_controller
{
    public:
        pid_controller(float kp, float ki, float kd, float output_ramp, float limit);

    public:
        float operator()(float error);

    public:
        float kp;
        float ki;
        float kd;
        float ramp;
        float limit;
        float output_ramp;

    private:
        float error_prev = 0.0f;
        float output_prev = 0.0f;
        float integral_prev = 0.0f;
        uint32_t timestamp_prev;
};

#endif
