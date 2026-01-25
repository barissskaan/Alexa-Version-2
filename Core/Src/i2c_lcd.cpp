/**
 * @file    i2c_lcd.cpp
 * @brief   I2C LCD Driver Implementation
 */

#include "i2c_lcd.h"
#include <string.h>

// Static variables
static I2C_HandleTypeDef* lcd_i2c = NULL;
static uint8_t lcd_addr = LCD_I2C_ADDR << 1;  // Left shift for HAL
static uint8_t lcd_backlight = LCD_BACKLIGHT_ON;
static uint8_t lcd_display_ctrl = LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF;

// Private functions
static void LCD_WriteNibble(uint8_t nibble, uint8_t mode);
static void LCD_WriteByte(uint8_t data, uint8_t mode);
static void LCD_SendCommand(uint8_t cmd);
static void LCD_SendData(uint8_t data);

/**
 * @brief Write 4-bit nibble to LCD via I2C
 */
static void LCD_WriteNibble(uint8_t nibble, uint8_t mode) {
    uint8_t data = (nibble & 0xF0) | mode | lcd_backlight;
    
    // Send data with EN high
    uint8_t data_en = data | 0x04;  // EN = 1
    HAL_I2C_Master_Transmit(lcd_i2c, lcd_addr, &data_en, 1, 100);
    HAL_Delay(1);
    
    // Send data with EN low
    HAL_I2C_Master_Transmit(lcd_i2c, lcd_addr, &data, 1, 100);
    HAL_Delay(1);
}

/**
 * @brief Write byte to LCD (two 4-bit nibbles)
 */
static void LCD_WriteByte(uint8_t data, uint8_t mode) {
    LCD_WriteNibble(data & 0xF0, mode);      // High nibble
    LCD_WriteNibble((data << 4) & 0xF0, mode); // Low nibble
}

/**
 * @brief Send command to LCD
 */
static void LCD_SendCommand(uint8_t cmd) {
    LCD_WriteByte(cmd, 0x00);  // RS = 0 for command
    if (cmd == LCD_CLEAR || cmd == LCD_HOME) {
        HAL_Delay(2);  // Clear/Home need more time
    }
}

/**
 * @brief Send data to LCD
 */
static void LCD_SendData(uint8_t data) {
    LCD_WriteByte(data, 0x01);  // RS = 1 for data
}

/**
 * @brief Initialize LCD
 */
bool LCD_Init(I2C_HandleTypeDef* hi2c) {
    if (hi2c == NULL) {
        return false;
    }
    
    lcd_i2c = hi2c;
    
    // Wait for LCD to power up
    HAL_Delay(50);
    
    // Initialize in 4-bit mode
    LCD_WriteNibble(0x30, 0x00);  // Function set: 8-bit mode
    HAL_Delay(5);
    LCD_WriteNibble(0x30, 0x00);
    HAL_Delay(1);
    LCD_WriteNibble(0x30, 0x00);
    HAL_Delay(1);
    LCD_WriteNibble(0x20, 0x00);  // Function set: 4-bit mode
    HAL_Delay(1);
    
    // Configure LCD
    LCD_SendCommand(LCD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8_DOTS);
    LCD_SendCommand(LCD_DISPLAY_CTRL | LCD_DISPLAY_OFF);
    LCD_SendCommand(LCD_CLEAR);
    LCD_SendCommand(LCD_ENTRY_MODE | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DEC);
    LCD_SendCommand(LCD_DISPLAY_CTRL | lcd_display_ctrl);
    
    lcd_backlight = LCD_BACKLIGHT_ON;
    
    return true;
}

/**
 * @brief Clear display
 */
void LCD_Clear(void) {
    LCD_SendCommand(LCD_CLEAR);
    HAL_Delay(2);
}

/**
 * @brief Set cursor position
 */
void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t address = (row == 0) ? 0x00 : 0x40;
    address += col;
    LCD_SendCommand(LCD_DDRAM_ADDR | address);
}

/**
 * @brief Print string
 */
void LCD_Print(const char* str) {
    while (*str) {
        LCD_SendData(*str++);
    }
}

/**
 * @brief Print string at position
 */
void LCD_PrintAt(uint8_t row, uint8_t col, const char* str) {
    LCD_SetCursor(row, col);
    LCD_Print(str);
}

/**
 * @brief Control backlight
 */
void LCD_Backlight(bool state) {
    lcd_backlight = state ? LCD_BACKLIGHT_ON : LCD_BACKLIGHT_OFF;
    
    // Send dummy write to update backlight
    uint8_t data = lcd_backlight;
    HAL_I2C_Master_Transmit(lcd_i2c, lcd_addr, &data, 1, 100);
}

/**
 * @brief Display on/off
 */
void LCD_Display(bool state) {
    if (state) {
        lcd_display_ctrl |= LCD_DISPLAY_ON;
    } else {
        lcd_display_ctrl &= ~LCD_DISPLAY_ON;
    }
    LCD_SendCommand(LCD_DISPLAY_CTRL | lcd_display_ctrl);
}
