#ifndef AS5600_H
#define AS5600_H

#include "stm32f4xx_hal.h"
#include "drivers/bus/i2c_bus.h"
#include "../sensors.h"

class as5600 : public encoder_sensors {
    public:
        as5600(uint8_t i2c_bus_id, uint8_t dev_addr);

    public:
        void init() override;

    private:
        void update() override;
        uint32_t get_sample_id() override;
        float get_raw_angle() override;
        float get_raw_full_angle() override;
        float get_raw_velocity() override;

    private:
        float rad_angle = 0.0f;
        float rad_full_angle = 0.0f;
        float rad_velocity = 0.0f;

    private:
        void process_data();

    private:
        uint8_t i2c_bus_id;
        i2c_bus i2c;
        uint8_t dev_addr;
        uint8_t reg_addr = 0x0C;
    
    private:
        uint32_t sample_id = 0;
        uint8_t raw_data[2];
        uint16_t prev_count = 0;
        int32_t total_count = 0;
        uint32_t prev_Ts = 0;
        bool first_update = true;
        volatile int8_t process_step = I2C_DMA_BUSY;
};

#endif
