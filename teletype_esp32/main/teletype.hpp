
#ifndef __TELETYPE_H__
#define __TELETYPE_H__

#include <string>
#include "baudot_code.h"
#include "driver/gpio.h"

#define TELETYPE_LOGIC_0 0b0
#define TELETYPE_LOGIC_1 0b1
#define TTY_BAUDRATE 50
#define DELAY_BIT 1000/TTY_BAUDRATE
#define DELAY_STOPBIT DELAY_BIT*1.5

#define TTY_TX_PIN GPIO_NUM_23


typedef enum tty_state {STATE_UNKNOWN = 0, STATE_LETTER = 1, STATE_NUMBER = 2};

class Teletype
{
public:
    void print_string(std::string str);
    void tx_bits(uint8_t bits);
    void init();

private:
    tty_state state;
};


#endif