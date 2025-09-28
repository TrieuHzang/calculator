#include "liquidcrystal_i2c.h"
#include <string.h>

/* Mapping HIGH-NIBBLE:
 * Bit7..0:  D7 D6 D5 D4  BL EN RW RS
 *           P7 P6 P5 P4  P3 P2 P1 P0
 * -> Khi g?i 1 nibble, d?t nibble vào P4..P7 (bits 7..4) và thêm RS/BL,
 *    sau dó pulse EN (P2).
 */

extern I2C_HandleTypeDef hi2c1;

/* State */
static uint8_t _addr = DEVICE_ADDR;         /* 8-bit (7-bit<<1) cho HAL */
static uint8_t _backlight = LCD_BACKLIGHT;  /* b?t BL m?c d?nh */
static uint8_t _displayfunction = 0;
static uint8_t _displaycontrol  = 0;
static uint8_t _displaymode     = 0;
static uint8_t _numlines        = 2;

/* ===== Low-level ===== */
static void i2cWrite(uint8_t byte)
{
    (void)HAL_I2C_Master_Transmit(&hi2c1, _addr, &byte, 1, 20);
}

static void expanderWrite(uint8_t data)
{
    i2cWrite((uint8_t)(data | _backlight));
}

static void pulseEnable(uint8_t data)
{
    expanderWrite((uint8_t)(data | ENABLE));                // EN=1
    HAL_Delay(1);
    expanderWrite((uint8_t)(data & (uint8_t)~ENABLE));      // EN=0
    HAL_Delay(1);
}

/* Ghi 4 bit dã d?t ? v? trí P4..P7 (high nibble) kèm c? RS/BL */
static void write4bits(uint8_t data)
{
    expanderWrite(data);
    pulseEnable(data);
}

/* mode: 0=cmd, 1=data. V?i mapping high-nibble:
   - Nibble cao: value&0xF0 (dã ? bits 7..4)
   - Nibble th?p: (value<<4)&0xF0 (dua lên bits 7..4)
*/
static void send(uint8_t value, uint8_t mode)
{
    uint8_t high = (uint8_t)(value & 0xF0);
    uint8_t low  = (uint8_t)((value << 4) & 0xF0);
    uint8_t rsflag = (mode ? RS : 0x00);

    write4bits((uint8_t)(high | rsflag));
    write4bits((uint8_t)(low  | rsflag));
}

static void command(uint8_t value) { send(value, 0); }
static void writeData(uint8_t val) { send(val, 1); }

/* ===== Public API ===== */
void HD44780_Init(uint8_t rows)
{
    _numlines = rows ? rows : 2;
    HAL_Delay(50);

    _displayfunction = (uint8_t)(LCD_4BITMODE | ((_numlines > 1) ? LCD_2LINE : LCD_1LINE) | LCD_5x8DOTS);

    /* Chu?i kh?i t?o 4-bit chu?n (dua nibble 0x3,0x3,0x3,0x2 lên P4..P7) */
    write4bits(0x30); HAL_Delay(5);
    write4bits(0x30); HAL_Delay(5);
    write4bits(0x30); HAL_Delay(5);
    write4bits(0x20); HAL_Delay(1);

    command((uint8_t)(LCD_FUNCTIONSET | _displayfunction));  // 4-bit, 2 line, 5x8
    _displaycontrol = (uint8_t)(LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
    command((uint8_t)(LCD_DISPLAYCONTROL | _displaycontrol));

    HD44780_Clear();

    _displaymode = (uint8_t)(LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
    command((uint8_t)(LCD_ENTRYMODESET | _displaymode));

    HD44780_Backlight();
}

void HD44780_Clear(void)
{
    command(LCD_CLEARDISPLAY);
    HAL_Delay(2);
}

void HD44780_Home(void)
{
    command(LCD_RETURNHOME);
    HAL_Delay(2);
}

void HD44780_NoDisplay(void)
{
    _displaycontrol &= (uint8_t)~LCD_DISPLAYON;
    command((uint8_t)(LCD_DISPLAYCONTROL | _displaycontrol));
}

void HD44780_Display(void)
{
    _displaycontrol |= LCD_DISPLAYON;
    command((uint8_t)(LCD_DISPLAYCONTROL | _displaycontrol));
}

void HD44780_NoBlink(void)
{
    _displaycontrol &= (uint8_t)~LCD_BLINKON;
    command((uint8_t)(LCD_DISPLAYCONTROL | _displaycontrol));
}

void HD44780_Blink(void)
{
    _displaycontrol |= LCD_BLINKON;
    command((uint8_t)(LCD_DISPLAYCONTROL | _displaycontrol));
}

void HD44780_NoCursor(void)
{
    _displaycontrol &= (uint8_t)~LCD_CURSORON;
    command((uint8_t)(LCD_DISPLAYCONTROL | _displaycontrol));
}

void HD44780_Cursor(void)
{
    _displaycontrol |= LCD_CURSORON;
    command((uint8_t)(LCD_DISPLAYCONTROL | _displaycontrol));
}

void HD44780_ScrollDisplayLeft(void)
{
    command((uint8_t)(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT));
}

void HD44780_ScrollDisplayRight(void)
{
    command((uint8_t)(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT));
}

void HD44780_PrintLeft(void)
{
    _displaymode |= LCD_ENTRYLEFT;
    command((uint8_t)(LCD_ENTRYMODESET | _displaymode));
}

void HD44780_PrintRight(void)
{
    _displaymode &= (uint8_t)~LCD_ENTRYLEFT;
    command((uint8_t)(LCD_ENTRYMODESET | _displaymode));
}

void HD44780_LeftToRight(void)
{
    _displaymode |= LCD_ENTRYLEFT;
    command((uint8_t)(LCD_ENTRYMODESET | _displaymode));
}

void HD44780_RightToLeft(void)
{
    _displaymode &= (uint8_t)~LCD_ENTRYLEFT;
    command((uint8_t)(LCD_ENTRYMODESET | _displaymode));
}

void HD44780_ShiftIncrement(void)
{
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    command((uint8_t)(LCD_ENTRYMODESET | _displaymode));
}

void HD44780_ShiftDecrement(void)
{
    _displaymode &= (uint8_t)~LCD_ENTRYSHIFTINCREMENT;
    command((uint8_t)(LCD_ENTRYMODESET | _displaymode));
}

void HD44780_NoBacklight(void)
{
    _backlight = LCD_NOBACKLIGHT;
    expanderWrite(0);
}

void HD44780_Backlight(void)
{
    _backlight = LCD_BACKLIGHT;
    expanderWrite(0);
}

void HD44780_AutoScroll(void)
{
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    command((uint8_t)(LCD_ENTRYMODESET | _displaymode));
}

void HD44780_NoAutoScroll(void)
{
    _displaymode &= (uint8_t)~LCD_ENTRYSHIFTINCREMENT;
    command((uint8_t)(LCD_ENTRYMODESET | _displaymode));
}

void HD44780_CreateSpecialChar(uint8_t location, uint8_t charmap[])
{
    location &= 0x7;
    command((uint8_t)(LCD_SETCGRAMADDR | (location << 3)));
    for (int i = 0; i < 8; i++) writeData(charmap[i]);
}

void HD44780_LoadCustomCharacter(uint8_t char_num, uint8_t *rows)
{
    HD44780_CreateSpecialChar(char_num, rows);
}

void HD44780_PrintSpecialChar(uint8_t index)
{
    writeData((uint8_t)(index & 0x07));
}

void HD44780_SetCursor(uint8_t col, uint8_t row)
{
    static const uint8_t row_offsets[4] = {0x00, 0x40, 0x14, 0x54};
    if (row >= _numlines) row = (uint8_t)(_numlines - 1u);
    command((uint8_t)(LCD_SETDDRAMADDR | (col + row_offsets[row])));
}

void HD44780_SetBacklight(uint8_t new_val)
{
    _backlight = new_val ? LCD_BACKLIGHT : LCD_NOBACKLIGHT;
    expanderWrite(0);
}

void HD44780_PrintStr(const char s[])
{
    if (!s) return;
    while (*s) writeData((uint8_t)*s++);
}
