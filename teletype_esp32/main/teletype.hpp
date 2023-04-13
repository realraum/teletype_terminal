#ifndef __TELETYPE_H__
#define __TELETYPE_H__

// system includes
#include <string>

// esp-idf includes
#include <driver/gpio.h>
#include <hal/gpio_types.h>

// local includes
#include "baudot_code.h"

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
    Teletype(uint8_t baudrate, gpio_num_t rx_pin, gpio_num_t tx_pin, uint8_t max_chars);

    // Teletype printing functions
    void print_string(std::string str);
    void print_ascii_character(char c);
    void print_all_characters(); // for later use when we want to test the alphabet (print everything)

    // Teletype receiving functions (keyboard)
    char receive_ascii_character();
    
    // Conversions
    print_baudot_char_t convert_ascii_character_to_baudot(char c);
    char convert_baudot_char_to_ascii(uint8_t bits);

    // getter + setter
    uint8_t get_TTY_BAUDRATE();
    uint8_t get_TTY_MAX_CHARS_PAPER();
    gpio_num_t get_TTY_RX_PIN();
    gpio_num_t get_TTY_TX_PIN();

private:
    // Teletype properties (timing etc.)
    uint8_t TTY_BAUDRATE{};
    uint16_t DELAY_BIT{};
    uint16_t DELAY_STOPBIT{};
    uint8_t TTY_MAX_CHARS_PAPER{};

    // Hardware connections
    gpio_num_t TTY_RX_PIN{};
    gpio_num_t TTY_TX_PIN{};

    // State variables
    tty_mode_t kb_mode{}; // keyboard mode
    tty_mode_t pr_mode{}; // printer mode
    uint8_t characters_on_paper{};

    uint8_t rx_bits();
    void tx_bits(uint8_t bits);

    void set_number();
    void set_letter();
    void set_mode(tty_mode_t mode);

    void print_bd_character(print_baudot_char_t bd_char);
};

#endif
