#include "taho.h"

#include <avr/interrupt.h>

static volatile uint16_t cnt;
static volatile uint16_t cntBuf;

static uint8_t ppt;									/* Pulses per turn from tahometer */
static uint8_t autooff;								/* Display autooff flag */

void setPpt(uint8_t value)
{
	uint16_t ocr1;

	ppt = value;
	if (ppt < 6)
		ppt = 6;
	if (ppt > 96)
		ppt = 96;

	ocr1 = 12500 / ppt * 6;
	OCR1AH = ocr1 >> 8;
	OCR1AL = ocr1 & 0xFF;

	return;
}

void setAutooff(uint8_t value)
{
	autooff = value;
}

uint8_t getPpt(void)
{
	return ppt;
}

uint8_t getAutoff(void)
{
	return autooff;
}

void tahoInit(uint8_t ppt, uint8_t autooff)
{
	/* Set INT1 pin as input with pull-up */
	TAHO_DDR &= ~TAHO;
	TAHO_PORT |= TAHO;

	MCUCR |= (1<<ISC11) | (1<<ISC10);				/* Interrupt INT1 on rising edge */
	GICR |= (1<<INT1);								/* Enable INT1 interrupt */

	/* Init Timer 1*/
	TCCR1A = 0;
	TCCR1B = (1<<CS12) | (0<<CS11) | (0<<CS10);		/* Divider = 256 => 62500 Hz*/
	TCCR1B |= (1<<WGM12);							/* Clear Timer1 on match */

	/* Select measurement time to have (PPM / 10) pulses */
	setPpt(ppt);

	/* Set display autooff flag*/
	setAutooff(autooff);

	/* Enable Timer1 A compare match interrupt */
	TIMSK |= (1<<OCIE1A);

	return;
}

ISR(INT1_vect)
{
	cnt++;

	return;
}

ISR (TIMER1_COMPA_vect)
{
	cntBuf = cnt;
	cnt = 0;

	return;
}

uint16_t getTaho()
{
	return cntBuf * 50;
}
