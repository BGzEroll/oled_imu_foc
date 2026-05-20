#ifndef VBUS_ADC_H
#define VBUS_ADC_H

#include "stm32f4xx_hal.h"
#include "../sensors.h"

class vbus_adc : public vbus_sensors
{
    public:
        vbus_adc(ADC_HandleTypeDef *adc_handle);

    public:
        void init() override;

    private:
        float get_raw_vbus() override;

    private:
        ADC_HandleTypeDef *adc_handle;
        volatile uint16_t vbus = 0;
};

#endif
