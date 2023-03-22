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

#define CLK GPIO_NUM_4
#define TX GPIO_NUM_2

#define BAUD_RATE 5
#define CLOCK_PERIOD_MS 200 //  (1 / BAUD_RATE) * 1000

static const char *TAG = "NRZ";

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
    gpio_set_direction(CLK, GPIO_MODE_OUTPUT);
    gpio_set_direction(TX, GPIO_MODE_OUTPUT);

    hd44780 hd44780_t;
}

void app_main()
{
    esp_err_t ret;

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    peripherical_init();

    ESP_LOGI(TAG, "sending data...");
    ESP_LOGI(TAG, "next byte:");
    send_byte_synchronous('O');
    ESP_LOGI(TAG, "next byte:");
    send_byte_synchronous('l');
    ESP_LOGI(TAG, "next byte:");
    send_byte_synchronous('á');
    ESP_LOGI(TAG, "next byte:");
    send_byte_synchronous(' ');
    ESP_LOGI(TAG, "next byte:");
    send_byte_synchronous('m');
    ESP_LOGI(TAG, "next byte:");
    send_byte_synchronous('u');
    ESP_LOGI(TAG, "next byte:");
    send_byte_synchronous('n');
    ESP_LOGI(TAG, "next byte:");
    send_byte_synchronous('d');
    ESP_LOGI(TAG, "next byte:");
    send_byte_synchronous('o');

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}