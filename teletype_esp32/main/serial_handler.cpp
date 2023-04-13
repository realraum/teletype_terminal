
// esp-idf includes
#include <esp_log.h>

// local includes
#include "serial_handler.hpp"


namespace {
constexpr const char TAG[] = "SERIAL";
} // namespace


// static members
bool SerialHandler::flush_buffer;
std::mutex SerialHandler::uart_buffer_mutex;
Teletype* SerialHandler::tty;

SerialHandler::SerialHandler(Teletype* tty)
{
    if (tty != nullptr)
    {
        this->tty = tty;

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
    }
    else
    {
        ESP_LOGE(TAG, "ERROR: tty not defined.");
    }
}

[[noreturn]] void SerialHandler::uart_task_rx(void *pvParameters)
{
    ESP_LOGI(TAG, "Hello from the UART RX Task");
    char buf[1];
    while (true)
    {
        {
            std::lock_guard<std::mutex> lck(uart_buffer_mutex);
            if (flush_buffer)
            {
                ESP_LOGW(TAG, "FLUSHING RX BUFFER");
                uart_flush(UART_NUM_1);
                flush_buffer = false;
            }
        }
        int ret = uart_read_bytes(UART_NUM_1, &buf, 1, portMAX_DELAY);
        if (ret > 0)
        {
            //putc(buf[0], stdout);
            tty->print_ascii_character(buf[0]);
            if(buf[0] == '\n')
                tty->print_ascii_character('\r');
        }
    }
}

void SerialHandler::uart_task_tx(void *pvParameters)
{
    ESP_LOGI(TAG, "Hello from the UART TX Task");

    char out[] = {tty->receive_ascii_character()};
    if (out[0] != 0)
    {
        if (out[0] == ASCII_ETX)
        {
            std::lock_guard<std::mutex> lck(uart_buffer_mutex);
            flush_buffer = true;
        }
        uart_write_bytes(UART_NUM_1, &out, 1);
    }
    vTaskDelete(nullptr);
}

void IRAM_ATTR SerialHandler::data_isr_handler(void* arg)
{
    gpio_intr_disable(tty->get_TTY_TX_PIN());
    xTaskCreate(uart_task_tx, "UART Task TX", 4096, nullptr, 1, nullptr);
}