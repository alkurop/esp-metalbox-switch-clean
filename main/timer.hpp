#pragma once
#include <functional>
#include <esp_log.h>
#include "esp_timer.h"

#define SEC_TO_NANO(x) (x) * 1000000
#define NANO_TO_SEC(x) (uint16_t)((x) / 1000000)

namespace App
{
    class Timer;

    using TimeoutListener = std::function<void(Timer *)>;

    class Timer
    {
    private:
        esp_timer_handle_t timerHandler;
        TimeoutListener timeoutListener;

    public:
        ~Timer();
        void init(TimeoutListener listener);
        void startOneShot(uint8_t seconds);
        void stop();
        void onTimeOut();
    };

}
