#ifndef FOC_CONTROLLER_H
#define FOC_CONTROLLER_H

#include "stm32f4xx_hal.h"

class foc_motor;

class foc_controller
{
    public:
        enum class type {
            none,
            torque,
            velocity,
            angle,
            velocity_openloop,
            angle_openloop,
        };

        enum class torque_type {
            voltage,
            dc_current,
            foc_current
        };

        enum class direction {
            CW = 1,
            CCW = -1,
            UNKNOWN = 0
        };

    private:
        void bind(foc_motor *motor);

    private:
        void move(type controller_type, float target);

    private:
        void torque(float torque);
        void velocity(float velocity);
        void angle(float angle);
        void velocity_openloop(float velocity);
        void angle_openloop(float angle);

    private:
        void set_dq_voltage(float Uq, float Ud, float electrical_angle);

    private:
        foc_motor *motor = nullptr;

    private:
        bool is_enable = false;
        uint32_t open_loop_timestamp = 0;
        float shaft_angle = 0.0f;
        float shaft_velocity = 0.0f;
        float voltage_bemf = 0.0f;

    private:
        friend class foc_motor;
};

#endif
