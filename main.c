#include "ks0066.h"
#include "ds18x20.h"

#include <util/delay.h>

#define DISPLAY_TIME_TEMP		1000

static uint8_t strBuf[] = "                ";			/* String buffer */
static ds18x20Dev devs[DS18X20_MAX_DEV];

void adcInit(void)
{
	ADCSRA = (1<<ADEN);									/* Enable ADC */
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);		/* Set ADC prescaler to 128 (62.5 kHz) */

	ADMUX = (1<<ADLAR);									/* Adjust result to left (8bit) */
	ADMUX |= (1<<REFS1) | (1<<REFS0);					/* Use internal reference voltage */

	return;
}

void adcMux(uint8_t mux)
{
	ADMUX &= ~0x07;
	ADMUX |= mux;

	return;
}

uint8_t adcVolt(void)
{
	ADCSRA |= (1<<ADSC);								/* Start ADC conversion */
	while(ADCSRA & (1<<ADSC));							/* Wait for measure finich */

	return ADCH;
}

uint8_t *mkNumString(int16_t number, uint8_t width, uint8_t frac)
{
	uint8_t sign = ' ';
	int8_t pos;

	if (number < 0) {
		sign = '-';
		number = -number;
	}

	/* Clear buffer and go to it's tail */
	for (pos = 0; pos < width; pos++)
		strBuf[pos] = ' ';
	strBuf[width] = '\0';
	pos = width - 1;

	/* Fill buffer from right to left */
	while (number > 0 || pos > width - frac - 2) {
		if (frac && (frac == width - pos - 1))
			strBuf[pos--] = '.';
		strBuf[pos] = number % 10 + 0x30;
		pos--;
		number /= 10;
	}

	if (pos >= 0)
		strBuf[pos] = sign;

	return strBuf;
}

void showTemp(uint8_t sensor, uint8_t time)
{
	uint8_t i;

	for (i = 0; i < time; i++) {
		if (ds18x20IsOnBus()) {
			ds18x20GetTemp(&devs[sensor]);
			ks0066SetXY(sensor % 2 * 8, 1);
			ks0066WriteString(mkNumString(devs[sensor].temp, 5, 1));
		}
	}

	return;
}

int main(void)
{
	ks0066Init();
	adcInit();

	uint8_t max = 48;
	uint8_t i, j = 0;

	adcMux(6);

	uint8_t count = 0;

	while(1) {

		ks0066Clear();
		ks0066ShowColon(7);
		for (i = 0; i < 10; i++) {
			ks0066ShowBigNum(i, j * 9 / 2);
			_delay_ms(1000);
			j = (j + 1) % 4;
		}

		if (ds18x20IsOnBus()) {
			count = ds18x20SearchAllRoms(devs, DS18X20_MAX_DEV);
			if (ds18x20IsOnBus()) {
				convertTemp(); /* It should be delay > 750ms after this */
				_delay_ms(DISPLAY_TIME_TEMP);
			}
			ks0066Clear();
			ks0066SetXY(0, 0);
			ks0066WriteString(mkNumString(count, 1, 0));
			ks0066WriteString((uint8_t*)" sensors found.");
			for (i = 0; i < count; i++) {
				showTemp(i, 2);
			}
			_delay_ms(3000);
		} else {
			ks0066Clear();
			for (i = 0; i <= max; i++) {
				ks0066SetXY(0, 0);
				ks0066WriteString(mkNumString(adcVolt(), 4, 1));
				ks0066WriteData('V');

				ks0066SetXY(13, 0);
				ks0066WriteString(mkNumString(i, 3, 0));
				ks0066ShowBar(i, max);
				_delay_ms(100);
			}
		}
	}

	return 0;
}
