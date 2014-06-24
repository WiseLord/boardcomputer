#include "ks0066.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#define swap(x) (__builtin_avr_swap(x))		/*  Swaps nibbles in byte */

static uint8_t userSybmols = LCD_USER_SYMBOLS_EMPTY;

static void ks0066writeStrob()
{
	asm("nop");	/* 40ns */
	KS0066_CTRL_PORT |= KS0066_E;
	asm("nop");	/* 230ns */
	asm("nop");
	asm("nop");
	asm("nop");
	KS0066_CTRL_PORT &= ~KS0066_E;

	return;
}

static uint8_t ks0066readStrob()
{
	uint8_t pin;

	asm("nop");	/* 40ns */
	KS0066_CTRL_PORT |= KS0066_E;
	asm("nop");	/* 230ns */
	asm("nop");
	asm("nop");
	asm("nop");
	pin = swap(KS0066_DATA_PIN & 0x0F);
	KS0066_CTRL_PORT &= ~KS0066_E;
	asm("nop");	/* 230ns */
	asm("nop");
	asm("nop");
	asm("nop");
	KS0066_CTRL_PORT |= KS0066_E;
	asm("nop");	/* 230ns */
	asm("nop");
	asm("nop");
	asm("nop");
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

	KS0066_DATA_PORT |= swap(KS0066_INIT_DATA);
	KS0066_CTRL_PORT &= ~(KS0066_RS | KS0066_RW);
	_delay_ms(20);
	ks0066writeStrob();
	_delay_ms(5);
	ks0066writeStrob();
	_delay_us(120);
	ks0066writeStrob();

	ks0066WriteCommand(swap(KS0066_FUNCTION | KS0066_4BIT));
	ks0066WriteCommand(KS0066_FUNCTION | KS0066_4BIT | KS0066_2LINES);

	ks0066WriteCommand(KS0066_DISPLAY | KS0066_DISPAY_ON);
	ks0066WriteCommand(KS0066_CLEAR);
	_delay_ms(2);
	ks0066WriteCommand(KS0066_SET_MODE | KS0066_INC_ADDR);

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
