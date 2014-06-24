#include "ks0066.h"
#include <util/delay.h>

static uint8_t strBuf[] = "                ";			/* String buffer */

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

int main(void)
{
	ks0066Init();
	adcInit();

	uint8_t max = 48;
	uint8_t i;

	while(1) {

		for (i = 0; i <= max; i++) {
			adcMux(6);
			ks0066SetXY(0, 0);
			ks0066WriteString(mkNumString(adcVolt(), 4, 1));
			ks0066WriteData('V');
			adcMux(7);
			ks0066SetXY(7, 0);
			ks0066WriteString(mkNumString(adcVolt(), 4, 1));
			ks0066WriteData('V');

			ks0066SetXY(13, 0);
			ks0066WriteString(mkNumString(i, 3, 0));
			ks0066ShowBar(i, max);
			_delay_ms(100);
		}
	}

	return 0;
}
