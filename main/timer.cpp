#include "timer.hpp"

using namespace App;

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
        .name = "one shot"};
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &this->timerHandler));
};

Timer::~Timer()
{
    ESP_ERROR_CHECK(esp_timer_delete(this->timerHandler));
}

void Timer::startOneShot(uint32_t seconds)
{
    ESP_ERROR_CHECK(esp_timer_start_once(this->timerHandler, SEC_TO_MICRO(seconds)));
};
void Timer::onTimeOut()
{
    this->timeoutListener(this);
};

void Timer::stop()
{
    ESP_ERROR_CHECK(esp_timer_stop(this->timerHandler));
};
