#include "as5600.h"

#include "../../foc_utils.h"

#define COUNT_TO_RAD        (2.0f * _PI / 4096.0f)

/**
 * @brief as5600 构造函数
 * 
 * @param i2c_bus_id i2c 总线编号
 * @param dev_addr as5600 设备地址
 */
as5600::as5600(uint8_t i2c_bus_id, uint8_t dev_addr)
    : i2c_bus_id(i2c_bus_id),
      dev_addr(dev_addr),
      i2c(i2c_bus_id){}

/**
 * @brief as5600 初始化函数
 */
void as5600::init()
{
    i2c.init();
}

/**
 * @brief 更新 as5600 数据
 */
void as5600::update()
{
    if(process_step == 1)
    {
        if(i2c.get_dma_status())
        {
            return;
        }

        process_step = false;

        if(!i2c.get_dma_error())
        {
            process_data();
        }
    }

    if(dma_get_raw_data())
    {
        process_step = true;
    }
}

/**
 * @brief 获取当前样本序号
 * 
 * @return 当前样本序号
 */
uint32_t as5600::get_sample_id()
{
    return sample_id;
}

/**
 * @brief 获取 as5600 角度
 * 
 * @return float 角度，单位：弧度，范围：[0, 2PI)
 */
float as5600::get_raw_angle()
{
    return rad_angle;
}

/**
 * @brief 获取 as5600 累积角度
 * 
 * @return float 累积角度，单位：弧度
 */
float as5600::get_raw_full_angle()
{
    return rad_full_angle;
}

/**
 * @brief 获取 as5600 角速度
 * 
 * @return float 角速度，单位：弧度/秒
 */
float as5600::get_raw_velocity()
{
    return rad_velocity;
}

/**
 * @brief dma 获取 as5600 原始数据
 * 
 * @return true 成功
 * @return false 失败
 */
bool as5600::dma_get_raw_data()
{
    return i2c.dma_read_bytes(dev_addr, reg_addr, raw_data, 2);
}

/**
 * @brief 获取 as5600 原始数据
 */
void as5600::get_raw_data()
{
    i2c.read_bytes(dev_addr, reg_addr, raw_data, 2);
}

/**
 * @brief 处理 as5600 数据
 */
void as5600::process_data()
{
    uint16_t count = (raw_data[0] << 8) | raw_data[1];
    count &= 0x0FFF;

    // 单圈角度范围 [0, 2PI)
    rad_angle = (float)count * COUNT_TO_RAD;

    uint32_t now = foc_utils::get_us_tick();
    if(prev_Ts)
    {
        int16_t dCount = (int16_t)(count - prev_count);

        if(dCount >= 2048){dCount -= 4096;}
        else if(dCount < -2048){dCount += 4096;}

        // 多圈累积角度
        total_count += dCount;
        rad_full_angle = (float)total_count * COUNT_TO_RAD;

        float dt = (float)(now - prev_Ts) * 1.0e-6f;
        if(dt > 1.0e-6f)
        {
            // 计算角速度
            rad_velocity = (float)dCount / dt * COUNT_TO_RAD;
        }
    }
    else
    {
        total_count = count;
        rad_full_angle = rad_angle;
    }

    // 更新中间变量
    prev_count = count;
    prev_Ts = now;
    sample_id++;
}
