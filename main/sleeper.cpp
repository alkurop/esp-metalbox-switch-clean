#include "sleeper.hpp"
#include "esp_log.h"

void App::Sleeper::sleep()
{
    esp_rom_printf("Fell to sleep");
};
void App::Sleeper::wake()
{
    esp_rom_printf("Woke up");
};
