#include "lowpass_filter.h"

#include "../foc_utils.h"

/**
 * @brief 低通滤波器
 * 
 * @param Tf 滤波时间常数
 */
lowpass_filter::lowpass_filter(float Tf)
    : Tf(Tf)
{
    timestamp_prev = foc_utils::get_us_tick();
}

/**
 * @brief 低通滤波器
 * 
 * @param input 输入信号
 */
float lowpass_filter::operator()(float input)
{
    uint32_t timestamp = foc_utils::get_us_tick();
    float dt = (float)(timestamp - timestamp_prev) * 1e-6f;

    if(Tf <= 0.0f)
    {
        y_prev = input;
        timestamp_prev = timestamp;
        return input;
    }

    if(dt < 0.0f){dt = 1.0e-3f;}
    else if(dt > 0.3f)
    {
        y_prev = input;
        timestamp_prev = timestamp;
        return input;
    }

    float alpha = Tf / (Tf + dt);
    float y = alpha * y_prev + (1.0f - alpha) * input;
    y_prev = y;
    timestamp_prev = timestamp;
    return y;
}

/**
 * @brief 设置滤波时间常数
 * 
 * @param Tf 滤波时间常数
 */
void lowpass_filter::set_Tf(float Tf)
{
    this->Tf = Tf;
}
