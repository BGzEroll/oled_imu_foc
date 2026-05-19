#include "spi_bus.h"

#define QUEUE_SIZE           16

extern SPI_HandleTypeDef hspi1;

class spi_dev
{
    public:
        spi_dev(SPI_HandleTypeDef *spi_handle)
            : spi_handle(spi_handle){}

        void init()
        {
            // spi 初始化由 CubeMX 管理
            if(is_init || !spi_handle){return;}
            is_init = true;
        }

        void cs_low(GPIO_TypeDef *port, uint16_t pin)
        {
            HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
        }

        void cs_high(GPIO_TypeDef *port, uint16_t pin)
        {
            HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
        }

        void rx(uint8_t *buf, uint32_t len)
        {
            if(!spi_handle || !buf || !len){return;}
            if(dma_busy){return;}

            HAL_SPI_Receive(spi_handle, buf, len, HAL_MAX_DELAY);
        }

        void tx(const uint8_t *buf, uint32_t len)
        {
            if(!spi_handle || !buf || !len){return;}
            if(dma_busy){return;}

            HAL_SPI_Transmit(spi_handle, (uint8_t *)buf, len, HAL_MAX_DELAY);
        }

        void rx_tx(const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len)
        {
            if(!spi_handle || (!tx_buf && !rx_buf) || !len){return;}
            if(dma_busy){return;}

            HAL_SPI_TransmitReceive(spi_handle, (uint8_t *)tx_buf, rx_buf, len, HAL_MAX_DELAY);
        }

        bool submit_dma_tx(const uint8_t *buf, uint32_t len, volatile int8_t *dma_done)
        {
            if(!spi_handle || !buf || !len){return false;}

            dma_req req = {buf, nullptr, len, dma_done, dma_mode::DMA_MODE_TX};

            __disable_irq();
            bool ok = queue_push(req);
            __enable_irq();

            if(ok && !dma_busy)
            {
                start_next_transfer();
            }

            return ok;
        }

        bool submit_dma_rx(uint8_t *buf, uint32_t len, volatile int8_t *dma_done)
        {
            if(!spi_handle || !buf || !len){return false;}

            dma_req req = {nullptr, buf, len, dma_done, dma_mode::DMA_MODE_RX};

            __disable_irq();
            bool ok = queue_push(req);
            __enable_irq();

            if(ok && !dma_busy)
            {
                start_next_transfer();
            }

            return ok;
        }

        bool submit_dma_rx_tx(const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len, volatile int8_t *dma_done)
        {
            if(!spi_handle || !tx_buf || !rx_buf || !len){return false;}

            dma_req req = {tx_buf, rx_buf, len, dma_done, dma_mode::DMA_MODE_RX_TX};

            __disable_irq();
            bool ok = queue_push(req);
            __enable_irq();

            if(ok && !dma_busy)
            {
                start_next_transfer();
            }

            return ok;
        }

    private:
        struct dma_req
        {
            const uint8_t *tx_buf;
            uint8_t *rx_buf;
            uint32_t len;
            volatile int8_t *dma_done;
            uint8_t mode;
        };

        enum dma_mode
        {
            DMA_MODE_RX = 0,
            DMA_MODE_TX,
            DMA_MODE_RX_TX,
        };

    private:
        bool queue_empty() const
        {
            return queue_head == queue_tail;
        }

        bool queue_full() const
        {
            return (queue_tail + 1) % QUEUE_SIZE == queue_head;
        }

        bool queue_push(const dma_req &req)
        {
            if(queue_full()){return false;}
            queue[queue_tail] = req;
            queue_tail = (queue_tail + 1) % QUEUE_SIZE;
            return true;
        }

        bool queue_pop(dma_req &req)
        {
            if(queue_empty()){return false;}
            req = queue[queue_head];
            queue_head = (queue_head + 1) % QUEUE_SIZE;
            return true;
        }

        void start_next_transfer()
        {
            while(!queue_empty())
            {
                if(!queue_pop(current_req)){break;}

                HAL_StatusTypeDef ret = HAL_ERROR;

                if(current_req.mode == dma_mode::DMA_MODE_TX)
                {
                    ret = HAL_SPI_Transmit_DMA(spi_handle, (uint8_t *)current_req.tx_buf, current_req.len);
                }
                else if(current_req.mode == dma_mode::DMA_MODE_RX)
                {
                    ret = HAL_SPI_Receive_DMA(spi_handle, current_req.rx_buf, current_req.len);
                }
                else
                {
                    ret = HAL_SPI_TransmitReceive_DMA(spi_handle, (uint8_t *)current_req.tx_buf, current_req.rx_buf, current_req.len);
                }

                if(ret == HAL_OK)
                {
                    dma_busy = true;
                    if(current_req.dma_done){*current_req.dma_done = SPI_DMA_BUSY;}
                    return;
                }

                if(current_req.dma_done){*current_req.dma_done = SPI_DMA_ERROR;}
            }

            dma_busy = false;
        }

        void on_dma_done()
        {
            dma_busy = false;
            if(current_req.dma_done){*current_req.dma_done = SPI_DMA_OK;}
            start_next_transfer();
        }

        void on_dma_error()
        {
            dma_busy = false;
            if(current_req.dma_done){*current_req.dma_done = SPI_DMA_ERROR;}
            start_next_transfer();
        }

    private:
        SPI_HandleTypeDef *spi_handle;
        bool is_init = false;
        volatile bool dma_busy = false;
        dma_req queue[QUEUE_SIZE];
        volatile uint8_t queue_head = 0;
        volatile uint8_t queue_tail = 0;
        dma_req current_req;

    private:
        friend void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
        friend void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);
        friend void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);
        friend void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi);
};

// 静态 spi 设备表（资源池）
static spi_dev spi_devs[] =
{
    spi_dev(&hspi1)
};
static constexpr uint8_t SPI_DEV_NUM = sizeof(spi_devs) / sizeof(spi_devs[0]);

/**
 * @brief 根据 bus_id 获取对应底层设备
 *
 * @note 超出范围时默认返回 bus0；
 * @note 保证始终返回有效指针；
 */
static spi_dev *get_dev(uint8_t bus_id)
{
    if(bus_id < SPI_DEV_NUM)
    {
        return &spi_devs[bus_id];
    }
    return &spi_devs[0];
}

/**
 * @brief spi 总线构造函数
 *
 * @param bus_id spi 总线编号
 */
spi_bus::spi_bus(uint8_t bus_id)
    : bus_id(bus_id){}

/**
 * @brief 初始化 spi 总线
 */
void spi_bus::init()
{
    get_dev(bus_id)->init();
}

/**
 * @brief spi 拉低 cs 电平
 *
 * @param port cs 引脚端口
 * @param pin cs 引脚编号
 */
void spi_bus::cs_low(GPIO_TypeDef *port, uint16_t pin)
{
    get_dev(bus_id)->cs_low(port, pin);
}

/**
 * @brief spi 拉高 cs 电平
 *
 * @param port cs 引脚端口
 * @param pin cs 引脚编号
 */
void spi_bus::cs_high(GPIO_TypeDef *port, uint16_t pin)
{
    get_dev(bus_id)->cs_high(port, pin);
}

/**
 * @brief spi 接收数据
 *
 * @param buf 接收缓冲区
 * @param len 接收长度
 */
void spi_bus::rx(uint8_t *buf, uint32_t len)
{
    get_dev(bus_id)->rx(buf, len);
}

/**
 * @brief spi 发送数据
 *
 * @param buf 数据缓冲区
 * @param len 数据长度
 */
void spi_bus::tx(const uint8_t *buf, uint32_t len)
{
    get_dev(bus_id)->tx(buf, len);
}

/**
 * @brief spi 发送接收数据
 *
 * @param tx_buf 发送缓冲区
 * @param rx_buf 接收缓冲区
 * @param len 数据长度
 */
void spi_bus::rx_tx(const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len)
{
    get_dev(bus_id)->rx_tx(tx_buf, rx_buf, len);
}

/**
 * @brief 提交 dma 接收请求
 *
 * @param buf 接收缓冲区
 * @param len 接收长度
 * @param dma_done 完成标志指针
 *
 * @return true 成功
 * @return false 失败
 * 
 * @note dma_done 有三种状态：
 * @note 0 未完成
 * @note 1 完成
 * @note -1 错误
 */
bool spi_bus::submit_dma_rx(uint8_t *buf, uint32_t len, volatile int8_t *dma_done)
{
    return get_dev(bus_id)->submit_dma_rx(buf, len, dma_done);
}

/**
 * @brief 提交 dma 发送请求
 *
 * @param buf 数据缓冲区
 * @param len 数据长度
 * @param dma_done 完成标志指针
 *
 * @return true 成功
 * @return false 失败
 * 
 * @note dma_done 有三种状态：
 * @note 0 未完成
 * @note 1 完成
 * @note -1 错误
 */
bool spi_bus::submit_dma_tx(const uint8_t *buf, uint32_t len, volatile int8_t *dma_done)
{
    return get_dev(bus_id)->submit_dma_tx(buf, len, dma_done);
}

/**
 * @brief 提交 dma 发送接收请求
 *
 * @param tx_buf 发送缓冲区
 * @param rx_buf 接收缓冲区
 * @param len 数据长度
 * @param dma_done 完成标志指针
 *
 * @return true 成功
 * @return false 失败
 * 
 * @note dma_done 有三种状态：
 * @note 0 未完成
 * @note 1 完成
 * @note -1 错误
 */
bool spi_bus::submit_dma_rx_tx(const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len, volatile int8_t *dma_done)
{
    return get_dev(bus_id)->submit_dma_rx_tx(tx_buf, rx_buf, len, dma_done);
}

/**
 * @brief spi tx dma 完成回调
 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    spi_dev *p = nullptr;
    if(hspi->Instance == SPI1){p = get_dev(0);}

    if(!p || !p->is_init){return;}

    p->on_dma_done();
}

/**
 * @brief spi rx dma 完成回调
 */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    spi_dev *p = nullptr;
    if(hspi->Instance == SPI1){p = get_dev(0);}

    if(!p || !p->is_init){return;}

    p->on_dma_done();
}

/**
 * @brief spi rx tx dma 完成回调
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    spi_dev *p = nullptr;
    if(hspi->Instance == SPI1){p = get_dev(0);}

    if(!p || !p->is_init){return;}

    p->on_dma_done();
}

/**
 * @brief spi dma 错误回调
 */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    spi_dev *p = nullptr;
    if(hspi->Instance == SPI1){p = get_dev(0);}

    if(!p || !p->is_init){return;}

    p->on_dma_error();
}
