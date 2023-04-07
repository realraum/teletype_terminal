#include "sdkconfig.h"
#include <stdio.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_log.h>
#include <chrono>
#include <memory>

#include <algorithm>

#include "teletype.hpp"



void Teletype::init()
{
    gpio_set_direction(TTY_TX_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(TTY_TX_PIN, TELETYPE_LOGIC_1);
}

void Teletype::tx_bits(uint8_t bits)
{
    printf("pattern: %x\n", bits);
    bool tx_bit = 0b0;

    // startbit
    gpio_set_level(TTY_TX_PIN, TELETYPE_LOGIC_0);
    vTaskDelay(DELAY_BIT / portTICK_PERIOD_MS);

    for(int i = NUMBER_OF_BITS - 1; i>=0;i--)
    {
        tx_bit = (bits & (1 << i));
        printf("%d\n", tx_bit);
        gpio_set_level(TTY_TX_PIN, tx_bit & 0b1);
        vTaskDelay(DELAY_BIT / portTICK_PERIOD_MS);
    }

    // Stopbit
    gpio_set_level(TTY_TX_PIN, TELETYPE_LOGIC_1);
    vTaskDelay(DELAY_STOPBIT / portTICK_PERIOD_MS);
}

void Teletype::print_string(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    printf("%s \n", str.c_str());
}