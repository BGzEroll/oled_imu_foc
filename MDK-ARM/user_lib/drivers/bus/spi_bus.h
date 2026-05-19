#ifndef SPI_BUS_H
#define SPI_BUS_H

#include "stm32f4xx_hal.h"

#define SPI_DMA_BUSY         0
#define SPI_DMA_OK           1
#define SPI_DMA_ERROR       -1

class spi_bus
{
    public:
        explicit spi_bus(uint8_t bus_id = 0);

    public:
        void init();
        void cs_low(GPIO_TypeDef *port, uint16_t pin);
        void cs_high(GPIO_TypeDef *port, uint16_t pin);
        void rx(uint8_t *buf, uint32_t len);
        void tx(const uint8_t *buf, uint32_t len);
        void rx_tx(const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len);
        bool submit_dma_tx(const uint8_t *buf, uint32_t len, volatile int8_t *dma_done);
        bool submit_dma_rx(uint8_t *buf, uint32_t len, volatile int8_t *dma_done);
        bool submit_dma_rx_tx(const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len, volatile int8_t *dma_done);

    private:
        uint8_t bus_id;
};

#endif
