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
	hwInit();

	uint16_t maxAccel = 4800;
	uint8_t count = 0;
	uint16_t i;

	while(1) {
		count = ds18x20Process();
		if (count) {
//			showTemp(count);
			for (i = 0; i < count; i++) {
				showBigTemp(i);
				_delay_ms(2000);
			}
		} else {
//			ks0066Clear();
			for (i = 0; i <= maxAccel; i+=100) {
//				showVoltage(ADCMUX_BATTERY, 0, 0);
//				showVoltage(ADCMUX_VOLTS, 6, 0);
//				showRPM(i);
				showBigRPM(i);
				_delay_ms(100);
			}
		}
	}

	return 0;
}
