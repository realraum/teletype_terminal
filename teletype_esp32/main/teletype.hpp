
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

#define TTY_TX_PIN GPIO_NUM_22


typedef enum tty_state {MODE_UNKNOWN = 0, MODE_LETTER = 1, MODE_NUMBER = 2};
typedef struct print_baudot_char
{
    uint8_t bitcode;
    tty_state mode;
};

class Teletype
{
public:
    void print_string(std::string str);
    void tx_bits(uint8_t bits);
    void init();

    print_baudot_char convert_ascii_character_to_baudot(char c);
    char convert_baudot_char_to_ascii(uint8_t bits);

private:
    tty_state mode;

    void set_number();
    void set_letter();
};


#endif