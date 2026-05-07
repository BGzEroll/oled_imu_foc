#include "foc_controller.h"

#include "foc_motor.h"
#include "foc_utils.h"
#include "sensors/sensors.h"

/**
 * @brief 绑定电机对象
 * 
 * @param motor 电机指针
 */
void foc_controller::bind(foc_motor *motor)
{
    this->motor = motor;
}

/**
 * @brief 移动电机
 * 
 * @param controller_type 控制器类型
 * @param target 目标值
 */
void foc_controller::move(type controller_type, float target)
{
    if(!is_enable){return;}

    switch(controller_type)
    {
        case type::torque:
            torque(target);
            break;

        case type::velocity:
            velocity(target);
            break;

        case type::angle:
            angle(target);
            break;

        case type::velocity_openloop:
            velocity_openloop(target);
            break;

        case type::angle_openloop:
            angle_openloop(target);
            break;

        default:
            set_dq_voltage(0.0f, 0.0f, 0.0f);
            break;
    }
}

/**
 * @brief 扭矩控制
 * 
 * @param torque 目标扭矩
 */
void foc_controller::torque(float torque)
{
    if(!motor->sensor){return;}
    if(motor->sensor_direction == direction::UNKNOWN){return;}

    shaft_angle = (float)motor->sensor_direction * motor->sensor->get_full_angle();
    shaft_velocity = (float)motor->sensor_direction * motor->sensor->get_velocity();

    if(!is_enable){return;}

    if(motor->KV > 0.0f)
    {
        voltage_bemf = shaft_velocity / motor->KV / _RPM_TO_RADS;
    }

    if(motor->R > 0.0f)
    {
        // current.q = ()
    }

    float U_q = 0.0f, U_d = 0.0f, e_angle = 0.0f;

    if(motor->torque_type == torque_type::voltage)
    {
        if(motor->R < 0.0f){U_q = torque;}
        else{U_q = torque * motor->R + voltage_bemf;}

        U_q = _constrain(U_q, -motor->voltage_limit, motor->voltage_limit);
        U_d = 0.0f;
    }
    else
    {
        // current_sp = torque;
    }

    e_angle = foc_utils::normalize_angle((float)motor->sensor_direction * motor->PP * motor->sensor->get_angle() - motor->zero_electric_angle);

    switch(motor->torque_type)
    {
        case torque_type::voltage:
            break;

        case torque_type::dc_current:
            // TODO
            break;

        case torque_type::foc_current:
            // TODO
            break;
    }

    set_dq_voltage(U_q, U_d, e_angle);
}

/**
 * @brief 速度闭环控制
 * 
 * @param velocity 目标速度，单位：弧度/秒
 */
void foc_controller::velocity(float velocity)
{
    if(!motor){return;}

    // set_dq_voltage(0.0f, velocity, 0.0f);
}

/**
 * @brief 角度闭环控制
 * 
 * @param angle 目标角度，单位：弧度
 */
void foc_controller::angle(float angle)
{
    if(!motor->sensor){return;}
    if(motor->sensor_direction == direction::UNKNOWN){return;}

    shaft_angle = (float)motor->sensor_direction * motor->sensor->get_full_angle();
    shaft_velocity = (float)motor->sensor_direction * motor->sensor->get_velocity();

    if(!is_enable){return;}

    if(motor->KV > 0.0f)
    {
        voltage_bemf = shaft_velocity / motor->KV / _RPM_TO_RADS;
    }

    if(motor->R > 0.0f)
    {
        // current.q = ()
    }
}

/**
 * @brief 速度开环控制
 * 
 * @param velocity 目标速度，单位：弧度/秒
 */
void foc_controller::velocity_openloop(float velocity)
{
    if(!motor){return;}

    uint32_t now = foc_utils::get_us_tick();
    float dt = (float)(now - open_loop_timestamp) * 1.0e-6f;
    if(dt <= 0.0f || dt > 0.5f){dt = 1.0e-3f;}

    shaft_angle = foc_utils::normalize_angle(shaft_angle + velocity * dt);
    float e_angle = foc_utils::electrical_angle(shaft_angle, motor->PP);

    open_loop_timestamp = now;

    set_dq_voltage(motor->voltage_limit, 0.0f, e_angle);
}

/**
 * @brief 角度开环控制
 * 
 * @param angle 目标角度，单位：弧度
 */
void foc_controller::angle_openloop(float angle)
{
    if(!motor){return;}

    // set_dq_voltage(0.0f, 0.0f, angle);
}

/**
 * @brief 设置 dq 电压
 * 
 * @param Uq q 电压
 * @param Ud d 电压
 * @param electrical_angle 电角度
 * 
 * @note 关闭中断，确保线程安全设置
 */
void foc_controller::set_dq_voltage(float Uq, float Ud, float electrical_angle)
{
    if(!motor){return;}

    __disable_irq();
    motor->Uq = Uq;
    motor->Ud = Ud;
    motor->electrical_angle = electrical_angle;
    __enable_irq();
}
