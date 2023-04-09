#include "sdkconfig.h"

// system includes
#include <algorithm>
#include <chrono>
#include <list>

// esp-idf includes
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// local includes
#include "teletype.hpp"

namespace {
constexpr const char TAG[] = "TTY";
} // namespace

Teletype::Teletype()
{
    esp_log_level_set(TAG, ESP_LOG_WARN);
    gpio_set_direction(TTY_RX_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(TTY_RX_PIN, 1);
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
    set_mode(MODE_LETTER);
}

void Teletype::set_mode(tty_mode_t mode)
{
    switch (mode)
    {
    case MODE_LETTER:
        tx_bits(SWITCH_LETTER);
        pr_mode = MODE_LETTER;
        break;
    case MODE_NUMBER:
        tx_bits(SWITCH_NUMBER);
        pr_mode = MODE_NUMBER;
        break;
    default:;
    }
}

void Teletype::tx_bits(uint8_t bits)
{
    ESP_LOGI(TAG, "pattern: %x\n", bits);
    bool tx_bit{false};

    // startbit
    gpio_set_level(TTY_RX_PIN, 0);
    usleep(DELAY_BIT * 1000);

    for (int i = 0; i < NUMBER_OF_BITS; i++)
    {
        tx_bit = (bits & (1 << i));
        gpio_set_level(TTY_RX_PIN, tx_bit);
        usleep(DELAY_BIT * 1000);
    }

    // Stopbit
    gpio_set_level(TTY_RX_PIN, 1);
    usleep(DELAY_STOPBIT * 1000);
}

void Teletype::print_character(print_baudot_char_t bd_char)
{
    if (pr_mode != MODE_BOTH_POSSIBLE && pr_mode != bd_char.mode)
    {
        set_mode(bd_char.mode);
    }
    tx_bits(bd_char.bitcode);
}

void Teletype::print_string(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), toupper);
    ESP_LOGI(TAG, "%s", str.c_str());

    std::list<print_baudot_char_t> bd_char_list;

    for (auto it = str.begin(); it != str.end(); it++)
    {
        char c = *it;
        //TODO: decide if we REALLY want to change the contents of the string THAT MUCH
        if (c == '\n')
        {
            // change \n to CR + LF
            bd_char_list.push_back(convert_ascii_character_to_baudot('\r'));
            bd_char_list.push_back(convert_ascii_character_to_baudot('\n'));
            if(*(it+1) == '\r') // if string contains \n\r the \r needs to be discarded -> swap to \r\n
                it++;
        }
        else if (c == '\r' && *(it+1) != '\n')
        {
            // change \r to CR + LF
            bd_char_list.push_back(convert_ascii_character_to_baudot('\r'));
            bd_char_list.push_back(convert_ascii_character_to_baudot('\n'));
        }
        else
        {
            // normal character or direct
            bd_char_list.push_back(convert_ascii_character_to_baudot(c));
        }
    }

    for (auto it = bd_char_list.begin(); it != bd_char_list.end(); it++)
    {
        print_character(*it);
        ESP_LOGI(TAG, "%x %d %c", it->bitcode, it->mode, convert_baudot_char_to_ascii(it->bitcode));
    }
}


print_baudot_char_t Teletype::convert_ascii_character_to_baudot(char c)
{
    c = static_cast<char>(toupper(c));

    print_baudot_char_t bd_char;
    bd_char.bitcode = 0b0;
    bd_char.mode = MODE_UNKNOWN;

    bool found{false};

    ESP_LOGI(TAG, "CHAR = '%c'", c);

    for (int i = 0; i < NUMBER_OF_BAUDOT_CHARS && !found; i++)
    {
        if (baudot_alphabet[i].mode_letter == c)
        {
            bd_char.bitcode = baudot_alphabet[i].bitcode;
            bd_char.mode = MODE_LETTER;
            found = true;
            ESP_LOGI(TAG, "LETTER %c", baudot_alphabet[i].mode_letter);
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
            ESP_LOGI(TAG, "NUMBER %c", baudot_alphabet[i].mode_number);
        }
    }

    if (!found)
    {
        ESP_LOGW(TAG, "ERROR: letter not found in alphabet, printing space");
        // bd_char = this->convert_ascii_character_to_baudot(' '); // TODO: avoid unneccesary mode change when printing space
        bd_char.mode = MODE_BOTH_POSSIBLE;
    }

    ESP_LOGI(TAG, "BITCODE = '%x'", bd_char.bitcode);
    return bd_char;
}

char Teletype::convert_baudot_char_to_ascii(uint8_t bits)
{
    bool found{false};

    char ret{0}
;
    // TODO: Remove this hack if we ever loopback locally
    if (bits == 0b11111)
        kb_mode = MODE_LETTER;
    else if (bits == 0b11011)
        kb_mode = MODE_NUMBER;
    else
    {
        for (int i = 0; i < NUMBER_OF_BAUDOT_CHARS && !found; i++) // found is unused
        {
            if (baudot_alphabet[i].bitcode == bits)
            {
                if(kb_mode == MODE_LETTER)
                {
                    ret = baudot_alphabet[i].mode_letter;
                }
                else if (kb_mode == MODE_NUMBER)
                {
                    ret = baudot_alphabet[i].mode_number;
                }
                else
                {
                    ESP_LOGI(TAG, "ERROR: state unknown, returning 0");
                }
            }
        }
    }
    return ret;
}

void Teletype::print_all_characters()
{
    print_baudot_char_t bd_char;

    for (auto &i : baudot_alphabet)
    {
        bd_char.bitcode = i.bitcode;
        bd_char.mode = MODE_LETTER;
        print_character(bd_char);
        vTaskDelay(DELAY_STOPBIT / portTICK_PERIOD_MS);
    }

    for (auto &i : baudot_alphabet)
    {
        bd_char.bitcode = i.bitcode;
        bd_char.mode = MODE_NUMBER;
        print_character(bd_char);
        vTaskDelay(DELAY_STOPBIT / portTICK_PERIOD_MS);
    }
}
