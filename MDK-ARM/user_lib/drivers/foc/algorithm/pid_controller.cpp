#include "pid_controller.h"

#include "../foc_utils.h"

/**
 * @brief PID 控制器构造函数
 * 
 * @param kp 比例系数
 * @param ki 积分系数
 * @param kd 微分系数
 * @param output_ramp 输出速率限制
 * @param limit 输出范围
 */
pid_controller::pid_controller(float kp, float ki, float kd, float output_ramp, float limit)
    : kp(kp),
      ki(ki),
      kd(kd),
      output_ramp(output_ramp),
      limit(limit)
{
    timestamp_prev = foc_utils::get_us_tick();
}

/**
 * @brief PID 控制器运算符重载
 * 
 * @param error 误差值
 * @return float 输出值
 */
float pid_controller::operator()(float error)
{
    uint32_t timestamp_now = foc_utils::get_us_tick();
    float Ts = (float)(timestamp_now - timestamp_prev) / 1.0e-6f;
    if(Ts <= 0.0f || Ts > 0.5f){Ts = 1.0e-3f;}

    // 比例项
    float proportional = kp * error;

    // 积分项
    float integral = integral_prev + ki * Ts * 0.5f * (error + error_prev);
    integral = _constrain(integral, -limit, limit);

    // 微分项
    float derivative = kd * (error - error_prev) / Ts;

    // 计算总输出
    float output = proportional + integral + derivative;
    output = _constrain(output, -output_ramp, output_ramp);

    // 输出速率限制
    if(output_ramp > 0.0f)
    {
        float output_rate = (output - output_prev) / Ts;
        if(output_rate > output_ramp)
        {
            output = output_prev + output_ramp * Ts;
        }
        else if(output_rate < -output_ramp)
        {
            output = output_prev - output_ramp * Ts;
        }
    }

    // 更新中间变量
    error_prev = error;
    integral_prev = integral;
    output_prev = output;
    timestamp_prev = timestamp_now;

    return output;
}
