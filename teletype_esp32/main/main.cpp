#include "sdkconfig.h"

// system includes
#include <chrono>
#include <memory>
#include <string>
#include <thread>

// esp-idf includes
#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// local includes
#include "baudot_code.h"
#include "teletype.hpp"

namespace {
constexpr const char TAG[] = "MAIN";
Teletype* global_tty;
} // namespace

[[noreturn]] void uart_task_rx(void *pvParameters)
{
    ESP_LOGI(TAG, "Hello from the UART RX Task");
    char buf[1];
    while (true)
    {
        int ret = uart_read_bytes(UART_NUM_1, &buf, 1, portMAX_DELAY);
        if (ret > 0)
        {
            //putc(buf[0], stdout);
            global_tty->print_ascii_character(buf[0]);
            if(buf[0] == '\n')
                global_tty->print_ascii_character('\r');
        }
    }
}

void uart_task_tx(void *pvParameters)
{
    ESP_LOGI(TAG, "Hello from the UART TX Task");

    char out[] = {global_tty->receive_ascii_character()};
    if (out[0] != 0)
    {
        uart_write_bytes(UART_NUM_1, &out, 1);
    }
    vTaskDelete(nullptr);
}

void IRAM_ATTR data_isr_handler(void* arg)
{
    gpio_intr_disable(TTY_TX_PIN);
    xTaskCreate(uart_task_tx, "UART Task TX", 4096, nullptr, 1, nullptr);
}

extern "C" [[noreturn]] void app_main(void)
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
    uart_driver_install(UART_NUM_1, 1024, 1024, 0, nullptr, 0);

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << TTY_TX_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_POSEDGE,
    };

    gpio_config(&io_conf);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    global_tty = new Teletype();
    gpio_isr_handler_add(TTY_TX_PIN, data_isr_handler, nullptr);

    // --- Start UART task ---
    xTaskCreate(uart_task_rx, "UART Task RX", 4096, nullptr, 1, nullptr);

    // std::string my_string = "the quick brown fox jumps over the lazy dog 1234567890\n-?:().,\'=/+\a\n";

    // tty.print_string(my_string);
    // tty.print_all_characters();

    while (true)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
