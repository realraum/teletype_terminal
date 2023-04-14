#ifndef __MODE_MANAGER_H__
#define __MODE_MANAGER_H__

// system includes
#include <thread>

// esp-idf includes
#include <driver/gpio.h>

// local includes
#include "serial_handler.hpp"
#include "teletype.hpp"

class ModeManager
{
public:
    ModeManager(Teletype* tty, SerialHandler* serh);

    void PollButtons();

private:
    static const gpio_num_t Button1 = GPIO_NUM_25;
    static const gpio_num_t Button2 = GPIO_NUM_26;
    static const gpio_num_t Button3 = GPIO_NUM_27;
    static const gpio_num_t LED_RED = GPIO_NUM_13;

    static Teletype* tty;
    static SerialHandler* serh;

    static void IRAM_ATTR button_handler(void* arg);

    static void button_task(void *pvParameters);

};

#endif