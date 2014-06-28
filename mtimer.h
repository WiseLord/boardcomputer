#ifndef MTIMER_H
#define MTIMER_H

#include <avr/io.h>

/* Buttons definitions */
#define BTN_DDR		DDRC
#define BTN_PIN		PINC
#define BTN_PORT	PORTC

#define BTN_1		(1<<PC5)
#define BTN_2		(1<<PC4)
#define BTN_3		(1<<PC3)
#define BTN_4		(1<<PC2)
#define BTN_5		(1<<PC1)

#define BTN_MASK	(BTN_1 | BTN_2 | BTN_3 | BTN_4 | BTN_5)

// Handling long press actions */
#define SHORT_PRESS		50
#define LONG_PRESS		500

enum {
	CMD_BTN_1,
	CMD_BTN_2,
	CMD_BTN_3,
	CMD_BTN_4,
	CMD_BTN_5,
	CMD_BTN_1_LONG,
	CMD_BTN_2_LONG,
	CMD_BTN_3_LONG,
	CMD_BTN_4_LONG,
	CMD_BTN_5_LONG,

	CMD_EMPTY = 0xFF
};

void mTimerInit(void);

uint16_t getTempTimer();
void setTempTimer(uint16_t val);

uint8_t getBtnCmd(void);

#endif /* MTIMER_H */
