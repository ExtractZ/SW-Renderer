#ifndef LCD_H
#define LCD_H

#include "main.h"
#include <stdint.h>

#define LCD_WIDTH 240
#define LCD_HEIGHT 240
#define LCD_Y_OFFSET 0
/*
This is the definitions of the LCD driver for the SPI display
*/

void lcd_init(SPI_HandleTypeDef *hspi); //turn display on and setup
void lcd_cmd(uint8_t cmd); //basic send function. low DC. commands are one byte
void lcd_data(const uint8_t *data, uint32_t len);// high DC. 8bit = pointer. 32bit = length
void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);// sets the pixels, makes window
void lcd_push_dma(const uint16_t *buf, uint32_t count); //sends a frame without clogging CPU. RGB565. buffer is pointer to pixels. coount = nb of pixels
int lcd_busy(void); //checks if DMA still sending
void lcd_fill(uint16_t color); //fills screen with one color, one RGB565 value

#endif