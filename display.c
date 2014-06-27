#include "display.h"

#include "ks0066.h"
#include "ds18x20.h"
#include "adcvolt.h"

#include <avr/pgmspace.h>

static uint8_t userSybmols = LCD_USER_SYMBOLS_EMPTY;

static const uint8_t bigNumSegm[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, /* Top bar */
	0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, /* Bottom bar */
	0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x1F, 0x1F, /* Top and bottom bars */

	0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, /* Left part of dot */
	0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, /* Right part of dot */
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

static const uint8_t tempSymbols[] PROGMEM = {
	0x11, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x1E, 0x00, /* У */
	0x00, 0x00, 0x07, 0x09, 0x09, 0x09, 0x11, 0x00, /* л */
	0x00, 0x00, 0x11, 0x13, 0x15, 0x19, 0x11, 0x00, /* и */
	0x00, 0x00, 0x12, 0x12, 0x12, 0x12, 0x1F, 0x01, /* ц */
	0x00, 0x00, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x00, /* н */
	0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00, /* ° */
};

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

	value = (uint32_t)value * 48 / max;
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

void ks0066ShowBigDot(uint8_t x, uint8_t y)
{
	if (userSybmols != LCD_USER_SYMBOLS_BIGNUM)
		ks0066GenBigNum();

	ks0066SetXY(x, y);
	ks0066WriteData(0x03);
	ks0066WriteData(0x04);

	return;
}

void ks0066ShowBigColon(uint8_t x)
{
	ks0066ShowBigDot(x, 0);
	ks0066ShowBigDot(x, 1);

	return;
}

void ks0066GenTempSymbols(void)
{
	if (userSybmols != LCD_USER_SYMBOLS_TEMP) {

		ks0066WriteCommand(KS0066_SET_CGRAM);

		uint8_t i;

		for (i = 0; i < sizeof(tempSymbols); i++)
			ks0066WriteData(pgm_read_byte(&tempSymbols[i]));

		userSybmols = LCD_USER_SYMBOLS_TEMP;
	}

	return;
}

void showTemp(uint8_t count)
{
	ks0066GenTempSymbols();

	ks0066SetXY(0, 0);
	ks0066WriteString((uint8_t*)" \x08\x01\x02\x03""a    Ca\x01o\x04 ");

	ks0066SetXY(0, 1);
	ks0066WriteString(mkNumString(ds18x20GetTemp(0), 5, 1));
	ks0066WriteString((uint8_t*)"\x05""C  ");

	ks0066SetXY(9, 1);
	ks0066WriteString(mkNumString(ds18x20GetTemp(1), 5, 1));
	ks0066WriteString((uint8_t*)"\x05""C  ");

	return;
}

void showVoltage(uint8_t adcMux, uint8_t xPos, uint8_t yPos)
{
	adcSetMux(adcMux);

	ks0066SetXY(xPos, yPos);
	ks0066WriteString(mkNumString(adcGetVoltage(), 4, 1));
	ks0066WriteData('V');

}
