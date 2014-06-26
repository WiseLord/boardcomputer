#include "ks0066.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

static uint8_t userSybmols = LCD_USER_SYMBOLS_EMPTY;

static const uint8_t bigNumSegm[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, /* Top bar */
	0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, /* Bottom bar */
	0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x1F, 0x1F, /* Top and bottom bars */
	0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, /* Left part of colon */
	0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, /* Right part of colon */
};

static const uint8_t bigNum[] PROGMEM = {
	0xFF, 0x01, 0xFF, 0xFF, 0x00, 0xFF, /* 0 */
	0x01, 0xff, 0x20, 0x00, 0xFF, 0x00, /* 1 */
	0x02, 0x02, 0xFF, 0xFF, 0x00, 0x00, /* 2 */
	0x02, 0x02, 0xFF, 0x00, 0x00, 0xFF, /* 3 */
	0xFF, 0x00, 0xFF, 0x20, 0x20, 0xFF, /* 4 */
	0xFF, 0x02, 0x02, 0x00, 0x00, 0xFF, /* 5 */
	0xFF, 0x02, 0x02, 0xFF, 0x00, 0xFF, /* 6 */
	0x01, 0x01, 0xFF, 0x20, 0x20, 0xFF, /* 7 */
	0xFF, 0x02, 0xFF, 0xFF, 0x00, 0xFF, /* 8 */
	0xFF, 0x02, 0xFF, 0x00, 0x00, 0xFF, /* 9 */
};

static const uint8_t colon[] PROGMEM = {
	0x03, 0x04, 0x03, 0x04,
};

static void ks0066writeStrob()
{
	_delay_us(0.05);
	KS0066_CTRL_PORT |= KS0066_E;
	_delay_us(0.25);
	KS0066_CTRL_PORT &= ~KS0066_E;

	return;
}

static uint8_t ks0066readStrob()
{
	uint8_t pin;

	_delay_us(0.05);
	KS0066_CTRL_PORT |= KS0066_E;
	_delay_us(0.25);
	pin = swap(KS0066_DATA_PIN & 0x0F);
	KS0066_CTRL_PORT &= ~KS0066_E;
	_delay_us(0.25);
	KS0066_CTRL_PORT |= KS0066_E;
	_delay_us(0.25);
	pin |= (KS0066_DATA_PIN & 0x0F);
	KS0066_CTRL_PORT &= ~KS0066_E;

	return pin;
}

static void ks0066waitWhileBusy()
{
	uint8_t i = 0;

	KS0066_CTRL_PORT &= ~KS0066_RS;
	KS0066_CTRL_PORT |= KS0066_RW;

	KS0066_DATA_DDR &= 0xF0;

	while (ks0066readStrob() & KS0066_STA_BUSY) {
		if (i++ > 200)	/* Avoid endless loop */
			return;
	}

	return;
}

void ks0066WriteCommand(uint8_t command)
{
	ks0066waitWhileBusy();

	KS0066_CTRL_PORT &= ~(KS0066_RS | KS0066_RW);

	KS0066_DATA_DDR |= 0x0F;
	KS0066_DATA_PORT &= 0xF0;
	KS0066_DATA_PORT |= (swap(command) & 0x0F);
	ks0066writeStrob();
	KS0066_DATA_PORT &= 0xF0;
	KS0066_DATA_PORT |= (command & 0x0F);

	ks0066writeStrob();

	return;
}

void ks0066WriteData(uint8_t data)
{
	ks0066waitWhileBusy();

	KS0066_CTRL_PORT &= ~KS0066_RW;
	KS0066_CTRL_PORT |= KS0066_RS;

	KS0066_DATA_DDR |= 0x0F;
	KS0066_DATA_PORT &= 0xF0;
	KS0066_DATA_PORT |= (swap(data) & 0x0F);
	ks0066writeStrob();
	KS0066_DATA_PORT &= 0xF0;
	KS0066_DATA_PORT |= (data & 0x0F);

	ks0066writeStrob();

	return;
}

void ks0066Clear(void)
{
	ks0066WriteCommand(KS0066_CLEAR);
	_delay_ms(2);

	return;
}

void ks0066Init(void)
{
	KS0066_DATA_DDR |= 0x0F;
	KS0066_CTRL_DDR |= KS0066_RS | KS0066_RW | KS0066_E;
	KS0066_BCKL_DDR |= KS0066_BCKL;

	KS0066_DATA_PORT |= swap(KS0066_INIT_DATA);
	KS0066_CTRL_PORT &= ~(KS0066_RS | KS0066_RW);
	KS0066_BCKL_PORT |= KS0066_BCKL;

	_delay_ms(20);
	ks0066writeStrob();
	_delay_ms(5);
	ks0066writeStrob();
	_delay_us(120);
	ks0066writeStrob();

	ks0066WriteCommand(swap(KS0066_FUNCTION | KS0066_4BIT));
	ks0066WriteCommand(KS0066_FUNCTION | KS0066_4BIT | KS0066_2LINES);
	ks0066WriteCommand(KS0066_SET_MODE | KS0066_INC_ADDR);

	ks0066WriteCommand(KS0066_CLEAR);
	_delay_ms(2);
	ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON);

	return;
}

void ks0066SetXY(uint8_t x, uint8_t y)
{
	ks0066WriteCommand(KS0066_SET_DDRAM | (KS0066_LINE_WIDTH * y + x));

	return;
}

void ks0066WriteString(uint8_t *string)
{
	while(*string)
		ks0066WriteData(*string++);

	return;
}

static void ks0066GenBar(void)
{
	ks0066WriteCommand(KS0066_SET_CGRAM);

	uint8_t j, i;
	uint8_t bar[4] = {0x00, 0x10, 0x14, 0x15};

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 7; j++) {
			if (j == 3) {
				ks0066WriteData(0x15);
			} else {
				ks0066WriteData(bar[i]);
			}
		}
		ks0066WriteData(0x00);
	}

	userSybmols = LCD_USER_SYMBOLS_BAR;

	return;
}

void ks0066ShowBar(uint16_t value, uint16_t max)
{
	uint8_t i;

	if (userSybmols != LCD_USER_SYMBOLS_BAR)
		ks0066GenBar();

	ks0066SetXY(0, 1);

	value = value * 48 / max;
	for (i = 0; i < 16; i++) {
		if (value / 3 > i) {
			ks0066WriteData(0x03);
		} else {
			if (value / 3 < i) {
				ks0066WriteData(0x00);
			} else {
				ks0066WriteData(value % 3);
			}
		}
	}
}

static void ks0066GenBigNum(void)
{
	ks0066WriteCommand(KS0066_SET_CGRAM);

	uint8_t i;

	for (i = 0; i < sizeof(bigNumSegm); i++)
		ks0066WriteData(pgm_read_byte(&bigNumSegm[i]));

	userSybmols = LCD_USER_SYMBOLS_BIGNUM;

	return;
}

void ks0066ShowBigNum(uint16_t val, uint8_t pos)
{
	uint8_t i;

	if (userSybmols != LCD_USER_SYMBOLS_BIGNUM)
		ks0066GenBigNum();

	ks0066SetXY(pos, 0);
	for (i = 0; i < 6; i++) {
		if (i == 3)
			ks0066SetXY(pos, 1);
		ks0066WriteData(pgm_read_byte(&bigNum[val * 6 + i]));
	}
	return;
}

void ks0066ShowColon(uint8_t pos)
{
	uint8_t i;

	if (userSybmols != LCD_USER_SYMBOLS_BIGNUM)
		ks0066GenBigNum();

	ks0066SetXY(pos, 0);
	for (i = 0; i < 4; i++) {
		if (i == 2)
			ks0066SetXY(pos, 1);
		ks0066WriteData(pgm_read_byte(&colon[i]));
	}
	return;
}
