#include "foc_motor.h"

#include "foc_utils.h"
#include "sensors/sensors.h"

/**
 * @brief 构造函数
 * 
 * @param tim_handle 时基定时句柄
 * @param en_gpio 使能引脚 GPIO 句柄
 * @param en_pin 使能引脚引脚号
 * @param PP 电机极对数
 * @param R 电机线电阻
 * @param KV KV 值
 */
foc_motor::foc_motor(TIM_HandleTypeDef *tim_handle, GPIO_TypeDef *en_gpio, uint16_t en_pin,
    uint8_t PP, float R, float KV)
    : tim_handle(tim_handle),
      en_gpio(en_gpio),
      en_pin(en_pin),
      PP(PP),
      R(R),
      KV(KV)
{
    controller.bind(this);
}

/**
 * @brief 初始化电机驱动
 */
void foc_motor::init()
{
    if(encoder_sensor){encoder_sensor->init();}
    if(current_sensor){current_sensor->init();}
    if(vbus_sensor){vbus_sensor->init();}

    if(voltage_power_supply < 0.0f){voltage_power_supply = 0.0f;}
    if(voltage_limit < 0.0f){voltage_limit = 0.0f;}
    if(voltage_sensor_align < 0.0f){voltage_sensor_align = 0.0f;}

    if(voltage_limit > voltage_power_supply){voltage_limit = voltage_power_supply;}

    HAL_TIM_PWM_Start(tim_handle, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(tim_handle, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(tim_handle, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(tim_handle, TIM_CHANNEL_4);

    if(!align())
    {
        disable();
    }
    else
    {
        enable();
    }

    __HAL_TIM_ENABLE_IT(tim_handle, TIM_IT_UPDATE);

    if(current_sensor){current_sensor->get_offset();}
}

/**
 * @brief 使能电机驱动
 */
void foc_motor::enable()
{
    if(!en_gpio || !en_pin){return;}
    HAL_GPIO_WritePin(en_gpio, en_pin, GPIO_PIN_SET);
    controller.is_enable = true;
}

/**
 * @brief 失能电机驱动
 */
void foc_motor::disable()
{
    if(!en_gpio || !en_pin){return;}
    HAL_GPIO_WritePin(en_gpio, en_pin, GPIO_PIN_RESET);
    controller.is_enable = false;
}

/**
 * @brief 链接传感器
 * 
 * @param sensor 传感器指针
 */
void foc_motor::link_encoder_sensor(encoder_sensors *encoder_sensor)
{
    this->encoder_sensor = encoder_sensor;
}

/**
 * @brief 链接电流传感器
 * 
 * @param current_sensor 电流传感器指针
 */
void foc_motor::link_current_sensor(current_sensors *current_sensor)
{
    this->current_sensor = current_sensor;
}

/**
 * @brief 链接母线电压传感器
 * 
 * @param vbus_sensor 母线电压传感器指针
 */
void foc_motor::link_vbus_sensor(vbus_sensors *vbus_sensor)
{
    this->vbus_sensor = vbus_sensor;
}

/**
 * @brief 更新电机状态
 */
void foc_motor::update()
{
    if(encoder_sensor){encoder_sensor->update();}
}

/**
 * @brief 循环执行电机控制
 */
void foc_motor::loop()
{
    if(current_sensor){current_sensor->update();}
    set_phase_voltage();
}

/**
 * @brief 根据控制模式执行目标控制
 * 
 * @param target 目标值
 */
void foc_motor::move(float target)
{
    if(controller_type == foc_controller::type::none){return;}
    controller.move(controller_type, target);
}

/**
 * @brief 对齐电机
 * 
 * @return true 成功
 * @return false 失败
 */
bool foc_motor::align()
{
    if(!encoder_sensor){return false;}

    enable();

    float angle;

    // 正转
    for(uint32_t i = 0; i <= 500; i++)
    {
        angle = _3PI_2 + _2PI * i / 500.0f;
        set_dq_voltage(voltage_sensor_align, 0.0f, angle);
        set_phase_voltage();
        foc_utils::delay_ms(2);
    }

    for(uint32_t i = 0; i < 100; i++)
    {
        encoder_sensor->update();
        foc_utils::delay_ms(1);
    }
    float mid_angle = encoder_sensor->get_full_angle();

    // 反转
    for(int32_t i = 500; i >= 0; i--)
    {
        angle = _3PI_2 + _2PI * i / 500.0f;
        set_dq_voltage(voltage_sensor_align, 0.0f, angle);
        set_phase_voltage();
        foc_utils::delay_ms(2);
    }

    for(uint32_t i = 0; i < 100; i++)
    {
        encoder_sensor->update();
        foc_utils::delay_ms(1);
    }
    float end_angle = encoder_sensor->get_full_angle();

    // 测试方向
    float moved = fabsf(mid_angle - end_angle);
    if((mid_angle == end_angle) || (moved < 0.02f)){return false;}
    else if(mid_angle < end_angle){sensor_direction = foc_controller::direction::CCW;}
    else{sensor_direction = foc_controller::direction::CW;}

    // 计算零点漂移角度
    set_dq_voltage(voltage_sensor_align, 0.0f, _3PI_2);
    set_phase_voltage();
    foc_utils::delay_ms(700);
    for(uint32_t i = 0; i < 100; i++)
    {
        encoder_sensor->update();
        foc_utils::delay_ms(1);
    }
    zero_electric_angle = foc_utils::normalize_angle(
        foc_utils::electrical_angle((float)sensor_direction * encoder_sensor->get_full_angle(), PP));
    foc_utils::delay_ms(20);

    set_dq_voltage(voltage_sensor_align, 0.0f, _3PI_2);
    set_phase_voltage();
    foc_utils::delay_ms(100);

    disable();

    return true;
}

/**
 * @brief 设置相电压
 */
void foc_motor::set_phase_voltage()
{
    float U_q = _constrain(Uq, -voltage_limit, voltage_limit);
    float U_d = _constrain(Ud, -voltage_limit, voltage_limit);
    float e_angle = electrical_angle;

    float U_out;
    if(U_d)
    {
        U_out = _sqrt(U_q * U_q + U_d * U_d) / voltage_power_supply;

        // 只有在使用快速计算 sin/cos/sqrt_approx 时才需要
        e_angle = foc_utils::normalize_angle(e_angle + atan2f(U_q, U_d));
    }
    else
    {
        U_out = U_q / voltage_power_supply;

        // 只有在使用快速计算 sin/cos/sqrt_approx 时才需要
        e_angle = foc_utils::normalize_angle(e_angle + _PI_2);
    }

    // 确保 U_out 为非负
    if(U_out < 0.0f)
    {
        U_out = -U_out;
        e_angle = foc_utils::normalize_angle(e_angle + _PI);
    }

    // 不允许 SVPWM 请求超过母线电压在线性区内能合成的最大电压矢量
    if(U_out > 0.577f){U_out = 0.577f;}
    

    float T0, T1, T2;
    float Ta, Tb, Tc;
    uint32_t sector = e_angle / _PI_3 + 1;

    T1 = _SQRT3 * foc_utils::sin(sector * _PI_3 - e_angle) * U_out;
    T2 = _SQRT3 * foc_utils::sin(e_angle - (float)(sector - 1) * _PI_3) * U_out;
    T0 = 1.0f - T1 - T2;

    switch (sector) {
        case 1:
            Ta = T1 + T2 + T0 / 2;
            Tb = T2 + T0 / 2;
            Tc = T0 / 2;
            break;

        case 2:
            Ta = T1 + T0 / 2;
            Tb = T1 + T2 + T0 / 2;
            Tc = T0 / 2;
            break;

        case 3:
            Ta = T0 / 2;
            Tb = T1 + T2 + T0 / 2;
            Tc = T2 + T0 / 2;
            break;

        case 4:
            Ta = T0 / 2;
            Tb = T1 + T0 / 2;
            Tc = T1 + T2 + T0 / 2;
            break;

        case 5:
            Ta = T2 + T0 / 2;
            Tb = T0 / 2;
            Tc = T1 + T2 + T0 / 2;
            break;

        case 6:
            Ta = T1 + T2 + T0 / 2;
            Tb = T0 / 2;
            Tc = T1 + T0 / 2;
            break;

        default:
            Ta = 0;
            Tb = 0;
            Tc = 0;
    }

    __HAL_TIM_SetCompare(tim_handle, TIM_CHANNEL_1, Tc * get_pwm_arr());
    __HAL_TIM_SetCompare(tim_handle, TIM_CHANNEL_2, Tb * get_pwm_arr());
    __HAL_TIM_SetCompare(tim_handle, TIM_CHANNEL_3, Ta * get_pwm_arr());
}
