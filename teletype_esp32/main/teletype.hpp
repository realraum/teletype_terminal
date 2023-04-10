#ifndef __TELETYPE_H__
#define __TELETYPE_H__

// system includes
#include <string>

// esp-idf includes
#include <driver/gpio.h>
#include <hal/gpio_types.h>

// local includes
#include "baudot_code.h"

constexpr const int TTY_BAUDRATE = 50;
constexpr const int DELAY_BIT = (1000 / TTY_BAUDRATE);
constexpr const int DELAY_STOPBIT = (DELAY_BIT * 1.5);

constexpr const auto TTY_RX_PIN{gpio_num_t::GPIO_NUM_22};
constexpr const auto TTY_TX_PIN{gpio_num_t::GPIO_NUM_23};


typedef enum {
    MODE_UNKNOWN,
    MODE_LETTER,
    MODE_NUMBER,
    MODE_BOTH_POSSIBLE
} tty_mode_t;

typedef struct
{
    uint8_t bitcode;
    tty_mode_t mode;
} print_baudot_char_t;

class Teletype
{
public:
    Teletype();

    void print_string(std::string str);

    static uint8_t rx_bits();
    static void tx_bits(uint8_t bits);
    static print_baudot_char_t convert_ascii_character_to_baudot(char c);

    void print_character(print_baudot_char_t bd_char);
    char convert_baudot_char_to_ascii(uint8_t bits);

    void print_all_characters();

private:
    tty_mode_t kb_mode{}; // keyboard mode
    tty_mode_t pr_mode{}; // printer mode

    void set_number();
    void set_letter();
    void set_mode(tty_mode_t mode);
};

#endif
