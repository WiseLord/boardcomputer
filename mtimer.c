#include "mtimer.h"

#include <avr/interrupt.h>

static volatile uint16_t tempTimer = 0;

void mTimerInit(void)
{
	TIMSK |= (1<<OCIE2);						/* Enable Timer2 compare match interrupt */
	TCCR2 |= (1<<CS22) | (0<<CS21) | (0<<CS20);	/* Set timer prescaller to 64 (250kHz) */

	TCCR2 |= (1<<WGM21) | (0<<WGM20);			/* Clear Timer2 on match */

	OCR2 = 250;									/* Set Timer2 period to 1ms */
	TCNT2 = 0;									/* Reset Timer2 value */

	return;
}

#include "ks0066.h"

ISR (TIMER2_COMP_vect)
{
	if (tempTimer > 0)
		tempTimer--;
}

uint16_t getTempTimer()
{
	return tempTimer;
}

void setTempTimer(uint16_t val)
{
	tempTimer = val;
}
