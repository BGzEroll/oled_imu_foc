#ifndef MPU6050_H
#define MPU6050_H

#include "stm32f4xx_hal.h"
#include "bus/i2c_bus.h"

class mpu6050 {
    public:
        mpu6050(uint8_t i2c_bus_num, uint8_t addr, float acc_coef);

    public:
        void init(bool cail = false);
        void update();
    
    public:
        float temperature;
        float acc[3];
        float gyro[3];
        float angle[3];
    
    private:
        uint8_t i2c_bus_num;
        i2c_bus i2c;
        uint8_t addr;
        float acc_coef;
    
    private:
        void get_raw();
        void process_data();
        void write_cfg(uint8_t reg, uint8_t val);
        void get_gyro_offset();

    private:
        uint8_t raw[14];
        float gyro_offset[3] = {0.0f};
        uint32_t prev_Ts;
        float gyro_angle[3];
        bool first_update = true;
        volatile int8_t process_step = I2C_DMA_BUSY;
};

#endif
