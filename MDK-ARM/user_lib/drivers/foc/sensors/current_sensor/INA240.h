#ifndef INA240_H
#define INA240_H

#include "stm32f4xx_hal.h"
#include "../sensors.h"

class INA240 : public current_sensors
{
    public:
        INA240(ADC_HandleTypeDef *adc_handle,
            uint32_t injected_rank_1 = 0,
            float shunt_resistance = 0.0f, float gain = 1.0f);
        INA240(ADC_HandleTypeDef *adc_handle,
            uint32_t injected_rank_1 = 0, uint32_t injected_rank_2 = 0,
            float shunt_resistance = 0.0f, float gain = 1.0f);
        INA240(ADC_HandleTypeDef *adc_handle,
            uint32_t injected_rank_1 = 0, uint32_t injected_rank_2 = 0, uint32_t injected_rank_3 = 0,
            float shunt_resistance = 0.0f, float gain = 1.0f);

    public:
        void init() override;
        void get_offset() override;
        void update() override;

    private:
        float *get_raw_current() override;
        float *get_raw_voltage() override;

    private:
        ADC_HandleTypeDef *adc_handle;
        float shunt_resistance;
        float gain;
        uint32_t injected_rank[3] = {0};
        uint16_t raw_data[3] = {0};
        float offset_voltage[3] = {0.0f};
        uint8_t rank_num = 0;
        float current[3] = {0.0f};
        float voltage[3] = {0.0f};
};

#endif
