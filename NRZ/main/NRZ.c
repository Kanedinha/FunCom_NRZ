#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"

static const char *TAG = "NRZ";

#define LCD_EN GPIO_NUM_4
#define LCD_RS GPIO_NUM_5
#define LCD_D4 GPIO_NUM_18
#define LCD_D5 GPIO_NUM_19
#define LCD_D6 GPIO_NUM_21
#define LCD_D7 GPIO_NUM_22

void lcd_pulse_enable(void)
{
    gpio_set_level(LCD_EN, 1);
    gpio_set_level(LCD_EN, 0);
}

void lcd_send_nibble(uint8_t nibble)
{
    gpio_set_level(LCD_D4, (nibble >> 0) & 1);
    gpio_set_level(LCD_D5, (nibble >> 1) & 1);
    gpio_set_level(LCD_D6, (nibble >> 2) & 1);
    gpio_set_level(LCD_D7, (nibble >> 3) & 1);
    lcd_pulse_enable();
}

void lcd_send_byte(uint8_t data, uint8_t rs)
{
    gpio_set_level(LCD_RS, rs);
    lcd_send_nibble(data >> 4);
    lcd_send_nibble(data & 0x0F);
}

void lcd_init(void)
{
    gpio_pad_select_gpio(LCD_EN);
    gpio_pad_select_gpio(LCD_RS);
    gpio_pad_select_gpio(LCD_D4);
    gpio_pad_select_gpio(LCD_D5);
    gpio_pad_select_gpio(LCD_D6);
    gpio_pad_select_gpio(LCD_D7);

    gpio_set_direction(LCD_EN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_RS, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_D4, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_D5, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_D6, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_D7, GPIO_MODE_OUTPUT);

    ets_delay_us(50000);
    lcd_send_nibble(0x03);
    ets_delay_us(4500);
    lcd_send_nibble(0x03);
    ets_delay_us(4500);
    lcd_send_nibble(0x03);
    ets_delay_us(150);
    lcd_send_nibble(0x02);
    ets_delay_us(150);

    lcd_send_byte(0x28, 0); // 4 bits, 2 linhas, 5x8 fonte
    lcd_send_byte(0x0C, 0); // display ligado, cursor desligado, piscando desligado
    lcd_send_byte(0x06, 0); // modo de entrada: incrementa cursor, não desloca display
    lcd_send_byte(0x01, 0); // limpa display
    ets_delay_us(2000);
}

void app_main()
{

}