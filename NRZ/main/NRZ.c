#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include <rom/ets_sys.h>
#include "driver/gpio.h"
#include <esp_types.h>
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_log.h"
#include <esp_task_wdt.h>
#include <esp_rom_gpio.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"

#include "nvs_flash.h"

#include "hd44780.h"

#define CLK GPIO_NUM_4
#define TX GPIO_NUM_2

#define BAUD_RATE 5
#define CLOCK_PERIOD_MS 200 //  (1 / BAUD_RATE) * 1000

static const char *TAG = "NRZ";
const char *dataWord = "Olá mundo";
bool lastCharIsExtra = false;

uint8_t newChar[8] = {
    0b00010,
    0b00100,
    0b01110,
    0b00001,
    0b01111,
    0b10001,
    0b01111,
    0b00000};

hd44780_t lcd;

bool clockState = 0;

void clock_generate()
{
    vTaskDelay(CLOCK_PERIOD_MS / portTICK_PERIOD_MS);
    clockState = !clockState;
    gpio_set_level(CLK, clockState);

    vTaskDelay(CLOCK_PERIOD_MS / portTICK_PERIOD_MS);
    clockState = !clockState;
    gpio_set_level(CLK, clockState);
}

void send_byte_asynchronous(uint8_t data)
{
    // ASCII values verification
    if ((data < 0) && (data > 255))
    {
        ESP_LOGE(TAG, "Data out of range %d", data);
        return;
    }

    // Copy the data
    uint8_t dataToSend = data;
    uint8_t bitToSend = 0;

    // Start bit
    gpio_set_level(TX, 1);
    vTaskDelay(CLOCK_PERIOD_MS / portTICK_PERIOD_MS);

    // Send byte
    for (uint8_t i = 0; i < 8; i++)
    {
        bitToSend = dataToSend & 0x01;
        ESP_LOGI(TAG, "send bit: %d", bitToSend);

        hd44780_gotoxy(&lcd, 7 - i, 1);
        hd44780_putc(&lcd, bitToSend + 48);

        gpio_set_level(TX, bitToSend);
        vTaskDelay(CLOCK_PERIOD_MS / portTICK_PERIOD_MS);
        dataToSend = dataToSend >> 1;
    }

    gpio_set_level(TX, 0);
    vTaskDelay(CLOCK_PERIOD_MS / portTICK_PERIOD_MS);

    return;
}

void send_byte_synchronous(uint8_t data)
{
    // ASCII values verification
    if ((data < 0) && (data > 255))
    {
        ESP_LOGE(TAG, "Data out of range %d", data);
        return;
    }

    // Copy the data
    uint8_t dataToSend = data;
    uint8_t bitToSend = 0;

    // Send byte
    for (uint8_t i = 0; i < 8; i++)
    {
        bitToSend = dataToSend & 0x01;
        ESP_LOGI(TAG, "send bit: %d", bitToSend);
        gpio_set_level(TX, bitToSend);
        clock_generate();
        dataToSend = dataToSend >> 1;
    }
    return;
}

void peripherical_init()
{
    esp_err_t ret = ESP_OK;
    gpio_set_direction(CLK, GPIO_MODE_OUTPUT);
    gpio_set_direction(TX, GPIO_MODE_OUTPUT);

    lcd.pins.rs = GPIO_NUM_32;
    lcd.pins.e = GPIO_NUM_33;
    lcd.pins.d4 = GPIO_NUM_26;
    lcd.pins.d5 = GPIO_NUM_27;
    lcd.pins.d6 = GPIO_NUM_14;
    lcd.pins.d7 = GPIO_NUM_12;
    lcd.font = HD44780_FONT_5X8;
    lcd.write_cb = NULL;
    lcd.lines = 2;

    hd44780_init(&lcd);

    ret = hd44780_upload_character(&lcd, 1, newChar);
    if (ret == ESP_OK)
        ESP_LOGI(TAG, "New Char");
    else
        ESP_LOGE(TAG, "Char Upload Error");
}

void app_main()
{
    esp_err_t ret;
    uint8_t dataWordLength = strlen(dataWord);
    uint8_t extraChar = 0;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    peripherical_init();

    ESP_LOGI(TAG, "sending data...");
    ESP_LOGI(TAG, "word length: %d", dataWordLength);
    hd44780_control(&lcd, true, true, true);

    // ---------- sync -------------

    // for (uint8_t i = 0; i < dataWordLength; i++)
    // {
    //     hd44780_gotoxy(&lcd, 1, 1);
    //     hd44780_puts(&lcd, "        ");
    //     ESP_LOGI(TAG, "next byte:");
    //     hd44780_gotoxy(&lcd, i, 0);
    //     hd44780_putc(&lcd, dataWord[i]);
    //     send_byte_synchronous(dataWord[i]);
    // }

    // ---------- async -------------

    for (uint8_t i = 0; i < dataWordLength; i++)
    {
        hd44780_gotoxy(&lcd, 0, 1);
        hd44780_puts(&lcd, "        ");
        ESP_LOGI(TAG, "next byte: %d", dataWord[i]);
        hd44780_gotoxy(&lcd, i - extraChar, 0);
        if (dataWord[i] == 195)
        {
            extraChar += 1;
            ESP_LOGI(TAG, "Extra Char!!!");
            lastCharIsExtra = true;
            hd44780_putc(&lcd, 96);
        }
        else if (lastCharIsExtra)
        {
            switch ((uint8_t)dataWord[i])
            {
            case 161:
                ESP_LOGI(TAG, "printing A");
                hd44780_putc(&lcd, 1);
                break;
            case 169:
                break;
            case 173:
                break;
            case 179:
                break;
            case 186:
                break;
            default:
                extraChar -= 1;
                hd44780_gotoxy(&lcd, i - extraChar, 0);
                hd44780_putc(&lcd, dataWord[i]);
                break;
            }

            lastCharIsExtra = false;
        }
        else
        {
            lastCharIsExtra = false;
            hd44780_putc(&lcd, dataWord[i]);
        }

        send_byte_asynchronous(dataWord[i]);
    }

    hd44780_gotoxy(&lcd, 8, 1);

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}