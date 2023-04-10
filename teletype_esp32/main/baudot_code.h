#ifndef BAUDOT_CODE_H
#define BAUDOT_CODE_H

// system includes
#include <inttypes.h>

#define NUMBER_OF_BITS 5
#define NUMBER_OF_BAUDOT_CHARS 30

// Special pattens (non printeable)
#define SWITCH_LETTER 0b11111
#define SWITCH_NUMBER 0b11011

// Special characters
#define UNDEFINED_CHAR '\xff'
#define ASCII_ETX '\x03' // CTRL + C
#define ASCII_BEL '\x07' // Bell

typedef enum {
    NO_INCREMENT_CHAR_COUNT,
    INCREMENT_CHAR_COUNT,
    RESET_CHAR_COUNT
} char_count_action_t;

struct character
{
    uint8_t bitcode;
    char mode_letter;
    char mode_number;
    char_count_action_t cc_action;
};

static character baudot_alphabet[] =
{
    { 0b00011, 'A', '-', INCREMENT_CHAR_COUNT },
    { 0b11001, 'B', '?', INCREMENT_CHAR_COUNT },
    { 0b01110, 'C', ':', INCREMENT_CHAR_COUNT },
    { 0b01001, 'D', ASCII_ETX, INCREMENT_CHAR_COUNT }, // DO NOT PRINT THIS CHARACTER IN NUMBER MODE (triggers identification)
    { 0b00001, 'E', '3', INCREMENT_CHAR_COUNT },
    { 0b01101, 'F', UNDEFINED_CHAR, INCREMENT_CHAR_COUNT }, // Can't send this char in number mode on the teletype (square)
    { 0b11010, 'G', UNDEFINED_CHAR, INCREMENT_CHAR_COUNT }, // Can't send this char in number mode on the teletype (square horizontal line)
    { 0b10100, 'H', UNDEFINED_CHAR, INCREMENT_CHAR_COUNT }, // Can't send this char in number mode on the teletype (square diagonal line)
    { 0b00110, 'I', '8', INCREMENT_CHAR_COUNT },
    { 0b01011, 'J', ASCII_BEL, INCREMENT_CHAR_COUNT }, // 0x07 = BELL, POSIX: '\a'
    { 0b01111, 'K', '(', INCREMENT_CHAR_COUNT },
    { 0b10010, 'L', ')', INCREMENT_CHAR_COUNT },
    { 0b11100, 'M', '.', INCREMENT_CHAR_COUNT },
    { 0b01100, 'N', ',', INCREMENT_CHAR_COUNT },
    { 0b11000, 'O', '9', INCREMENT_CHAR_COUNT },
    { 0b10110, 'P', '0', INCREMENT_CHAR_COUNT },
    { 0b10111, 'Q', '1', INCREMENT_CHAR_COUNT },
    { 0b01010, 'R', '4', INCREMENT_CHAR_COUNT },
    { 0b00101, 'S', '\'', INCREMENT_CHAR_COUNT },
    { 0b10000, 'T', '5', INCREMENT_CHAR_COUNT },
    { 0b00111, 'U', '7', INCREMENT_CHAR_COUNT },
    { 0b11110, 'V', '=', INCREMENT_CHAR_COUNT },
    { 0b10011, 'W', '2', INCREMENT_CHAR_COUNT },
    { 0b11101, 'X', '/', INCREMENT_CHAR_COUNT },
    { 0b10101, 'Y', '6', INCREMENT_CHAR_COUNT },
    { 0b10001, 'Z', '+', INCREMENT_CHAR_COUNT },
    { 0b01000, '\r', '\r', RESET_CHAR_COUNT },
    { 0b00010, '\n', '\n', NO_INCREMENT_CHAR_COUNT },
    { 0b00100, ' ', ' ', INCREMENT_CHAR_COUNT },
    { 0b00000, 0x0, 0x0, NO_INCREMENT_CHAR_COUNT }
};

static constexpr char replacements[][2]
{
    {'\"', '\''}
};

#endif // BAUDOT_CODE_H
