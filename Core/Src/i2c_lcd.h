/**
 * @file    i2c_lcd.h
 * @brief   I2C LCD Driver (HD44780 + PCF8574)
 * 
 * 16x2 LCD display driver for I2C interface
 * I2C Address: 0x27 (default) or 0x3F
 * Connections: PB8 (SCL), PB9 (SDA)
 */

#ifndef I2C_LCD_H
#define I2C_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

// LCD Commands
#define LCD_CLEAR           0x01
#define LCD_HOME            0x02
#define LCD_ENTRY_MODE      0x04
#define LCD_DISPLAY_CTRL    0x08
#define LCD_CURSOR_SHIFT    0x10
#define LCD_FUNCTION_SET    0x20
#define LCD_CGRAM_ADDR      0x40
#define LCD_DDRAM_ADDR      0x80

// Entry Mode Flags
#define LCD_ENTRY_RIGHT     0x00
#define LCD_ENTRY_LEFT      0x02
#define LCD_ENTRY_SHIFT_INC 0x01
#define LCD_ENTRY_SHIFT_DEC 0x00

// Display Control Flags
#define LCD_DISPLAY_ON      0x04
#define LCD_DISPLAY_OFF     0x00
#define LCD_CURSOR_ON       0x02
#define LCD_CURSOR_OFF      0x00
#define LCD_BLINK_ON        0x01
#define LCD_BLINK_OFF       0x00

// Function Set Flags
#define LCD_8BIT_MODE       0x10
#define LCD_4BIT_MODE       0x00
#define LCD_2LINE           0x08
#define LCD_1LINE           0x00
#define LCD_5x10_DOTS       0x04
#define LCD_5x8_DOTS        0x00

// Backlight Control
#define LCD_BACKLIGHT_ON    0x08
#define LCD_BACKLIGHT_OFF   0x00

// I2C Address (try both if one doesn't work)
#define LCD_I2C_ADDR        0x27  // or 0x3F

/**
 * @brief  Initialize LCD display
 * @param  hi2c: I2C handle
 * @retval true if successful
 */
bool LCD_Init(I2C_HandleTypeDef* hi2c);

/**
 * @brief  Clear display
 */
void LCD_Clear(void);

/**
 * @brief  Set cursor position
 * @param  row: Row number (0 or 1)
 * @param  col: Column number (0-15)
 */
void LCD_SetCursor(uint8_t row, uint8_t col);

/**
 * @brief  Print string to LCD
 * @param  str: String to print
 */
void LCD_Print(const char* str);

/**
 * @brief  Print string at specific position
 * @param  row: Row number (0 or 1)
 * @param  col: Column number (0-15)
 * @param  str: String to print
 */
void LCD_PrintAt(uint8_t row, uint8_t col, const char* str);

/**
 * @brief  Control backlight
 * @param  state: true = on, false = off
 */
void LCD_Backlight(bool state);

/**
 * @brief  Display on/off
 * @param  state: true = on, false = off
 */
void LCD_Display(bool state);

#ifdef __cplusplus
}
#endif

#endif /* I2C_LCD_H */
