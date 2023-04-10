#ifndef BAUDOT_CODE_H
#define BAUDOT_CODE_H

// system includes
#include <inttypes.h>

#define NUMBER_OF_BITS 5
#define NUMBER_OF_BAUDOT_CHARS 30
#define UNDEFINED_CHAR '\xff'
#define SWITCH_LETTER 0b11111
#define SWITCH_NUMBER 0b11011


struct character
{
    uint8_t bitcode;
    char mode_letter;
    char mode_number;
    bool increment_line_cnt;
};

static character baudot_alphabet[] =
{
    { 0b00011, 'A', '-', true },
    { 0b11001, 'B', '?', true },
    { 0b01110, 'C', ':', true },
    { 0b01001, 'D', UNDEFINED_CHAR, true }, // DO NOT PRINT THIS CHARACTER IN NUMBER MODE (triggers identification)
    { 0b00001, 'E', '3', true },
    { 0b01101, 'F', UNDEFINED_CHAR, true },
    { 0b11010, 'G', UNDEFINED_CHAR, true },
    { 0b10100, 'H', UNDEFINED_CHAR, true },
    { 0b00110, 'I', '8', true },
    { 0b01011, 'J', 0x07, true }, // 0x07 = BELL, POSIX: '\a'
    { 0b01111, 'K', '(', true },
    { 0b10010, 'L', ')', true },
    { 0b11100, 'M', '.', true },
    { 0b01100, 'N', ',', true },
    { 0b11000, 'O', '9', true },
    { 0b10110, 'P', '0', true },
    { 0b10111, 'Q', '1', true },
    { 0b01010, 'R', '4', true },
    { 0b00101, 'S', '\'', true },
    { 0b10000, 'T', '5', true },
    { 0b00111, 'U', '7', true },
    { 0b11110, 'V', '=', true },
    { 0b10011, 'W', '2', true },
    { 0b11101, 'X', '/', true },
    { 0b10101, 'Y', '6', true },
    { 0b10001, 'Z', '+', true },
    { 0b01000, '\r', '\r', false },
    { 0b00010, '\n', '\n', false },
    { 0b00100, ' ', ' ', true },
    { 0b00000, 0x0, 0x0, false }
};

static constexpr char replacements[][2]
{
    {'\"', '\''}
};

#endif // BAUDOT_CODE_H
