
// system includes

// esp-idf includes
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// local includes
#include "mode_manager.hpp"
#include "serial_handler.hpp"
#include "teletype.hpp"


namespace {
constexpr const char TAG[] = "MODE MANAGER";
} // namespace

// static members
Teletype* ModeManager::tty{};
SerialHandler* ModeManager::serh{};

ModeManager::ModeManager(Teletype* tty, SerialHandler* serh)
{
    gpio_config_t button1_conf = {
        .pin_bit_mask = {1ULL << Button1},
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config_t button2_conf = {
        .pin_bit_mask = {1ULL << Button2},
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config_t button3_conf = {
        .pin_bit_mask = {1ULL << Button3},
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config_t led_red_conf = {
        .pin_bit_mask = {1ULL << LED_RED},
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&button1_conf);
    gpio_config(&button2_conf);
    gpio_config(&button3_conf);

    gpio_isr_handler_add(Button1, button_handler, nullptr);
    gpio_isr_handler_add(Button2, button_handler, nullptr);
    gpio_isr_handler_add(Button3, button_handler, nullptr);

    this->tty = tty;
    this->serh = serh;

    gpio_config(&led_red_conf);
    gpio_set_level(LED_RED, 0);
}

void ModeManager::button_task(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    int b1 = gpio_get_level(Button1);
    int b2 = gpio_get_level(Button2);
    int b3 = gpio_get_level(Button3);

    if (b1 == 0)
    {
        ESP_LOGI(TAG, "B1");
        if (serh->get_local_loopback_enabled())
        {
            serh->local_loop_disable();
            gpio_set_level(LED_RED, 0);
        }
        else
        {
            serh->local_loop_enable();
            gpio_set_level(LED_RED, 1);
        }
    }
    if (b2 == 0)
    {
        ESP_LOGI(TAG, "B2");
    }
    if (b3 == 0)
    {
        ESP_LOGI(TAG, "B3");
    }

    gpio_intr_enable(Button1);
    gpio_intr_enable(Button3);
    gpio_intr_enable(Button2);
    vTaskDelete(nullptr);
}

void IRAM_ATTR ModeManager::button_handler(void* arg)
{
    gpio_intr_disable(Button1);
    gpio_intr_disable(Button2);
    gpio_intr_disable(Button3);
    xTaskCreate(button_task, "Button Task", 4096, nullptr, 1, nullptr);
}
