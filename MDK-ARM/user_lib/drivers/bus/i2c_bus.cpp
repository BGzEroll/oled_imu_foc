#include "i2c_bus.h"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

class i2c_dev
{
    public:
        i2c_dev(I2C_HandleTypeDef *i2c_handle)
            : i2c_handle(i2c_handle){}

        void init()
        {
            // i2c 初始化由 CubeMX 管理
            if(is_init || !i2c_handle){return;}
            is_init = true;
        }

        bool get_dma_status(){return dma_rx_busy;}
        bool get_dma_error(){return dma_error;}
        
        bool dma_read_bytes(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len)
        {
            if(!i2c_handle || !buf || !len){return false;}
            if(HAL_I2C_GetState(i2c_handle) != HAL_I2C_STATE_READY){return false;}
            if(dma_rx_busy){return false;}

            if(HAL_I2C_Mem_Read_DMA(i2c_handle, addr << 1, reg, I2C_MEMADD_SIZE_8BIT, buf, len) == HAL_OK)
            {
                dma_rx_busy = true;
                dma_error = false;
                return true;
            }

            return false;
        }

        bool read_bytes(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len)
        {
            if(!i2c_handle || !buf || !len){return false;}
            if(dma_rx_busy){return false;}

            if(HAL_I2C_Mem_Read(i2c_handle, addr << 1, reg, I2C_MEMADD_SIZE_8BIT, buf, len, HAL_MAX_DELAY) == HAL_OK)
            {
                return true;
            }
            
            return false;
        }

        void write_bytes(uint8_t addr, uint8_t reg, const uint8_t *buf, uint8_t len)
        {
            if(!i2c_handle || (!buf && len > 0)){return;}
            if(dma_rx_busy){return;}

            HAL_I2C_Mem_Write(i2c_handle, addr << 1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)buf, len, HAL_MAX_DELAY);
        }

    private:
        I2C_HandleTypeDef *i2c_handle;
        bool is_init = false;
        volatile bool dma_rx_busy = false;
        volatile bool dma_error = false;

    private:
        friend void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);
        friend void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c);
};

// 静态 i2c 设备表（资源池）
static i2c_dev i2c_devs[] =
{
    i2c_dev(&hi2c1),
    i2c_dev(&hi2c2)
};
static constexpr uint8_t I2C_DEV_NUM = sizeof(i2c_devs) / sizeof(i2c_devs[0]);

/**
 * @brief 根据 bus_id 获取对应底层设备
 *
 * @note 超出范围时默认返回 bus0；
 * @note 保证始终返回有效指针；
 */
static i2c_dev *get_dev(uint8_t bus_id)
{
    if(bus_id < I2C_DEV_NUM)
    {
        return &i2c_devs[bus_id];
    }
    return &i2c_devs[0];
}

/**
 * @brief i2c 总线构造函数
 * 
 * @param bus_id i2c 总线编号
 */
i2c_bus::i2c_bus(uint8_t bus_id)
    : bus_id(bus_id){}

/**
 * @brief 初始化 i2c 总线
 */
void i2c_bus::init()
{
    get_dev(bus_id)->init();
}

/**
 * @brief 获取 dma 状态
 * 
 * @return true dma 繁忙
 * @return false dma 空闲
 */
bool i2c_bus::get_dma_status()
{
    return get_dev(bus_id)->get_dma_status();
}

/**
 * @brief 获取 dma 错误状态
 * 
 * @return true dma 错误
 * @return false dma 正常
 */
bool i2c_bus::get_dma_error()
{
    return get_dev(bus_id)->get_dma_error();
}

/**
 * @brief dma 连续读取寄存器数据
 * 
 * @param addr 设备地址
 * @param reg  起始寄存器地址
 * @param buf  接收缓冲区
 * @param len  读取长度
 * 
 * @return true 成功
 * @return false 失败
 */
bool i2c_bus::dma_read_bytes(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    return get_dev(bus_id)->dma_read_bytes(addr, reg, buf, len);
}

/**
 * @brief 连续读取寄存器数据
 * 
 * @param addr 设备地址
 * @param reg  起始寄存器地址
 * @param buf  接收缓冲区
 * @param len  读取长度
 * 
 * @return true 成功
 * @return false 失败
 */
bool i2c_bus::read_bytes(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    return get_dev(bus_id)->read_bytes(addr, reg, buf, len);
}

/**
 * @brief 连续写入寄存器数据
 * 
 * @param addr 设备地址
 * @param reg  起始寄存器地址
 * @param buf  数据缓冲区
 * @param len  写入长度
 */
void i2c_bus::write_bytes(uint8_t addr, uint8_t reg, const uint8_t *buf, uint8_t len)
{
    get_dev(bus_id)->write_bytes(addr, reg, buf, len);
}

/**
 * @brief i2c dma 接收完成回调
 */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    i2c_dev *p = nullptr;
    if(hi2c->Instance == I2C1){p = get_dev(0);}
    else if(hi2c->Instance == I2C2){p = get_dev(1);}

    if(!p || !p->is_init){return;}

    p->dma_rx_busy = false;
}

/**
 * @brief i2c dma 接收错误回调
 */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    i2c_dev *p = nullptr;
    if(hi2c->Instance == I2C1){p = get_dev(0);}
    else if(hi2c->Instance == I2C2){p = get_dev(1);}
    
    if(!p || !p->is_init){return;}

    p->dma_rx_busy = false;
    p->dma_error = true;
}
