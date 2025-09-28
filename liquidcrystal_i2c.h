#ifndef LIQUIDCRYSTAL_I2C_H_
#define LIQUIDCRYSTAL_I2C_H_

#include "main.h"    // ho?c "stm32f1xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HD44780 Commands */
#define LCD_CLEARDISPLAY    0x01
#define LCD_RETURNHOME      0x02
#define LCD_ENTRYMODESET    0x04
#define LCD_DISPLAYCONTROL  0x08
#define LCD_CURSORSHIFT     0x10
#define LCD_FUNCTIONSET     0x20
#define LCD_SETCGRAMADDR    0x40
#define LCD_SETDDRAMADDR    0x80

/* Entry Mode */
#define LCD_ENTRYRIGHT            0x00
#define LCD_ENTRYLEFT             0x02
#define LCD_ENTRYSHIFTINCREMENT   0x01
#define LCD_ENTRYSHIFTDECREMENT   0x00

/* Display On/Off */
#define LCD_DISPLAYON   0x04
#define LCD_DISPLAYOFF  0x00
#define LCD_CURSORON    0x02
#define LCD_CURSOROFF   0x00
#define LCD_BLINKON     0x01
#define LCD_BLINKOFF    0x00

/* Cursor Shift */
#define LCD_DISPLAYMOVE  0x08
#define LCD_CURSORMOVE   0x00
#define LCD_MOVERIGHT    0x04
#define LCD_MOVELEFT     0x00

/* Function Set */
#define LCD_8BITMODE   0x10
#define LCD_4BITMODE   0x00
#define LCD_2LINE      0x08
#define LCD_1LINE      0x00
#define LCD_5x10DOTS   0x04
#define LCD_5x8DOTS    0x00

/* ---- PCF8574 mapping (HIGH-NIBBLE ph? bi?n) ----
 * Bit7..0:  D7 D6 D5 D4  BL EN RW RS
 *           ^  ^  ^  ^   ^  ^  ^  ^
 *           P7 P6 P5 P4  P3 P2 P1 P0
 */
#define LCD_BACKLIGHT   0x08u   /* P3 */
#define LCD_NOBACKLIGHT 0x00u
#define ENABLE          0x04u   /* P2 */
#define RS              0x01u   /* P0 */
/* RW luôn 0 (ghi) */

/* Ð?a ch? I2C m?c d?nh cho PCF8574T = 0x27 (7-bit).
 * HAL dùng 8-bit => (addr7 << 1).
 */
#ifndef DEVICE_ADDR
# define DEVICE_ADDR (0x27u << 1)
#endif

/* API */
void HD44780_Init(uint8_t rows);
void HD44780_Clear(void);
void HD44780_Home(void);
void HD44780_NoDisplay(void);
void HD44780_Display(void);
void HD44780_NoBlink(void);
void HD44780_Blink(void);
void HD44780_NoCursor(void);
void HD44780_Cursor(void);
void HD44780_ScrollDisplayLeft(void);
void HD44780_ScrollDisplayRight(void);
void HD44780_PrintLeft(void);
void HD44780_PrintRight(void);
void HD44780_LeftToRight(void);
void HD44780_RightToLeft(void);
void HD44780_ShiftIncrement(void);
void HD44780_ShiftDecrement(void);
void HD44780_NoBacklight(void);
void HD44780_Backlight(void);
void HD44780_AutoScroll(void);
void HD44780_NoAutoScroll(void);
void HD44780_CreateSpecialChar(uint8_t location, uint8_t charmap[]);
void HD44780_PrintSpecialChar(uint8_t index);
void HD44780_SetCursor(uint8_t col, uint8_t row);
void HD44780_SetBacklight(uint8_t new_val);
void HD44780_LoadCustomCharacter(uint8_t char_num, uint8_t *rows);
void HD44780_PrintStr(const char s[]);

#ifdef __cplusplus
}
#endif
#endif /* LIQUIDCRYSTAL_I2C_H_ */
