#ifndef DISPLAY_H
#define DISPLAY_H

#include <avr/io.h>

#define ADCMUX_BATTERY		6
#define ADCMUX_VOLTS		7

/* User generated characters */
enum {
	LCD_USER_SYMBOLS_EMPTY,
	LCD_USER_SYMBOLS_BAR,
	LCD_USER_SYMBOLS_BIGNUM,
	LCD_USER_SYMBOLS_TEMP
};

void ks0066ShowBar(uint16_t value, uint16_t max);

void ks0066ShowBigString(uint8_t *string, uint8_t pos);

void showRPM(uint16_t rpm);
void showBigRPM(uint16_t rpm);

void showTemp(uint8_t count);
void showBigTemp(uint8_t sensor);

void showVoltage(uint8_t adcMux, uint8_t xPos, uint8_t yPos);

#endif /* DISPLAY_H */