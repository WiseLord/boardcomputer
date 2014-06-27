#include "display.h"

#include "ks0066.h"
#include "ds18x20.h"
#include "adcvolt.h"

#include <avr/pgmspace.h>

static uint8_t userSybmols = LCD_USER_SYMBOLS_EMPTY;

static const uint8_t bigCharSegm[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, /* Top bar */
	0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, /* Bottom bar */
	0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x1F, 0x1F, /* Top and bottom bars */

	0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, /* Left part of dot */
	0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, /* Right part of dot */
};

static const uint8_t bigChar[] PROGMEM = {
	0xFF, 0x01, 0xFF, 0xFF, 0x00, 0xFF, /* 0 */
	0x01, 0xff, 0x20, 0x00, 0xFF, 0x00, /* 1 */
	0x02, 0x02, 0xFF, 0xFF, 0x00, 0x00, /* 2 */
	0x02, 0x02, 0xFF, 0x00, 0x00, 0xFF, /* 3 */
	0xFF, 0x00, 0xFF, 0x20, 0x20, 0xFF, /* 4 */
	0xFF, 0x02, 0x02, 0x00, 0x00, 0xFF, /* 5 */
	0xFF, 0x02, 0x02, 0xFF, 0x00, 0xFF, /* 6 */
	0xFF, 0x01, 0xFF, 0x20, 0x20, 0xFF, /* 7 */
	0xFF, 0x02, 0xFF, 0xFF, 0x00, 0xFF, /* 8 */
	0xFF, 0x02, 0xFF, 0x00, 0x00, 0xFF, /* 9 */
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, /*   */
	0x20, 0x20, 0x20, 0x20, 0x01, 0x01, /* - */
	0x20, 0x20, 0x20, 0x03, 0x04, 0x20, /* . */
	0x03, 0x04, 0x20, 0x03, 0x04, 0x20, /* : */
};

static const uint8_t tempSymbols[] PROGMEM = {
	0x11, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x1E, 0x00, /* У */
	0x00, 0x00, 0x07, 0x09, 0x09, 0x09, 0x11, 0x00, /* л */
	0x00, 0x00, 0x11, 0x13, 0x15, 0x19, 0x11, 0x00, /* и */
	0x00, 0x00, 0x12, 0x12, 0x12, 0x12, 0x1F, 0x01, /* ц */
	0x00, 0x00, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x00, /* н */
	0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00, /* ° */
};

static const uint8_t voltSymbols[] PROGMEM = {
	0x1F, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E, 0x00, /* Б */
	0x00, 0x00, 0x1F, 0x04, 0x04, 0x04, 0x04, 0x00, /* т */
	0x00, 0x00, 0x07, 0x09, 0x07, 0x05, 0x09, 0x00, /* я */
	0x00, 0x00, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x00, /* н */
	0x00, 0x00, 0x07, 0x09, 0x09, 0x09, 0x11, 0x00, /* л */
	0x00, 0x00, 0x10, 0x10, 0x1C, 0x12, 0x1C, 0x00, /* ь */
};

static const uint8_t barSymbols[] PROGMEM = {
	0x00, 0x10, 0x14, 0x15,
};

static void ks0066GenBar(void)
{
	uint8_t i;
	uint8_t sym;

	if (userSybmols != LCD_USER_SYMBOLS_BAR) {
		ks0066WriteCommand(KS0066_SET_CGRAM);

		for (i = 0; i < 8 * sizeof(barSymbols); i++) {
			sym = i / 8;
			if (i % 8 == 3)
				sym = 3;
			if (i % 8 == 7)
				sym = 0;
			ks0066WriteData(pgm_read_byte(&barSymbols[sym]));
		}

		userSybmols = LCD_USER_SYMBOLS_BAR;
	}

	return;
}

void ks0066ShowBar(uint16_t value, uint16_t max)
{
	uint8_t i;

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

static void ks0066GenBigSymbols(void)
{
	uint8_t i;

	if (userSybmols != LCD_USER_SYMBOLS_BIGNUM) {

		ks0066WriteCommand(KS0066_SET_CGRAM);

		for (i = 0; i < sizeof(bigCharSegm); i++)
			ks0066WriteData(pgm_read_byte(&bigCharSegm[i]));

		userSybmols = LCD_USER_SYMBOLS_BIGNUM;
	}

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

void ks0066GenVoltSymbols(void)
{
	if (userSybmols != LCD_USER_SYMBOLS_VOLT) {

		ks0066WriteCommand(KS0066_SET_CGRAM);

		uint8_t i;

		for (i = 0; i < sizeof(voltSymbols); i++)
			ks0066WriteData(pgm_read_byte(&voltSymbols[i]));

		userSybmols = LCD_USER_SYMBOLS_VOLT;
	}

	return;
}

void ks0066ShowBigNum(uint16_t val, uint8_t pos)
{
	uint8_t i;

	ks0066GenBigSymbols();

	ks0066SetXY(pos, 0);
	for (i = 0; i < 6; i++) {
		if (i == 3)
			ks0066SetXY(pos, 1);
		ks0066WriteData(pgm_read_byte(&bigChar[val * 6 + i]));
	}

	return;
}

void ks0066ShowBigString(uint8_t *string, uint8_t pos)
{
	uint8_t ch;

	while(*string) {
		ch = *string++;
		if (ch >= '0' && ch <= '9') {
			ks0066ShowBigNum(ch - '0', pos);
			pos += 4;
		} else if (ch == ' ') {
			ks0066ShowBigNum(10, pos);
			pos += 4;
		} else if (ch == '-') {
			ks0066ShowBigNum(11, pos);
			pos += 4;
		} else if (ch == '.') {
			pos--;
			ks0066ShowBigNum(12, pos);
			pos += 2;
		} else if (ch == ':') {
			pos--;
			ks0066ShowBigNum(13, pos);
			pos += 2;
		}
	}

	return;
}

void showRPM(uint16_t rpm)
{
	uint16_t maxRPM = 4800;

	ks0066SetXY(12, 0);
	ks0066WriteString(mkNumString(rpm, 4, 0));
	ks0066ShowBar(rpm, maxRPM);

	return;
}

void showBigRPM(uint16_t rpm)
{
	ks0066SetXY(0, 0);

	ks0066ShowBigString(mkNumString(rpm, 4, 0), 1);

	return;
}

void showTemp(uint8_t count)
{
	ks0066GenTempSymbols();

	ks0066SetXY(0, 0);
	ks0066WriteString((uint8_t*)" \x08\x01\x02\x03""a    Ca\x01o\x04 ");

	ks0066SetXY(0, 1);
	ks0066WriteString(mkNumString(ds18x20GetTemp(0), 4, 1));
	ks0066WriteString((uint8_t*)"\x05""C  ");

	ks0066SetXY(9, 1);
	ks0066WriteString(mkNumString(ds18x20GetTemp(1), 4, 1));
	ks0066WriteString((uint8_t*)"\x05""C  ");

	return;
}

void showBigTemp(uint8_t sensor)
{
	int16_t temp;

	temp = ds18x20GetTemp(sensor);

	ks0066SetXY(0, 0);
	ks0066ShowBigString(mkNumString(temp, 4, 1), 0);

	return;
}

static void showVoltage(uint8_t adcMux)
{
	adcSetMux(adcMux);

	uint8_t i;
	uint16_t avgVoltage = 0;

	for (i = 0; i < 8; i++) {
		avgVoltage += adcGetVoltage();
	}

	ks0066WriteString(mkNumString(avgVoltage >> 5, 3, 1));
	ks0066WriteString((uint8_t*)" B");

	return;
}

void showVoltageAll()
{
	ks0066GenVoltSymbols();

	ks0066SetXY(0, 0);
	ks0066WriteString((uint8_t*)"\x08""a""\x01""ape""\x02""   ");
	showVoltage(ADCMUX_BATTERY);
	ks0066SetXY(0, 1);
	ks0066WriteString((uint8_t*)"Ko""\x03\x01""po""\x04\x05""  ");
	showVoltage(ADCMUX_VOLTS);

	return;
}
