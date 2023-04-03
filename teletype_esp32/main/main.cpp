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
#include "baudot_code.h"

namespace {
    constexpr const char TAG[] = "MAIN";
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Teletype Debug");
}
