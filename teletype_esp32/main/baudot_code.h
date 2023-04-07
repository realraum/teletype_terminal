#ifndef BAUDOT_CODE_H
#define BAUDOT_CODE_H
#include <inttypes.h>

#define NUMBER_OF_BITS 5

#define UNDEFINED_CHAR 0xFF
#define SWITCH_LETTER 0b11111
#define SWITCH_NUMBER 0b11011


struct character
{
    uint8_t bitcode;
    char mode_letter;
    char mode_number;
};

static character baudot_alphabet[] =
{
    {0b00011, 'A', '-'},
    {0b11001, 'B', '?'},
    {0b01110, 'C', ':'},
    {0b01001, 'D', UNDEFINED_CHAR},
    {0b00001, 'E', '3'},
    {0b01101, 'F', UNDEFINED_CHAR},
    {0b11010, 'G', UNDEFINED_CHAR},
    {0b10100, 'H', UNDEFINED_CHAR},
    {0b00110, 'I', '8'},
    {0b01011, 'J', 0x07}, // 0x07 = BELL
    {0b01111, 'K', '('},
    {0b10010, 'L', ')'},
    {0b11100, 'M', '.'},
    {0b01100, 'N', ','},
    {0b11000, 'O', '9'},
    {0b10110, 'P', '0'},
    {0b10111, 'Q', '1'},
    {0b01010, 'R', '4'},
    {0b00101, 'S', '\''},
    {0b10000, 'T', '5'},
    {0b00111, 'U', '7'},
    {0b11110, 'V', '='},
    {0b10011, 'W', '2'},
    {0b11101, 'X', '/'},
    {0b10101, 'Y', '6'},
    {0b10001, 'Z', '+'},
    {0b01000, '\r', '\r'},
    {0b00010, '\n', '\n'},
    {0b00100, ' ', ' '},
    {0b00000, 0x0, 0x0}
};

static char replacements[1][2]
{
    {'\"', '\''}
};

#endif // BAUDOT_CODE_H
