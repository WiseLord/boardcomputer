#ifndef DISPLAY_H
#define DISPLAY_H

#include <avr/io.h>

/* Display modes */
enum {
	MODE_STANDBY,
	MODE_TEMP,
	MODE_BIG_TEMP_OUT,
	MODE_BIG_TEMP_CAR,
	MODE_VOLTAGE,
	MODE_BIG_VOLT_BATTERY,
	MODE_BIG_VOLT_BOARD,
	MODE_CLOCK,
	MODE_CLOCK_EDIT_H,
	MODE_CLOCK_EDIT_M,
	MODE_RPM,
	MODE_BIG_RPM
};

/* User generated characters */
enum {
	LCD_USER_SYMBOLS_EMPTY,
	LCD_USER_SYMBOLS_BAR,
	LCD_USER_SYMBOLS_BIGNUM,
	LCD_USER_SYMBOLS_TEMPVOLT
};

enum {
	TEMP_SENSOR_OUT = 0,
	TEMP_SENSOR_CAR = 1
};

enum {
	VOLTAGE_BATTERY = 0,
	VOLTAGE_BOARD = 1
};

void ks0066ShowBar(uint16_t value, uint16_t max);

void ks0066ShowBigString(uint8_t *string, uint8_t pos);

void showRPM(uint16_t rpm);
void showBigRPM(uint16_t rpm);

void showTemp(uint8_t count);
void showBigTemp(uint8_t sensor);

void showVoltageAll(void);
void showBigVoltage(uint8_t sensor);

void showClock(uint8_t *clkString);

#endif /* DISPLAY_H */
