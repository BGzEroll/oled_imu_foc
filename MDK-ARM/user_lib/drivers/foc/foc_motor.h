#ifndef FOC_MOTOR_H
#define FOC_MOTOR_H

#include "stm32f4xx_hal.h"
#include "foc_controller.h"
#include "sensors/sensors.h"

class foc_motor
{
    public:
        foc_motor(TIM_HandleTypeDef *tim_handle, GPIO_TypeDef *en_gpio = nullptr, uint16_t en_pin = 0,
            uint8_t PP = 0, float R = -114.514f, float KV = -114.514f);
        foc_motor(const foc_motor &) = delete;
        foc_motor &operator=(const foc_motor &) = delete;

    public:
        void init();
        void enable();
        void disable();
        void link_encoder_sensor(encoder_sensors *encoder_sensor);
        void link_current_sensor(current_sensors *current_sensor);
        void link_vbus_sensor(vbus_sensors *vbus_sensor);

    public:
        void update();
        void loop();
        void move(float target);

    public:
        float get_shaft_angle() const {return controller.shaft_angle;}
        float get_shaft_velocity() const {return controller.shaft_velocity;}

    public:
        foc_controller::type controller_type = foc_controller::type::none;
        foc_controller::torque_type torque_type = foc_controller::torque_type::voltage;
        float voltage_power_supply = 6.0f;
        float voltage_limit = 0.0f;
        float voltage_sensor_align = 0.0f;

    public:
        encoder_sensors *encoder_sensor = nullptr;
        current_sensors *current_sensor = nullptr;
        vbus_sensors *vbus_sensor = nullptr;

    private:
        TIM_HandleTypeDef *tim_handle;
        GPIO_TypeDef *en_gpio;
        uint16_t en_pin;
        uint8_t PP;
        float R;
        float KV;
        foc_controller controller;

    private:
        volatile float Uq = 0.0f;
        volatile float Ud = 0.0f;
        volatile float electrical_angle = 0.0f;

    private:
        bool align();
        inline uint32_t get_pwm_arr() const {return __HAL_TIM_GET_AUTORELOAD(tim_handle);}
        void set_dq_voltage(float Uq, float Ud, float electrical_angle){controller.set_dq_voltage(Uq, Ud, electrical_angle);}
        void set_phase_voltage();

    private:
        foc_controller::direction sensor_direction = foc_controller::direction::UNKNOWN;
        float zero_electric_angle = 0.0f;

    private:
        friend class foc_controller;
};

#endif
