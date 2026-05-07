#include "foc_utils.h"

// 正弦表，放大了 10000 倍
static const int16_t sine_array[200] = {
    0, 79, 158, 237, 316, 395, 473, 552, 631, 710,
    789, 867, 946, 1024, 1103, 1181, 1260, 1338, 1416, 1494,
    1572, 1650, 1728, 1806, 1883, 1961, 2038, 2115, 2192, 2269,
    2346, 2423, 2499, 2575, 2652, 2728, 2804, 2879, 2955, 3030,
    3105, 3180, 3255, 3329, 3404, 3478, 3552, 3625, 3699, 3772,
    3845, 3918, 3990, 4063, 4135, 4206, 4278, 4349, 4420, 4491,
    4561, 4631, 4701, 4770, 4840, 4909, 4977, 5046, 5113, 5181,
    5249, 5316, 5382, 5449, 5515, 5580, 5646, 5711, 5775, 5839,
    5903, 5967, 6030, 6093, 6155, 6217, 6279, 6340, 6401, 6461,
    6521, 6581, 6640, 6699, 6758, 6815, 6873, 6930, 6987, 7043,
    7099, 7154, 7209, 7264, 7318, 7371, 7424, 7477, 7529, 7581,
    7632, 7683, 7733, 7783, 7832, 7881, 7930, 7977, 8025, 8072,
    8118, 8164, 8209, 8254, 8298, 8342, 8385, 8428, 8470, 8512,
    8553, 8594, 8634, 8673, 8712, 8751, 8789, 8826, 8863, 8899,
    8935, 8970, 9005, 9039, 9072, 9105, 9138, 9169, 9201, 9231,
    9261, 9291, 9320, 9348, 9376, 9403, 9429, 9455, 9481, 9506,
    9530, 9554, 9577, 9599, 9621, 9642, 9663, 9683, 9702, 9721,
    9739, 9757, 9774, 9790, 9806, 9821, 9836, 9850, 9863, 9876,
    9888, 9899, 9910, 9920, 9930, 9939, 9947, 9955, 9962, 9969,
    9975, 9980, 9985, 9989, 9992, 9995, 9997, 9999, 10000, 10000
};

/**
 * @brief 计算正弦值
 * 
 * @param angle 角度，单位为弧度，范围为 [0, 2π]
 * 
 * @return float 正弦值
 */
float foc_utils::sin(float angle)
{
    if(angle < _PI_2)
    {
        return 0.0001f * (float)sine_array[_round(126.6873f * angle)];
    }
    else if(angle < _PI)
    {
        return 0.0001f * (float)sine_array[398 - _round(126.6873f * angle)];
    }
    else if(angle < _3PI_2)
    {
        return -0.0001f * (float)sine_array[398 - _round(126.6873f * angle)];
    }
    else
    {
        return -0.0001f * (float)sine_array[796 - _round(126.6873f * angle)];
    }
}

/**
 * @brief 计算余弦值
 * 
 * @param angle 角度，单位为弧度，范围为 [0, 2π]
 * 
 * @return float 余弦值
 */
float foc_utils::cos(float angle)
{
    float angle_sin = angle + _PI_2;
    angle_sin = angle_sin > _2PI ? angle_sin - _2PI : angle_sin;
    return foc_utils::sin(angle_sin);
}

/**
 * @brief 归一化角度
 * 
 * @param angle 角度，单位为弧度
 * 
 * @return float 归一化后的角度
 */
float foc_utils::normalize_angle(float angle)
{
    float angle_norm = fmod(angle, _2PI);
    return angle_norm >= 0 ? angle_norm : angle_norm + _2PI;
}

/**
 * @brief 计算电气角度
 * 
 * @param shaft_angle 轴角度，单位为弧度
 * @param pole_pairs 极对数
 * 
 * @return float 电气角度
 */
float foc_utils::electrical_angle(float shaft_angle, uint8_t pole_pairs)
{
    return shaft_angle * pole_pairs;
}

/**
 * @brief 快速计算平方根
 * 
 * @param x 输入值
 * 
 * @return float 平方根值
 */
float foc_utils::sqrt_approx(float x)
{
    if(x <= 0.0f){return 0.0f;}

    float y = x;
    float x_half = x * 0.5f;

    int32_t i = *(int32_t *)&y;
    i = 0x5f375a86 - (i >> 1);
    y = *(float *)&i;

    y = y * (1.5f - x_half * y * y);

    return x * y;
}
