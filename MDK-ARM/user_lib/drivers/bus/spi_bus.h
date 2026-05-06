#ifndef SPI_BUS_H
#define SPI_BUS_H

#include "stm32f4xx_hal.h"

class spi_bus
{
    public:
        explicit spi_bus(uint8_t bus_id = 0);

    public:
        void init();
        void tx(const uint8_t *buf, uint32_t len);
        void rx(uint8_t *buf, uint32_t len);
        void tx_rx(const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len);

    private:
        uint8_t bus_id;
};

#endif
