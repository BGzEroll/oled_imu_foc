#ifndef EVENT_H
#define EVENT_H

#include "stm32f4xx_hal.h"

class event
{
    public:
        event(uint32_t period, void (*proc)(uint32_t));
        event(uint32_t period, void (*proc_void)());

    public:
        void start();
        void stop();
        static void loop();

    public:
        static bool post(uint32_t id, uint32_t data = 0);
        static void register_handler(void (*handler)(uint32_t, uint32_t));
        static uint32_t drops();

    private:
        void run(uint32_t tick);
        static void poll(uint32_t now);
        static void dispatch(uint8_t max_events);
        static bool push_timer(event *target);

    private:
        uint32_t last_time = 0;
        uint32_t period = 0;
        void (*proc)(uint32_t) = nullptr;
        void (*proc_void)() = nullptr;
        event *next = nullptr;
        bool running = false;
        bool listed = false;
        volatile bool queued = false;
};

#endif
