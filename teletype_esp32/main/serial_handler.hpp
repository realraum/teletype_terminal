#ifndef __SERIAL_HANDLER_H__
#define __SERIAL_HANDLER_H__

// system includes
#include <memory>
#include <mutex>
#include <thread>

// esp-idf includes
#include <driver/uart.h>

// local includes
#include "teletype.hpp"

class SerialHandler
{
public:
    SerialHandler(Teletype* tty);

    [[noreturn]] static void uart_task_rx(void *pvParameters);
    static void uart_task_tx(void *pvParameters);

    static void IRAM_ATTR data_isr_handler(void* arg);

private:
    static bool flush_buffer;
    static std::mutex uart_buffer_mutex;
    static Teletype* tty;
    
        
};

#endif