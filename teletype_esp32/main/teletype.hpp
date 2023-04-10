#ifndef __TELETYPE_H__
#define __TELETYPE_H__

// system includes
#include <string>

// esp-idf includes
#include <driver/gpio.h>
#include <hal/gpio_types.h>

// local includes
#include "baudot_code.h"

// Teletype proteries (timing etc.)
constexpr const int TTY_BAUDRATE = 50;
constexpr const int DELAY_BIT = (1000 / TTY_BAUDRATE);
constexpr const int DELAY_STOPBIT = (DELAY_BIT * 1.5);
constexpr const int TTY_MAX_CHARS_PAPER = 68; // TODO: check this on the machine
// Hardware connections
constexpr const auto TTY_RX_PIN{gpio_num_t::GPIO_NUM_22};
constexpr const auto TTY_TX_PIN{gpio_num_t::GPIO_NUM_23};


typedef enum {
    MODE_UNKNOWN,
    MODE_LETTER,
    MODE_NUMBER,
    MODE_BOTH_POSSIBLE
} tty_mode_t;


// TODO: unify print_baudot_char_t and character from baudot_code.h
//       to one datatype that can be used in both places (alphabet and printing / de-/encoding)
typedef struct
{
    uint8_t bitcode;
    tty_mode_t mode;
    char_count_action_t cc_action;
} print_baudot_char_t;

class Teletype
{
public:
    Teletype();

    void print_string(std::string str);
    void print_ascii_character(char c);
    void print_all_characters(); // for later use when we want to test the alphabet (print everything)

    char receive_ascii_character();

    static print_baudot_char_t convert_ascii_character_to_baudot(char c);
    char convert_baudot_char_to_ascii(uint8_t bits);

private:
    tty_mode_t kb_mode{}; // keyboard mode
    tty_mode_t pr_mode{}; // printer mode
    uint8_t characters_on_paper{};

    static uint8_t rx_bits();
    static void tx_bits(uint8_t bits);

    void set_number();
    void set_letter();
    void set_mode(tty_mode_t mode);

    void print_bd_character(print_baudot_char_t bd_char);
};

#endif
