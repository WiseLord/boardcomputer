#include "ks0066.h"
#include "ds18x20.h"
#include "mtimer.h"

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

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
	static uint8_t strBuf[6];

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

void showTempAll(uint8_t count)
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

int main(void)
{
	ks0066Init();
	adcInit();
	mTimerInit();
	sei();

	uint8_t max = 48;
	uint8_t i;

	adcMux(6);

	uint8_t count = 0;

	while(1) {
		count = ds18x20Process();
		if (count) {
			showTempAll(count);
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
