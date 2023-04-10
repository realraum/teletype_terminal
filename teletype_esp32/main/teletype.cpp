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
    //esp_log_level_set(TAG, ESP_LOG_WARN);
    gpio_set_direction(TTY_RX_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(TTY_RX_PIN, 1);

    kb_mode = MODE_UNKNOWN;
    pr_mode = MODE_UNKNOWN;
    vTaskDelay(DELAY_BIT*5 / portTICK_PERIOD_MS);
    set_letter();
    vTaskDelay(DELAY_BIT*5 / portTICK_PERIOD_MS);
    // initialize with CR + LF
    print_ascii_character('\r');
    print_ascii_character('\n');
    characters_on_paper = 0;
}

void Teletype::set_number()
{
    set_mode(MODE_NUMBER);
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

uint8_t Teletype::rx_bits()
{
    uint8_t result{0};
    ESP_LOGI(TAG, "Hello from the RX Bits");
    TickType_t xLastWakeTime = xTaskGetTickCount();
    xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10)); // Wait till we are in the middle of Startbit
    if (gpio_get_level(TTY_TX_PIN) == 1)
    {
        for (int i = 0; i<5; i++)
        {
            xTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(20));
            result += ((1-gpio_get_level(TTY_TX_PIN)) << i);
        }
        xTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(30));
    }
    else
        ESP_LOGW(TAG, "ERROR! Start bit not 0! False trigger?");
    gpio_intr_enable(TTY_TX_PIN);

    return result;
}

void Teletype::print_ascii_character(char c)
{
    print_bd_character(convert_ascii_character_to_baudot(c)); // TODO: error checking (need to decide how)
}

void Teletype::print_bd_character(print_baudot_char_t bd_char)
{
    if (pr_mode != MODE_BOTH_POSSIBLE && pr_mode != bd_char.mode)
    {
        set_mode(bd_char.mode);
    }
    tx_bits(bd_char.bitcode);
    switch (bd_char.cc_action)
    {
        case INCREMENT_CHAR_COUNT:
            characters_on_paper++;
            if (characters_on_paper > TTY_MAX_CHARS_PAPER)
            {
                ESP_LOGE(TAG, "ERROR: maximum characters on paper exceeded, printing newline");
                this->print_string("\r\n");
                characters_on_paper = 0;
            }
            break;
        case RESET_CHAR_COUNT:
            characters_on_paper = 0;
            break;
        default:
            break;
    }
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
        print_bd_character(*it);
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

        if (baudot_alphabet[i].mode_number == c)
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
        if (found)
        {
            bd_char.cc_action = baudot_alphabet[i].cc_action;
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

    char ret{0};
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
                switch(kb_mode)
                {
                case MODE_LETTER:
                    ret = baudot_alphabet[i].mode_letter;
                    break;
                case MODE_NUMBER:
                    ret = baudot_alphabet[i].mode_number;
                    break;
                default:
                    ESP_LOGI(TAG, "ERROR: state unknown, returning 0"); 
                }
            }
        }
    }
    return ret;
}


char Teletype::receive_ascii_character()
{
    uint8_t bits = rx_bits();
    char ret = static_cast<char>(tolower(convert_baudot_char_to_ascii(bits))); // TODO: erro checking (need to decide how)

    return ret;
}

void Teletype::print_all_characters()
{
    print_baudot_char_t bd_char;

    for (auto &i : baudot_alphabet)
    {
        bd_char.bitcode = i.bitcode;
        bd_char.mode = MODE_LETTER;
        print_bd_character(bd_char);
        vTaskDelay(DELAY_STOPBIT / portTICK_PERIOD_MS);
    }

    for (auto &i : baudot_alphabet)
    {
        bd_char.bitcode = i.bitcode;
        bd_char.mode = MODE_NUMBER;
        print_bd_character(bd_char);
        vTaskDelay(DELAY_STOPBIT / portTICK_PERIOD_MS);
    }
}
