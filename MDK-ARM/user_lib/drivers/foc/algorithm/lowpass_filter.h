#ifndef LOWPASS_FILTER_H
#define LOWPASS_FILTER_H

#include "stm32f4xx_hal.h"

class lowpass_filter
{
    public:
        lowpass_filter(float Tf);
    
    public:
        float operator()(float input);

    public:
        void set_Tf(float Tf);

    private:
        float Tf;
        uint32_t timestamp_prev;
        float y_prev = 0.0f;
};

#endif
