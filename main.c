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
	uint8_t count = 0;
	uint8_t cmd = CMD_EMPTY;
	static uint16_t rpm = 0;

	hwInit();
	ds18x20Process();

	uint8_t dispMode = MODE_VOLTAGE;
	uint8_t dispModeTemp = MODE_TEMP;
	uint8_t dispModeRpm = MODE_RPM;
	uint8_t dispModeVoltage = MODE_VOLTAGE;
	uint8_t dispModeClock = MODE_CLOCK;

	uint8_t dispModePrev = dispMode;

	while(1) {
		count = ds18x20Process();
		cmd = getBtnCmd();

		rpm++;
		if (rpm > 7200)
			rpm = 0;

		/* Handle command */
		switch (cmd) {
		case CMD_BTN_1:
			if (dispMode == MODE_CLOCK_EDIT_H) {
				clockDecHour();
				break;
			}
			if (dispMode == MODE_CLOCK_EDIT_M) {
				clockDecMin();
				break;
			}
			dispMode = dispModeVoltage;
			break;
		case CMD_BTN_1_LONG:
			if (dispMode == MODE_CLOCK_EDIT_H || dispMode == MODE_CLOCK_EDIT_M) {
				break;
			}
			if (dispMode == MODE_BIG_VOLT_BATTERY)
				dispModeVoltage = MODE_BIG_VOLT_BOARD;
			else if (dispMode == MODE_VOLTAGE)
				dispModeVoltage = MODE_BIG_VOLT_BATTERY;
			else
				dispModeVoltage = MODE_VOLTAGE;

			dispMode = dispModeVoltage;
			break;
		case CMD_BTN_2:
			if (dispMode == MODE_CLOCK_EDIT_H) {
				clockIncHour();
				break;
			}
			if (dispMode == MODE_CLOCK_EDIT_M) {
				clockIncMin();
				break;
			}
			dispMode = dispModeTemp;
			break;
		case CMD_BTN_2_LONG:
			if (dispMode == MODE_CLOCK_EDIT_H || dispMode == MODE_CLOCK_EDIT_M) {
				break;
			}
			if (dispMode == MODE_BIG_TEMP_CAR)
				dispModeTemp = MODE_BIG_TEMP_OUT;
			else if (dispMode == MODE_TEMP)
				dispModeTemp = MODE_BIG_TEMP_CAR;
			else
				dispModeTemp = MODE_TEMP;
			dispMode = dispModeTemp;
			break;
		case CMD_BTN_3:
			if (dispMode == MODE_CLOCK_EDIT_H || dispMode == MODE_CLOCK_EDIT_M) {
				break;
			}
			dispMode = dispModeRpm;
			break;
		case CMD_BTN_3_LONG:
			if (dispMode == MODE_CLOCK_EDIT_H || dispMode == MODE_CLOCK_EDIT_M) {
				break;
			}
			if (dispMode == MODE_RPM)
				dispModeRpm = MODE_BIG_RPM;
			else
				dispModeRpm = MODE_RPM;
			dispMode = dispModeRpm;
			break;
		case CMD_BTN_4:
			if (dispMode == MODE_CLOCK_EDIT_H)
				dispModeClock = MODE_CLOCK_EDIT_M;
			else if (dispMode == MODE_CLOCK_EDIT_M)
				dispModeClock = MODE_CLOCK_EDIT_H;
			else dispModeClock = MODE_CLOCK;
			dispMode = dispModeClock;
			break;
		case CMD_BTN_4_LONG:
			if (dispMode == MODE_CLOCK)
				dispModeClock = MODE_CLOCK_EDIT_H;
			else
				dispModeClock = MODE_CLOCK;
			dispMode = dispModeClock;
			break;
		default:
			break;
		}

		/* Clear screen if mode has changed */
		if (dispMode != dispModePrev)
			ks0066Clear();

		/* Show things */
		switch (dispMode) {
		case MODE_TEMP:
			showTemp(count);
			break;
		case MODE_BIG_TEMP_OUT:
			showBigTemp(TEMP_SENSOR_OUT);
			break;
		case MODE_BIG_TEMP_CAR:
			showBigTemp(TEMP_SENSOR_CAR);
			break;
		case MODE_VOLTAGE:
			showVoltageAll();
			break;
		case MODE_BIG_VOLT_BATTERY:
			showBigVoltage(VOLTAGE_BATTERY);
			break;
		case MODE_BIG_VOLT_BOARD:
			showBigVoltage(VOLTAGE_BOARD);
			break;
		case MODE_CLOCK:
			showClock(getClock(CLOCK_NOEDIT));
			break;
		case MODE_CLOCK_EDIT_H:
			showClock(getClock(CLOCK_EDIT_H));
			break;
		case MODE_CLOCK_EDIT_M:
			showClock(getClock(CLOCK_EDIT_M));
			break;
		case MODE_RPM:
			showRPM(rpm);
			break;
		case MODE_BIG_RPM:
			showBigRPM(rpm);
			break;
		}

		/* Save current mode */
		dispModePrev = dispMode;

	}

	return 0;
}
