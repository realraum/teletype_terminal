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

void uart_task_rx( void * pvParameters )
{
    auto tty = std::unique_ptr<Teletype>{(Teletype*)pvParameters};
    ESP_LOGI(TAG, "Hello from the UART RX Task");
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

void uart_task_tx( void * pvParameters )
{
    auto tty = std::unique_ptr<Teletype>{(Teletype*)pvParameters};
    ESP_LOGI(TAG, "Hello from the UART TX Task");
    TickType_t xLastWakeTime = xTaskGetTickCount ();
    xTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(10)); // Wait till we are in the middle of Startbit
    if(gpio_get_level(TTY_TX_PIN) == 1)
    {
        uint8_t result = 0;
        for(int i = 0; i<5; i++)
        {
            xTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(20));
            result += ((1-gpio_get_level(TTY_TX_PIN)) << i);
        }
        xTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(30));
        char out[] = {(char)tolower(tty->convert_baudot_char_to_ascii(result))};
        uart_write_bytes(UART_NUM_1, &out, 1);
    }
    else
        ESP_LOGW(TAG, "ERROR! Start bit not 0! False trigger?");
    gpio_intr_enable(TTY_TX_PIN);
    vTaskDelete( NULL );
}

void IRAM_ATTR data_isr_handler(void* arg)
{
    gpio_intr_disable(TTY_TX_PIN);
    Teletype* tty = (Teletype*)arg;
    xTaskCreate(uart_task_tx, "UART Task TX", 4096, tty, 1, NULL);
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

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << TTY_TX_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_POSEDGE,
    };

    gpio_config(&io_conf);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    gpio_isr_handler_add(TTY_TX_PIN, data_isr_handler, tty);

    // --- Start UART task ---
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
