#include "sdkconfig.h"
#include <stdio.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_log.h>
#include <chrono>
#include <memory>
#include "esp_timer.h"

#include "driver/gpio.h"
#include <string>

#include "baudot_code.h"
#include "teletype.hpp"

namespace {
    constexpr const char TAG[] = "MAIN";
}


extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Teletype Debug");

    Teletype tty;
    tty.init();

    std::string my_string = "the quick brown fox jumps over the lazy dog 1234567890\n";

    char* buf;
    while(1)
    {
        tty.print_string(my_string);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        //getLineInput(buf, 10);
    }

    ESP_LOGI(TAG, "Restarting now. ==================================================================\n");
    fflush(stdout);
    esp_restart();
}
