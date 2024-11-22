#pragma once

#include <functional>
#include <esp_timer.h>

#define SEC_TO_MICRO(x) (x) * 1000000
#define MILLIS_TO_MICRO(x) (x) * 1000
#define MICRO_TO_SEC(x) (uint16_t)((x) / 1000000)

namespace TMR
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
        void startOneShot(uint32_t millis);
        void startOneShotMillis(uint32_t seconds);
        void startPeriodic(uint32_t seconds);
        void startPeriodicMillis(uint32_t millis);
        void stop();
        void onTimeOut();
    };

}
