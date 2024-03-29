#include "adcvolt.h"

void adcInit(void)
{
	ADCSRA = (1<<ADEN);									/* Enable ADC */
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);		/* Set ADC prescaler to 128 (125 kHz) */

	ADMUX |= (1<<REFS1) | (1<<REFS0);					/* Use internal reference voltage */

	return;
}

uint16_t adcGetVoltage(uint8_t mux)
{
	ADMUX &= ~0x07;
	ADMUX |= mux;

	ADCSRA |= (1<<ADSC);								/* Start ADC conversion */
	while(ADCSRA & (1<<ADSC));							/* Wait for measure finich */

	return ADCL | (ADCH << 8);
}
