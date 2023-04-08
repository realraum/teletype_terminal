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

namespace {
    constexpr const char TAG[] = "TTY";
}


void Teletype::init()
{
    gpio_set_direction(TTY_TX_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(TTY_TX_PIN, 1);
    this->mode = MODE_UNKNOWN;
    vTaskDelay(DELAY_BIT*5 / portTICK_PERIOD_MS);
    this->set_letter();
    vTaskDelay(DELAY_BIT*5 / portTICK_PERIOD_MS);
}

void Teletype::set_number()
{
    this->set_mode(MODE_NUMBER);
}

void Teletype::set_letter()
{
    this->set_mode(MODE_LETTER);
}

void Teletype::set_mode(tty_mode mode)
{
    if(mode == MODE_LETTER)
    {
        this->tx_bits(SWITCH_LETTER);
        this->mode = MODE_LETTER;
    }
    else if (mode == MODE_NUMBER)
    {
        this->tx_bits(SWITCH_NUMBER);
        this->mode = MODE_NUMBER;
    }
}

void Teletype::tx_bits(uint8_t bits)
{
    //printf("pattern: %x\n", bits);
    bool tx_bit = 0b0;

    // startbit
    gpio_set_level(TTY_TX_PIN, 0);
    vTaskDelay(DELAY_BIT / portTICK_PERIOD_MS);

    for(int i = 0; i<NUMBER_OF_BITS;i++)
    {
        tx_bit = (bits & (1 << i));
        //printf("%d\n", tx_bit);
        gpio_set_level(TTY_TX_PIN, tx_bit);
        vTaskDelay(DELAY_BIT / portTICK_PERIOD_MS);
    }

    // Stopbit
    gpio_set_level(TTY_TX_PIN, 1);
    vTaskDelay(DELAY_STOPBIT / portTICK_PERIOD_MS);
}

void Teletype::print_character(print_baudot_char bd_char)
{
    if(this->mode != MODE_BOTH_POSSIBLE && this->mode != bd_char.mode)
    {
        this->set_mode(bd_char.mode);
    }
    this->tx_bits(bd_char.bitcode);
}

void Teletype::print_string(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    ESP_LOGI(TAG, "%s", str.c_str());

    std::list<print_baudot_char> bd_char_list;

    for(auto it = str.begin(); it != str.end(); it++)
    {
        char c = *it;
        //TODO: decide if we REALLY want to change the contents of the string THAT MUCH
        if(c == '\n')
        {
            // change \n to CR + LF
            bd_char_list.push_back(this->convert_ascii_character_to_baudot('\r'));
            bd_char_list.push_back(this->convert_ascii_character_to_baudot('\n'));
            if(*(it+1) == '\r')it++; // if string contains \n\r the \r needs to be discarded -> swap to \r\n
        }
        else if (c == '\r' && *(it+1) != '\n')
        {
            // change \r to CR + LF
            bd_char_list.push_back(this->convert_ascii_character_to_baudot('\r'));
            bd_char_list.push_back(this->convert_ascii_character_to_baudot('\n'));
        }
        else
        {
            // normal character or direct
            bd_char_list.push_back(this->convert_ascii_character_to_baudot(c));
        }
    }

    for(auto it = bd_char_list.begin(); it != bd_char_list.end(); it++)
    {
        this->print_character(*it);
        ESP_LOGI(TAG, "%x %d %c", it->bitcode, it->mode, this->convert_baudot_char_to_ascii(it->bitcode));
    }
}


print_baudot_char Teletype::convert_ascii_character_to_baudot(char c)
{
    print_baudot_char bd_char;
    bd_char.bitcode = 0b0;
    bd_char.mode = MODE_UNKNOWN;
    bool found = false;
    for(int i = 0; i < NUMBER_OF_BAUDOT_CHARS && !found; i++)
    {
        if(baudot_alphabet[i].mode_letter == c)
        {
            bd_char.bitcode = baudot_alphabet[i].bitcode;
            bd_char.mode = MODE_LETTER;
            found = true;
        }
        if(baudot_alphabet[i].mode_number == c)
        {
            bd_char.bitcode = baudot_alphabet[i].bitcode;
            // if character is printable in both modes select the "both_modes" flag
            // --> space, newline, carriage return, NUL
            if(bd_char.mode != MODE_UNKNOWN)
            {
                bd_char.mode = MODE_BOTH_POSSIBLE;
            }
            else
            {
                bd_char.mode = MODE_NUMBER;
            }
            found = true;
        }
    }
    if(!found)
    {
        ESP_LOGI(TAG, "ERROR: letter not found in alphabet, printing space");
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
                ESP_LOGI(TAG, "ERROR: state unknown, returning 0");
            }
        }
    }

    return ret;
}

void Teletype::print_all_characters()
{
    print_baudot_char bd_char;
    for(int i = 0; i < NUMBER_OF_BAUDOT_CHARS; i++)
    {
        bd_char.bitcode = baudot_alphabet[i].bitcode;
        bd_char.mode = MODE_LETTER;
        this->print_character(bd_char);
        vTaskDelay(DELAY_STOPBIT / portTICK_PERIOD_MS);
    }
    for(int i = 0; i < NUMBER_OF_BAUDOT_CHARS; i++)
    {
        bd_char.bitcode = baudot_alphabet[i].bitcode;
        bd_char.mode = MODE_NUMBER;
        this->print_character(bd_char);
        vTaskDelay(DELAY_STOPBIT / portTICK_PERIOD_MS);
    }
}