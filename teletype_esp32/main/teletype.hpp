
#ifndef __TELETYPE_H__
#define __TELETYPE_H__

#include <string>
#include "baudot_code.h"
#include "driver/gpio.h"

#define TTY_BAUDRATE 50
#define DELAY_BIT 1000/TTY_BAUDRATE
#define DELAY_STOPBIT DELAY_BIT*1.5

#define TTY_TX_PIN GPIO_NUM_22


typedef enum tty_mode {MODE_UNKNOWN = 0, MODE_LETTER = 1, MODE_NUMBER = 2, MODE_BOTH_POSSIBLE = 3};

typedef struct print_baudot_char
{
    uint8_t bitcode;
    tty_mode mode;
};

class Teletype
{
public:
    void print_string(std::string str);
    void tx_bits(uint8_t bits);
    void init();

    void print_character(print_baudot_char bd_char);
    print_baudot_char convert_ascii_character_to_baudot(char c);
    char convert_baudot_char_to_ascii(uint8_t bits);

private:
    tty_mode mode;

    void set_number();
    void set_letter();
    void set_mode(tty_mode mode);
};


#endif