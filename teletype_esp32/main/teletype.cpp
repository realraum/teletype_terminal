#include "sdkconfig.h"
#include <stdio.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_log.h>
#include <chrono>
#include <memory>

#include <algorithm>
#include <list>

#include "teletype.hpp"



void Teletype::init()
{
    gpio_set_direction(TTY_TX_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(TTY_TX_PIN, TELETYPE_LOGIC_1);
    this->mode = MODE_UNKNOWN;
    vTaskDelay(DELAY_BIT*5 / portTICK_PERIOD_MS);
    this->set_letter();
    vTaskDelay(DELAY_BIT*5 / portTICK_PERIOD_MS);
}

void Teletype::set_number()
{
    this->tx_bits(SWITCH_NUMBER);
    this->mode = MODE_NUMBER;
}

void Teletype::set_letter()
{
    this->tx_bits(SWITCH_LETTER);
    this->mode = MODE_LETTER;
}

void Teletype::tx_bits(uint8_t bits)
{
    printf("pattern: %x\n", bits);
    bool tx_bit = 0b0;

    // startbit
    gpio_set_level(TTY_TX_PIN, TELETYPE_LOGIC_0);
    vTaskDelay(DELAY_BIT / portTICK_PERIOD_MS);

    for(int i = NUMBER_OF_BITS - 1; i>=0;i--)
    {
        tx_bit = (bits & (1 << i));
        printf("%d\n", tx_bit);
        gpio_set_level(TTY_TX_PIN, tx_bit & TELETYPE_LOGIC_1);
        vTaskDelay(DELAY_BIT / portTICK_PERIOD_MS);
    }

    // Stopbit
    gpio_set_level(TTY_TX_PIN, TELETYPE_LOGIC_1);
    vTaskDelay(DELAY_STOPBIT / portTICK_PERIOD_MS);
}

void Teletype::print_string(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    str.append("\r"); //TODO: only append \r when there's a \n and ONLY a \n at the end of a line (Linux stuff)
    printf("%s\n", str.c_str());

    std::list<print_baudot_char> bd_char_list;

    for(auto it = str.begin(); it != str.end(); it++)
    {
        char c = *it;
        bd_char_list.push_back(this->convert_ascii_character_to_baudot(c));
    }

    //TODO: change this to a print character function (we want to be able to place individual chars)
    for(auto it = bd_char_list.begin(); it != bd_char_list.end(); it++)
    {
        printf("%x %d %c\n", it->bitcode, it->mode, this->convert_baudot_char_to_ascii(it->bitcode));
        this->tx_bits(it->bitcode);
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);

}


print_baudot_char Teletype::convert_ascii_character_to_baudot(char c)
{
    print_baudot_char bd_char;
    bool found = false;
    for(int i = 0; i < NUMBER_OF_BAUDOT_CHARS && !found; i++)
    {
        if(baudot_alphabet[i].mode_letter == c)
        {
            bd_char.bitcode = baudot_alphabet[i].bitcode;
            bd_char.mode = MODE_LETTER;
            found = true;
        }
        else if (baudot_alphabet[i].mode_number == c)
        {
            bd_char.bitcode = baudot_alphabet[i].bitcode;
            bd_char.mode = MODE_NUMBER;
            found = true;
        }
    }
    if(!found)
    {
        printf("ERROR: letter not found in alphabet, printing space\n");
        bd_char = this->convert_ascii_character_to_baudot(' '); // TODO: avoid unneccesary mode change when printing space
    }
    return bd_char;
}

char Teletype::convert_baudot_char_to_ascii(uint8_t bits)
{
    bool found = false;
    char ret = 0;
    for(int i = 0; i < NUMBER_OF_BAUDOT_CHARS && !found; i++)
    {
        if(baudot_alphabet[i].bitcode == bits)
        {
            if(this->mode == MODE_LETTER)
            {
                ret = baudot_alphabet[i].mode_letter;
            }
            else if (this->mode == MODE_NUMBER)
            {
                ret = baudot_alphabet[i].mode_number;
            }
            else
            {
                printf("ERROR: state unknown, returning 0");
            }
        }
    }

    return ret;
}