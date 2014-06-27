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

void showRPM(uint16_t rpm)
{
	uint16_t maxRPM = 4800;

	ks0066SetXY(12, 0);
	ks0066WriteString(mkNumString(rpm, 4, 0));
	ks0066ShowBar(rpm, maxRPM);
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
			showTemp(count);
		} else {
			ks0066Clear();
			for (i = 0; i <= maxAccel; i+=100) {
				showVoltage(ADCMUX_BATTERY, 0, 0);
				showVoltage(ADCMUX_VOLTS, 6, 0);
				showRPM(i);
				_delay_ms(100);
			}
		}
	}

	return 0;
}
