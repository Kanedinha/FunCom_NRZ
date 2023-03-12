#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include <rom/ets_sys.h>
#include <esp_types.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include <esp_task_wdt.h>
#include <esp_rom_gpio.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"

#include "nvs_flash.h"

#define CLK 14
#define TX 13

static const char *TAG = "NRZ";

bool clockState = 0;

void clock_generate(void *arg)
{
    vTaskDelay(200 / portTICK_PERIOD_MS);
    clockState = !clockState;
    gpio_set_level(CLK, clockState);
    ESP_LOGI(TAG, "clock state: %d", clockState);
    vTaskDelete(NULL);
}

void peripherical_init()
{
    gpio_set_direction(CLK, GPIO_MODE_OUTPUT);
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

    while (1)
    {
        xTaskCreate(&clock_generate, "clock_generate", 1024, NULL, 5, NULL);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}