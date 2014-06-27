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

static const uint8_t tempVoltSymbols[] PROGMEM = {
	0x11, 0x11, 0x11, 0x0F, 0x01, 0x11, 0x0E, 0x00, /* У */
	0x07, 0x09, 0x09, 0x09, 0x09, 0x09, 0x11, 0x00, /* Л */
	0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11, 0x00, /* И */
	0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x1F, 0x01, /* Ц */
	0x1F, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E, 0x00, /* Б */
	0x0F, 0x11, 0x11, 0x0F, 0x05, 0x09, 0x11, 0x00, /* Я */
	0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00, /* ° */
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

void ks0066GenSymbols(uint8_t symType, const uint8_t *symArray)
{
	uint8_t i;
	uint8_t w = 0;

	if (symArray == tempVoltSymbols) {
		w = sizeof(tempVoltSymbols);
	} else if (symArray == bigCharSegm) {
		w = sizeof(bigCharSegm);
	}

	if (userSybmols != symType) {

		ks0066WriteCommand(KS0066_SET_CGRAM);

		for (i = 0; i < w; i++)
			ks0066WriteData(pgm_read_byte(&symArray[i]));

		userSybmols = symType;
	}

	return;
}

void ks0066ShowBigNum(uint16_t val, uint8_t pos)
{
	uint8_t i;

	ks0066GenSymbols(LCD_USER_SYMBOLS_BIGNUM, bigCharSegm);

	ks0066SetXY(pos, 0);
	for (i = 0; i < 6; i++) {
		if (i == 3) {
			ks0066WriteData(0x20);
			ks0066SetXY(pos, 1);
		}
		ks0066WriteData(pgm_read_byte(&bigChar[val * 6 + i]));
	}
	ks0066WriteData(0x20);

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

void showTemp(uint8_t sensor, uint8_t pos)
{
	ks0066SetXY(pos, 1);
	ks0066WriteString(mkNumString(ds18x20GetTemp(sensor), 4, 1));
	ks0066WriteString((uint8_t*)"\x06""C  ");

	return;
}

void showTempAll(uint8_t count)
{
	ks0066GenSymbols(LCD_USER_SYMBOLS_TEMPVOLT, tempVoltSymbols);

	ks0066SetXY(0, 0);
	ks0066WriteString((uint8_t*)" \x08\x01\x02\x03""А    CА\x01""OH ");

	showTemp(0, 0);
	showTemp(1, 9);

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
	ks0066GenSymbols(LCD_USER_SYMBOLS_TEMPVOLT, tempVoltSymbols);

	ks0066SetXY(0, 0);
	ks0066WriteString((uint8_t*)"\x04""ATAPE""\x05""   ");
	showVoltage(ADCMUX_BATTERY);
	ks0066SetXY(0, 1);
	ks0066WriteString((uint8_t*)"\x04""OPTOBOE  ");
	showVoltage(ADCMUX_VOLTS);

	return;
}
