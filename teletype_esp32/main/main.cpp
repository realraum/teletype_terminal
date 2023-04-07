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

#include "baudot_code.h"
#include "teletype.hpp"

namespace {
    constexpr const char TAG[] = "MAIN";
}

uint8_t bits_R = 0b01010;
uint8_t bits_Y = 0b10101;



extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Teletype Debug");

    printf("Hello world\n\n");

    Teletype tty;
    tty.init();

    for(int i = 3; i >0;i--)
    {
        printf("R\n");
        tty.tx_bits(bits_R);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Y\n");
        tty.tx_bits(bits_Y);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // Just a simple test to check if we can get bits in the correct order / speed

    for (int i = 2; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
