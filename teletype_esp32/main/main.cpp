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
#include "driver/uart.h"
#include <string>

#include "baudot_code.h"
#include "teletype.hpp"

namespace {
    constexpr const char TAG[] = "MAIN";
}

// ###############

//#define UART_EXAMPLE_CODE
#ifdef UART_EXAMPLE_CODE
// Copied from examples (TODO: move somewhere else (usb terminal class ????))
#define ECHO_TEST_TXD (CONFIG_EXAMPLE_UART_TXD)
#define ECHO_TEST_RXD (CONFIG_EXAMPLE_UART_RXD)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM      UART_NUM_0
#define ECHO_UART_BAUD_RATE     115200
#define ECHO_TASK_STACK_SIZE    1024

#define BUF_SIZE (1024)

static void echo_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    //ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
        // Write data back to the UART
        uart_write_bytes(ECHO_UART_PORT_NUM, (const char *) data, len);
        if (len) {
            data[len] = '\0';
            ESP_LOGI(TAG, "Recv str: %s", (char *) data);
        }
    }
}

// ###############
#endif

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Teletype Debug");

    Teletype tty;
    tty.init();

    std::string my_string = "the quick brown fox jumps over the lazy dog 1234567890\n-?:().,\'=/+\a\n";

    tty.print_string(my_string);
    //tty.print_all_characters();
    while(1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }


    ESP_LOGI(TAG, "Restarting now. ==================================================================\n");
    fflush(stdout);
    esp_restart();
}
