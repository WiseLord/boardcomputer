#include "mtimer.h"

#include <avr/interrupt.h>

static volatile uint8_t cmdBuf;

static volatile uint16_t tempTimer;				/* Timer of temperature measuring process */
static volatile uint16_t dispTimer;				/* Timer of current display mode */
static volatile uint16_t clockTimer;			/* 1 second timer */

static volatile clock time = {12, 00, 00};

void mTimerInit(void)
{
	TIMSK |= (1<<OCIE2);						/* Enable Timer2 compare match interrupt */
	TCCR2 |= (1<<CS22) | (0<<CS21) | (0<<CS20);	/* Set timer prescaller to 64 (250kHz) */

	TCCR2 |= (1<<WGM21) | (0<<WGM20);			/* Clear Timer2 on match */

	OCR2 = 249;									/* Set Timer2 period to 1ms */
	TCNT2 = 0;									/* Reset Timer2 value */

	/* Setup buttons as inputs with pull-up resistors */
	BTN_DDR &= ~(BTN_MASK);
	BTN_PORT |= BTN_MASK;

	cmdBuf = CMD_EMPTY;
	tempTimer = 0;
	dispTimer = 0;

	return;
}

void clockIncHour()
{
	time.hour++;
	if (time.hour >= 24)
		time.hour = 0;
	return;
}

void clockDecHour()
{
	time.hour--;
	if (time.hour < 0)
		time.hour = 23;
	return;
}

void clockIncMin()
{
	time.sec = 0;
	time.min++;
	if (time.min >= 60)
		time.min = 0;
	return;
}

void clockDecMin()
{
	time.sec = 0;
	time.min--;
	if (time.min < 0)
		time.min = 59;
	return;
}

static void incClock()
{
	time.tsec++;
	if (time.tsec >= 10) {
		time.tsec = 0;
		time.sec++;
	}
	if (time.sec >= 60) {
		time.sec = 0;
		time.min++;
	}
	if (time.min >= 60) {
		time.min = 0;
		time.hour++;
	}
	if (time.hour >= 24)
		time.hour = 0;

	return;
}

uint8_t *getClock(uint8_t clkEdit)
{
	static uint8_t *clockString = (uint8_t*)"  :  ";

	clockString[0] = time.hour / 10 + '0';
	clockString[1] = time.hour % 10 + '0';
	clockString[3] = time.min / 10 + '0';
	clockString[4] = time.min % 10 + '0';

	if (time.tsec == 0 || time.tsec == 5) {
		if (clkEdit == CLOCK_EDIT_H) {
			clockString[0] = ' ';
			clockString[1] = ' ';
		}
		if (clkEdit == CLOCK_EDIT_M) {
			clockString[3] = ' ';
			clockString[4] = ' ';
		}
	}

	if (time.tsec < 6)
		clockString[2] = ':';
	else
		clockString[2] = '`';

	return clockString;
}


ISR (TIMER2_COMP_vect)
{
	static int16_t btnCnt = 0;					/* Buttons press duration value */
	static uint8_t btnPrev = 0;					/* Previous buttons state */

	uint8_t btnNow = ~BTN_PIN & BTN_MASK;		/* Current buttons state */

	/* If button event has happened, place it to command buffer */
	if (btnNow) {
		if (btnNow == btnPrev) {
			btnCnt++;
			if (btnCnt == LONG_PRESS) {
				switch (btnPrev) {
				case BTN_1:
					cmdBuf = CMD_BTN_1_LONG;
					break;
				case BTN_2:
					cmdBuf = CMD_BTN_2_LONG;
					break;
				case BTN_3:
					cmdBuf = CMD_BTN_3_LONG;
					break;
				case BTN_4:
					cmdBuf = CMD_BTN_4_LONG;
					break;
				case BTN_5:
					cmdBuf = CMD_BTN_5_LONG;
					break;
				}
			}
		} else {
			btnPrev = btnNow;
		}
	} else {
		if ((btnCnt > SHORT_PRESS) && (btnCnt < LONG_PRESS)) {
			switch (btnPrev) {
			case BTN_1:
				cmdBuf = CMD_BTN_1;
				break;
			case BTN_2:
				cmdBuf = CMD_BTN_2;
				break;
			case BTN_3:
				cmdBuf = CMD_BTN_3;
				break;
			case BTN_4:
				cmdBuf = CMD_BTN_4;
				break;
			case BTN_5:
				cmdBuf = CMD_BTN_5;
				break;
			}
		}
		btnCnt = 0;
	}

	if (dispTimer)
		dispTimer--;

	if (tempTimer)
		tempTimer--;

	if (++clockTimer >= 100) {
		incClock();
		clockTimer = 0;
	}
}

uint16_t getTempTimer()
{
	return tempTimer;
}

void setTempTimer(uint16_t val)
{
	tempTimer = val;
}

uint8_t getBtnCmd(void)
{
	uint8_t ret = cmdBuf;
	cmdBuf = CMD_EMPTY;

	return ret;
}
