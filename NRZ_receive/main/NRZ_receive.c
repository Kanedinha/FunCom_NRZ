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
#define RX GPIO_NUM_15

#define BAUD_RATE 5
#define CLOCK_PERIOD_MS 200 //  (1 / BAUD_RATE) * 1000

static const char *TAG = "NRZ_receive";

uint8_t rxBuffer = 0x00;
uint8_t count = 0;
uint8_t flag = 0;
uint8_t bitReceived = 0;

// static void IRAM_ATTR bit_detect_synchronous(void *args)
// {
//     if (count >= 8)
//     {
//         ESP_LOGE(TAG, "buffer full");
//         return;
//     }
//     bitReceived = gpio_get_level(RX) << 7;
//     // ESP_LOGI(TAG, "received bit: %d", bitReceived);
//     if (count < 7)
//     {
//         rxBuffer = (rxBuffer | bitReceived) >> 1;
//         count++;
//     }
//     else
//     {
//         rxBuffer = (rxBuffer | bitReceived);
//         count++;
//     }
//     if (count >= 8)
//     {
//         flag = 1;
//     }
// }

void bit_detect_asynchronous(void *args)
{
    while (1)
    {
        if (gpio_get_level(RX) == 1)
        {
            ESP_LOGI(TAG, "start bit detected");
            vTaskDelay(CLOCK_PERIOD_MS / portTICK_PERIOD_MS);
            for (uint8_t i = 0; i < 8; i++)
            {
                bitReceived = gpio_get_level(RX) << 7;
                ESP_LOGI(TAG, "bit received: %d", bitReceived/128);
                if (count < 7)
                {
                    rxBuffer = (rxBuffer | bitReceived) >> 1;
                    count++;
                }
                else
                {
                    rxBuffer = (rxBuffer | bitReceived);
                    count++;
                }
                if (count >= 8)
                {
                    flag = 1;
                }
                vTaskDelay(CLOCK_PERIOD_MS / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void reset_rx_buffer()
{
    flag = 0;
    count = 0;
    rxBuffer = 0x00;
}

void peripherical_init()
{
    gpio_install_isr_service(0);

    gpio_set_direction(CLK, GPIO_MODE_INPUT);
    gpio_pulldown_en(CLK);
    gpio_pullup_dis(CLK);
    // gpio_set_intr_type(CLK, GPIO_INTR_NEGEDGE);
    // gpio_isr_handler_add(CLK, bit_detect_synchronous, (void *)CLK);

    gpio_set_direction(RX, GPIO_MODE_INPUT);
    // gpio_pulldown_en(RX);
    // gpio_pullup_dis(RX);
    // gpio_set_intr_type(RX, GPIO_INTR_POSEDGE);
    // gpio_isr_handler_add(RX, bit_detect_asynchronous, (void *)RX);
}

void app_main(void)
{
    uint8_t clock_state = 0;
    uint8_t rxData = 0x00;
    peripherical_init();
    xTaskCreate(&bit_detect_asynchronous, "bit_detect_asynchronous", 16000, NULL, 20, NULL);
    ESP_LOGI(TAG, "peripherical init!!!");
    while (1)
    {
        if (flag)
        {
            rxData = rxBuffer;
            ESP_LOGI(TAG, "data received: %c", (char)rxData);
            reset_rx_buffer();
        }
        vTaskDelay(10);
    }
}
