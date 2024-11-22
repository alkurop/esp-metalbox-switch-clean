#include "timer.hpp"

using namespace TMR;

static void onTimerCallback(void *arg)
{
    auto t = (Timer *)arg;
    t->onTimeOut();
}

void Timer::init(TimeoutListener timeoutListener)
{
    this->timeoutListener = timeoutListener;
    esp_timer_create_args_t timer_args = {
        .callback = &onTimerCallback,
        .arg = this,
        .name = "app timer"};
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &this->timerHandler));
};

Timer::~Timer()
{
    ESP_ERROR_CHECK(esp_timer_delete(this->timerHandler));
}

void Timer::startOneShot(uint32_t seconds)
{
    esp_timer_start_once(this->timerHandler, SEC_TO_MICRO(seconds));
};

void Timer::startOneShotMillis(uint32_t millis)
{
    esp_timer_start_once(this->timerHandler, MILLIS_TO_MICRO(millis));
};

void Timer::startPeriodicMillis(uint32_t millis)
{
    ESP_ERROR_CHECK(esp_timer_start_periodic(this->timerHandler, MILLIS_TO_MICRO(millis)));
};

void Timer::startPeriodic(uint32_t seconds)
{
    ESP_ERROR_CHECK(esp_timer_start_periodic(this->timerHandler, SEC_TO_MICRO(seconds)));
};
void Timer::onTimeOut()
{
    this->timeoutListener(this);
};

void Timer::stop()
{
    esp_timer_stop(this->timerHandler);
};
