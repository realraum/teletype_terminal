#include "sdkconfig.h"
#include <stdio.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_log.h>
#include <chrono>
#include <memory>
#include <thread>
#include "esp_timer.h"
#include "esp_sleep.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include <string>

#include "baudot_code.h"
#include "teletype.hpp"

namespace {
    constexpr const char TAG[] = "MAIN";
}

void uart_task_tx( void * pvParameters )
{
    auto tty = std::unique_ptr<Teletype>{(Teletype*)pvParameters};
    ESP_LOGI(TAG, "Hello from the UART TX Task");
    char buf[1];
    while(1)
    {
        int ret = uart_read_bytes(UART_NUM_1, &buf, 1, portMAX_DELAY);
        if(ret > 0)
        {
            //putc(buf[0], stdout);
            print_baudot_char bd_char = tty->convert_ascii_character_to_baudot(buf[0]);
            tty->print_character(bd_char);
            if(buf[0] == '\n')
                tty->print_character(tty->convert_ascii_character_to_baudot('\r'));
        }
    }
    vTaskDelete( NULL );
}

void uart_task_rx( void * pvParameters )
{
    auto tty = std::unique_ptr<Teletype>{(Teletype*)pvParameters};
    ESP_LOGI(TAG, "Hello from the UART RX Task");
    while(0)
    {
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    vTaskDelete( NULL );
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Teletype Debug");
    esp_log_level_set(TAG, ESP_LOG_WARN);

    // --- UART 2 (stty) config ---
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, 18, 19, 25, 26);
    uart_driver_install(UART_NUM_1, 1024, 1024, 0, NULL, 0);

    Teletype* tty = new Teletype{};
    tty->init();

    // --- Start UART task ---
    xTaskCreate(uart_task_tx, "UART Task TX", 4096, tty, 1, NULL);
    xTaskCreate(uart_task_rx, "UART Task RX", 4096, tty, 1, NULL);

    //std::string my_string = "the quick brown fox jumps over the lazy dog 1234567890\n-?:().,\'=/+\a\n";

    //ty.print_string(my_string);
    //tty.print_all_characters();
    while(1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }


    ESP_LOGI(TAG, "Restarting now. ==================================================================\n");
    fflush(stdout);
    esp_restart();
}
