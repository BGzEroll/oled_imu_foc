#ifndef SENSORS_H
#define SENSORS_H

#include "stm32f4xx_hal.h"
#include "../algorithm/lowpass_filter.h"

class encoder_sensors
{
    public:
        virtual void init() = 0;
        virtual void update() = 0;
        virtual float get_angle();
        virtual float get_full_angle();
        virtual float get_velocity();

    public:
        float velocity_filter_Tf = 0.0f;

    private:
        lowpass_filter velocity_filter{velocity_filter_Tf};

    private:
        float angle = 0.0f;
        float full_angle = 0.0f;
        float velocity = 0.0f;
        uint32_t sample_id = 0xFFFFFFFF;

    protected:
        virtual uint32_t get_sample_id() = 0;
        virtual float get_raw_angle() = 0;
        virtual float get_raw_full_angle() = 0;
        virtual float get_raw_velocity() = 0;
};

class current_sensors
{
    public:
        virtual void init() = 0;
        virtual void get_offset() = 0;
        virtual void update() = 0;
        virtual void get_current(float *i_a, float *i_b, float *i_c);
        virtual void get_voltage(float *v_a, float *v_b, float *v_c);

    protected:
        virtual float *get_raw_current() = 0;
        virtual float *get_raw_voltage() = 0;
};

class vbus_sensors
{
    public:
        virtual void init() = 0;
        virtual float get_vbus();

    protected:
        virtual float get_raw_vbus() = 0;
};

#endif
