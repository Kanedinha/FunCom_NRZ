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

#define CLK GPIO_NUM_4
#define TX GPIO_NUM_2

#define BAUD_RATE 5
#define CLOCK_PERIOD_MS 200 //  (1 / BAUD_RATE) * 1000

static const char *TAG = "NRZ_receive";

uint8_t read_byte_synchronous()
{
    bool clock_down = false;

}

void peripherical_init()
{
    gpio_set_direction(CLK, GPIO_MODE_INPUT);
    gpio_set_direction(TX, GPIO_MODE_INPUT);
}

void app_main(void)
{
    peripherical_init();
    while(gpio)
}
