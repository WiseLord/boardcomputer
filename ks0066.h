#ifndef KS0066_H
#define KS0066_H

#include <avr/io.h>

/* Data port */
#define KS0066_DATA_PORT			PORTB
#define KS0066_DATA_DDR				DDRB
#define KS0066_DATA_PIN				PINB

/* Control port */
#define KS0066_CTRL_PORT			PORTD
#define KS0066_CTRL_DDR				DDRD

#define KS0066_E					(1<<PD7)
#define KS0066_RW					(1<<PD6)
#define KS0066_RS					(1<<PD5)

/* Backlight port */
#define KS0066_BCKL_PORT			PORTD
#define KS0066_BCKL_DDR				DDRD

#define KS0066_BCKL					(1<<PD4)

#define KS0066_INIT_DATA			0x30

#define KS0066_CLEAR				0x01
#define KS0066_HOME					0x02

#define KS0066_SET_MODE				0x04
	#define KS0066_INC_ADDR			0x02
	#define KS0066_DEC_ADDR			0x00
	#define KS0066_SHIFT_WINDOW		0x01

#define KS0066_DISPLAY				0x08
	#define KS0066_DISPAY_ON		0x04
	#define KS0066_CURSOR_ON		0x02
	#define KS0066_CUR_BLINK_ON		0x01

#define KS0066_SHIFT				0x10
	#define KS0066_SHIFT_DISPLAY	0x08
	#define KS0066_SHIFT_CURSOR		0x00
	#define KS0066_SHIFT_RIGHT		0x04
	#define KS0066_SHIFT_LEFT		0x00

#define KS0066_FUNCTION				0x20
	#define KS0066_8BIT				0x10
	#define KS0066_4BIT				0x00
	#define KS0066_2LINES			0x08
	#define KS0066_1LINE			0x00
	#define KS0066_5X10				0x04
	#define KS0066_5X8				0x00

#define KS0066_SET_CGRAM			0x40
#define KS0066_SET_DDRAM			0x80
	#define KS0066_LINE_WIDTH		0x40

#define KS0066_STA_BUSY				0x80

/* User generated characters */
enum {
	LCD_USER_SYMBOLS_EMPTY,
	LCD_USER_SYMBOLS_BAR,
	LCD_USER_SYMBOLS_BIGNUM,
	LCD_USER_SYMBOLS_TEMP
};

void ks0066WriteCommand(uint8_t command);
void ks0066WriteData(uint8_t data);

void ks0066Clear(void);
void ks0066Init(void);

void ks0066SetXY(uint8_t x, uint8_t y);
void ks0066WriteString(uint8_t *string);

void ks0066ShowBar(uint16_t value, uint16_t max);

void ks0066ShowBigNum(uint16_t val, uint8_t pos);
void ks0066ShowBigDot(uint8_t x, uint8_t y);
void ks0066ShowBigColon(uint8_t x);

void ks0066GenTempSymbols(void);

#endif /* KS0066_H */
