#ifndef I2C_BUS_H
#define I2C_BUS_H

#include "stm32f4xx_hal.h"

class i2c_bus
{
    public:
        explicit i2c_bus(uint8_t bus_id = 0);

    public:
        void init();
        bool submit_dma_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len, void (*cb)(bool, void *), void *user_data = nullptr);
        bool read_bytes(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len);
        void write_bytes(uint8_t addr, uint8_t reg, const uint8_t *buf, uint8_t len);

    private:
        uint8_t bus_id;
};

#endif
