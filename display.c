#include "display.h"

#include "display/ks0066.h"
#include "ds18x20.h"
#include "adcvolt.h"
#include "taho.h"

#include <avr/pgmspace.h>

static uint8_t userSybmols = LCD_USER_SYMBOLS_EMPTY;

static const uint8_t bigCharSegm[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, /* Bottom bar */
	0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, /* Top bar */
	0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x1F, 0x1F, /* Top and bottom bars */

	0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, /* Left part of dot */
	0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, /* Right part of dot */

	0x01, 0x01, 0x03, 0x03, 0x07, 0x07, 0x0F, 0x0F, /* Left triangle */
	0x10, 0x10, 0x18, 0x18, 0x1C, 0x1C, 0x1E, 0x1E, /* Right triangle */

	0x1F, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E, 0x00, /* Б */
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
	0x20, 0x20, 0x80, 0x03, 0x04, 0x80, /* . */
	0x03, 0x04, 0x80, 0x03, 0x04, 0x80, /* : */
	0x20, 0x20, 0x80, 0x20, 0x20, 0x80, /* ` */
};

static const uint8_t tempVoltSymbols[] PROGMEM = {
	0x11, 0x11, 0x11, 0x0F, 0x01, 0x11, 0x0E, 0x00, /* У */
	0x07, 0x09, 0x09, 0x09, 0x09, 0x09, 0x11, 0x00, /* Л */
	0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11, 0x00, /* И */
	0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x1F, 0x01, /* Ц */
	0x1F, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E, 0x00, /* Б */
	0x0F, 0x11, 0x11, 0x0F, 0x05, 0x09, 0x11, 0x00, /* Я */
	0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00, /* ° */
	0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F, 0x00, /* Battery */
};

static const uint8_t barSymbols[] PROGMEM = {
	0x00, 0x10, 0x14, 0x15,
};

static const uint8_t rpmChars[] PROGMEM = {
	0x15, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11, 0x00, /* Й */
	0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00, /* П */
	0x1F, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E, 0x00, /* Б */
	0x07, 0x09, 0x09, 0x09, 0x09, 0x09, 0x11, 0x00, /* Л */
};

static const uint8_t icon5[] PROGMEM = {
	0x05, 0x06, 0x20, 0x05, 0x06, /* Houses */
	0xFF, 0xFF, 0x00, 0xFF, 0xFF,

	0x20, 0x05, 0xFF, 0xFF, 0x06, /* Car */
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

static char *mkNumString(int16_t value, uint8_t width, uint8_t prec)
{
	static char strBuf[6];

	uint8_t sign = ' ';
	int8_t pos;

	if (value < 0) {
		sign = '-';
		value = -value;
	}

	/* Clear buffer and go to it's tail */
	for (pos = 0; pos < width + prec; pos++)
		strBuf[pos] = ' ';
	strBuf[width + prec] = '\0';
	pos = width + prec - 1;

	/* Fill buffer from right to left */
	while (value > 0 || pos > width - 2) {
		if (prec && (width - pos - 1 == 0))
			strBuf[pos--] = '.';
		strBuf[pos] = value % 10 + 0x30;
		pos--;
		value /= 10;
	}

	if (pos >= 0)
		strBuf[pos] = sign;

	return strBuf;
}

static void ks0066GenBar(void)
{
	uint8_t i;
	uint8_t sym;

	if (userSybmols != LCD_USER_SYMBOLS_BAR) {
		ks0066WriteCommand(KS0066_SET_CGRAM);

		for (i = 0; i < 8 * 4; i++) {
			sym = i / 8;
			if (i % 8 == 3)
				sym = 3;
			if (i % 8 == 7)
				sym = 0;
			ks0066WriteData(pgm_read_byte(&barSymbols[sym]));
		}
		for (i = 0; i < sizeof(rpmChars); i++)
			ks0066WriteData(pgm_read_byte(&rpmChars[i]));

		userSybmols = LCD_USER_SYMBOLS_BAR;
	}

	return;
}

static void ks0066GenBigBar(void)
{
	uint8_t i;
	uint8_t sym;

	if (userSybmols != LCD_USER_SYMBOLS_BIG_BAR) {
		ks0066WriteCommand(KS0066_SET_CGRAM);

		for (i = 0; i < 8 * 4; i++) {
			sym = i / 8;
			if (i % 8 == 0)
				sym = 3;
			ks0066WriteData(pgm_read_byte(&barSymbols[sym]));
		}
		for (i = 0; i < 8 * 4; i++) {
			sym = i / 8;
			if (i % 8 == 7)
				sym = 3;
			ks0066WriteData(pgm_read_byte(&barSymbols[sym]));
		}

		userSybmols = LCD_USER_SYMBOLS_BIG_BAR;
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

void ks0066ShowBigBar(uint16_t value, uint16_t max)
{
	uint8_t i;

	ks0066GenBigBar();

	value = (uint32_t)value * 48 / max;

	ks0066SetXY(0, 0);
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

	ks0066SetXY(0, 1);
	for (i = 0; i < 16; i++) {
		if (value / 3 > i) {
			ks0066WriteData(0x07);
		} else {
			if (value / 3 < i) {
				ks0066WriteData(0x04);
			} else {
				ks0066WriteData(value % 3 + 4);
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

void ks0066ShowBigChar(uint16_t val, uint8_t pos)
{
	uint8_t i;
	uint8_t data;

	ks0066GenSymbols(LCD_USER_SYMBOLS_BIGNUM, bigCharSegm);

	ks0066SetXY(pos, 0);
	for (i = 0; i < 6; i++) {
		if (i == 3)
			ks0066SetXY(pos, 1);

		data = pgm_read_byte(&bigChar[val * 6 + i]);
		if (data != 0x80)
			ks0066WriteData(data);
	}

	return;
}

void ks0066ShowBigString(char *string, uint8_t pos)
{
	uint8_t ch;

	while(*string) {
		ch = *string++;
		if (ch >= '0' && ch <= '9') {
			ks0066ShowBigChar(ch - '0', pos);
			pos += 4;
		} else if (ch == ' ') {
			ks0066ShowBigChar(10, pos);
			pos += 4;
		} else if (ch == '-') {
			ks0066ShowBigChar(11, pos);
			pos += 4;
		} else if (ch == '.') {
			pos--;
			ks0066ShowBigChar(12, pos);
			pos += 2;
		} else if (ch == ':') {
			ks0066ShowBigChar(13, --pos);
			pos += 2;
		} else if (ch == '`') {
			ks0066ShowBigChar(14, --pos);
			pos += 2;
		}
	}

	return;
}

void showRPM(uint16_t rpm)
{
	ks0066SetXY(0, 0);

	ks0066WriteString("TAXOMETP    ");

	ks0066WriteString(mkNumString(rpm, 4, 0));
	ks0066ShowBar(rpm, RPM_MAX);

	return;
}

void showScaleRPM(uint16_t rpm)
{
	ks0066ShowBigBar(rpm, RPM_MAX);

	return;
}

void showBigRPM(uint16_t rpm)
{
	ks0066SetXY(0, 0);

	ks0066ShowBigString(mkNumString(rpm, 4, 0), 1);

	return;
}

void showEditRPM(uint16_t rpm)
{
	ks0066GenBar();

	ks0066SetXY(0, 0);
	ks0066WriteString("TAXOMETP   ");
	ks0066WriteString(mkNumString(rpm, 5, 0));

	ks0066SetXY(0, 1);
	ks0066WriteString("HACTPO\x04KA    ");
	ks0066WriteString(mkNumString(getPpt(), 3, 0));

	return;
}

void showEditAutooff(uint16_t rpm)
{
	ks0066GenBar();

	ks0066SetXY(0, 0);
	ks0066WriteString("TAXOMETP   ");
	ks0066WriteString(mkNumString(rpm, 5, 0));

	ks0066SetXY(0, 1);
	ks0066WriteString("ABTOOTK\x07.    ");
	ks0066WriteString(mkNumString(getAutoff(), 3, 0));

	return;
}

static void showSensorTemp(uint8_t sensor, uint8_t x, uint8_t y)
{
	ks0066SetXY(x, y);
	ks0066WriteString(mkNumString(ds18x20GetTemp(sensor), 4, 1));
	ks0066WriteString("\x06""C  ");

	return;
}

void showTemp(uint8_t count)
{
	ks0066GenSymbols(LCD_USER_SYMBOLS_TEMPVOLT, tempVoltSymbols);

	ks0066SetXY(0, 0);
	ks0066WriteString(" CA\x01""OH    \x08\x01\x02\x03""A ");

	showSensorTemp(0, 9, 1);
	showSensorTemp(1, 0, 1);

	return;
}

static void showTempIcon(uint8_t icon)
{
	uint8_t i;
	uint8_t data;

	ks0066GenSymbols(LCD_USER_SYMBOLS_BIGNUM, bigCharSegm);

	ks0066SetXY(0, 0);
	for (i = 0; i < 10; i++) {
		if (i == 5)
			ks0066SetXY(0, 1);

		data = pgm_read_byte(&icon5[icon * 10 + i]);
		if (data != 0x80)
			ks0066WriteData(data);
	}

	return;
}

void showBigTemp(uint8_t sensor)
{
	int16_t temp;

	temp = ds18x20GetTemp(sensor);

	ks0066ShowBigString(mkNumString(temp/10, 3, 0), 5);

	showTempIcon(sensor);

	return;
}

uint8_t getAvgVoltage(uint8_t sensor)
{
	uint8_t i;
	uint16_t avgVoltage = 0;

	for (i = 0; i < 8; i++)
		avgVoltage += adcGetVoltage(sensor);

	return avgVoltage >>= 5;
}

void showBigVoltage(uint8_t sensor)
{
	ks0066ShowBigString(mkNumString(getAvgVoltage(sensor), 3, 1), 4);

	ks0066SetXY(0, sensor);
	if (sensor == VOLTAGE_BATTERY)
		ks0066WriteString("\x07""AT");
	else
		ks0066WriteString("\x07""OP");

	return;
}

static void showVoltage(uint8_t sensor)
{
	ks0066WriteString(mkNumString(getAvgVoltage(sensor), 3, 1));
	ks0066WriteString(" B");

	return;
}

void showVoltageAll(void)
{
	ks0066GenSymbols(LCD_USER_SYMBOLS_TEMPVOLT, tempVoltSymbols);

	ks0066SetXY(0, 0);
	ks0066WriteString("\x04""ATAPE""\x05""   ");
	showVoltage(VOLTAGE_BATTERY);
	ks0066SetXY(0, 1);
	ks0066WriteString("\x04""OPTOBOE  ");
	showVoltage(VOLTAGE_BOARD);

	return;
}

void showBigClock(char *clkString)
{
	ks0066ShowBigString(clkString, 0);

	return;
}

void showClock(char *clkString)
{
	ks0066GenSymbols(LCD_USER_SYMBOLS_TEMPVOLT, tempVoltSymbols);

	ks0066SetXY(0, 0);
	ks0066WriteString(clkString);

	ks0066SetXY(0, 1);
	ks0066WriteString("\x07 ");
	showVoltage(VOLTAGE_BATTERY);

	showSensorTemp(0, 9, 1);
	showSensorTemp(1, 9, 0);

	return;
}
