#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"

#define CLK 14
#define TX 13

static const char *TAG = "NRZ";

bool clockState = 0;

void clock_generate()
{
    vTaskDelay(200 / portTICK_PERIOD_MS);
    clockState = !clockState;
    gpio_set_level(CLK, clockState);
    ESP_LOGI(TAG, "clock state: %d", clockState);
}

void peripherical_init()
{
    gpio_set_direction(CLK, GPIO_MODE_OUTPUT);
}

void app_main()
{
    peripherical_init();
    xTaskCreate(&clock_generate, "clock_generate", 1024, NULL, 5, NULL);
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}