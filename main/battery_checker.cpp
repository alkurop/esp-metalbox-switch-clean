#include "battery_checker.hpp"

using namespace App;

BatteryChecker::BatteryChecker(gpio_num_t enablePin, gpio_num_t checkPin, uint16_t timeoutSeconds)
{
    this->enablePin = enablePin;
    this->checkPin = checkPin;
    this->timeoutSeconds = timeoutSeconds;
    this->timeoutListener = [this](Timer *timer)
    { this->runBatteryCheckerTask(); };
};

void BatteryChecker::init(BatteryListener listener)
{
    this->listener = listener;
    this->timer.init(this->timeoutListener);
};

void BatteryChecker::stop()
{
    timer.startPeriodic(timeoutSeconds);
};
void BatteryChecker::start()
{
    timer.stop();
};
