#include "sdkconfig.h"

// system includes
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

// esp-idf includes
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// local includes
#include "serial_handler.hpp"
#include "teletype.hpp"

namespace {
constexpr const char TAG[] = "MAIN";
Teletype* global_tty;
SerialHandler* global_serial_handler;
} // namespace


extern "C" [[noreturn]] void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Teletype Debug");
    esp_log_level_set(TAG, ESP_LOG_WARN);

    global_tty = new Teletype(50, GPIO_NUM_22, GPIO_NUM_23, 68);
    global_serial_handler = new SerialHandler(global_tty);


    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << global_tty->get_TTY_TX_PIN()),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_POSEDGE,
    };

    gpio_config(&io_conf);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    gpio_isr_handler_add(global_tty->get_TTY_TX_PIN(), global_serial_handler->data_isr_handler, nullptr);

    // --- Start UART task ---
    xTaskCreate(global_serial_handler->uart_task_rx, "UART Task RX", 4096, nullptr, 1, nullptr);

    // std::string my_string = "the quick brown fox jumps over the lazy dog 1234567890\n-?:().,\'=/+\a\n";

    // tty.print_string(my_string);
    // tty.print_all_characters();

    while (true)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
