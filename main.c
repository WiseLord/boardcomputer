#include "ks0066.h"
#include "ds18x20.h"
#include "mtimer.h"
#include "adcvolt.h"
#include "display.h"

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

void hwInit(void)
{
	ks0066Init();
	adcInit();
	mTimerInit();

	sei();

	return;
}

int main(void)
{
//	uint16_t maxAccel = 4800;
	uint8_t count = 0;

	hwInit();
	ds18x20Process();
	showVoltageAll();
	_delay_ms(2000);

	uint8_t dispMode = MODE_TEMP;
	uint8_t dispModePrev = dispMode;

	while(1) {
		count = ds18x20Process();

		/* Clear screen if mode has changed */
		if (dispMode != dispModePrev)
			ks0066Clear();

		/* Show things */
		switch (dispMode) {
		case MODE_TEMP:
			showTempAll(count);
			break;
		case MODE_VOLTAGE:
			showVoltageAll();
			break;
		}

		/* Save current mode */
		dispModePrev = dispMode;

	}

	return 0;
}
