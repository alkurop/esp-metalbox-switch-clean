#include "battery_checker.hpp"

using namespace App;

void BatteryChecker::init(BatteryListener listener)
{
    this->listener = listener;
};
