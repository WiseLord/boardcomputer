#include "ks0066.h"
#include "ds18x20.h"
#include "mtimer.h"
#include "adcvolt.h"
#include "display.h"
#include "taho.h"

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#define EEPROM_PPT	((void*)0x01F0)

void hwInit(void)
{
	ks0066Init();
	adcInit();
	mTimerInit();
	tahoInit(eeprom_read_byte(EEPROM_PPT));

	sei();

	return;
}

void goStby(void)
{
	KS0066_BCKL_PORT &= ~KS0066_BCKL;

	return;
}

void exitStby(void)
{
	KS0066_BCKL_PORT |= KS0066_BCKL;

	return;
}

int main(void)
{
	uint8_t count = 0;
	uint8_t cmd = CMD_EMPTY;
	uint16_t rpm;

	hwInit();
	ds18x20Process();

	uint8_t dispMode = MODE_VOLTAGE;
	uint8_t dispModeTemp = MODE_TEMP;
	uint8_t dispModeRpm = MODE_RPM;
	uint8_t dispModeVoltage = MODE_VOLTAGE;
	uint8_t dispModeClock = MODE_CLOCK;

	uint8_t dispModePrev = dispMode;
	uint8_t dispModePrevActive = dispMode;

	while(1) {
		count = ds18x20Process();
		cmd = getBtnCmd();
		rpm = getTaho();

		/* Don't handle any command in standby mode except power on */
		if (dispMode == MODE_STANDBY) {
			if (cmd != CMD_BTN_5)
				cmd = CMD_EMPTY;
		}

		/* Handle command */
		switch (cmd) {
		case CMD_BTN_1:
			if (dispMode == MODE_EDIT_RPM) {
				setPpt(getPpt() - 1);
				break;
			}
			if (dispMode == MODE_EDIT_H) {
				clockDecHour();
				break;
			}
			if (dispMode == MODE_EDIT_M) {
				clockDecMin();
				break;
			}
			if (dispMode == MODE_BIG_VOLT_BATTERY)
				dispModeVoltage = MODE_BIG_VOLT_BOARD;
			else if (dispMode == MODE_BIG_VOLT_BOARD)
				dispModeVoltage = MODE_VOLTAGE;
			else if (dispMode == MODE_VOLTAGE)
				dispModeVoltage = MODE_BIG_VOLT_BATTERY;

			dispMode = dispModeVoltage;
			break;
		case CMD_BTN_2:
			if (dispMode == MODE_EDIT_RPM) {
				setPpt(getPpt() + 1);
				break;
			}
			if (dispMode == MODE_EDIT_H) {
				clockIncHour();
				break;
			}
			if (dispMode == MODE_EDIT_M) {
				clockIncMin();
				break;
			}
			if (dispMode == MODE_BIG_TEMP_CAR)
				dispModeTemp = MODE_BIG_TEMP_OUT;
			else if (dispMode == MODE_BIG_TEMP_OUT)
				dispModeTemp = MODE_TEMP;
			else if (dispMode == MODE_TEMP)
				dispModeTemp = MODE_BIG_TEMP_CAR;
			dispMode = dispModeTemp;
			break;
		case CMD_BTN_3:
			if (dispMode == MODE_EDIT_H || dispMode == MODE_EDIT_M || dispMode == MODE_EDIT_RPM) {
				break;
			}
			if (dispMode == MODE_RPM)
				dispModeRpm = MODE_BIG_RPM;
			else if (dispMode == MODE_BIG_RPM)
				dispModeRpm = MODE_RPM;
			dispMode = dispModeRpm;
			break;
		case CMD_BTN_3_LONG:
			if (dispMode == MODE_EDIT_H || dispMode == MODE_EDIT_M) {
				break;
			}
			if (dispMode == MODE_RPM || dispMode == MODE_BIG_RPM)
				dispModeRpm = MODE_EDIT_RPM;
			else if (dispMode == MODE_EDIT_RPM) {
				dispModeRpm = MODE_RPM;
				eeprom_update_byte(EEPROM_PPT, getPpt());
			}
			dispMode = dispModeRpm;
			break;
		case CMD_BTN_4:
			if (dispMode == MODE_EDIT_RPM) {
				break;
			}
			if (dispMode == MODE_EDIT_H)
				dispModeClock = MODE_EDIT_M;
			else if (dispMode == MODE_EDIT_M)
				dispModeClock = MODE_EDIT_H;
			else dispModeClock = MODE_CLOCK;
			dispMode = dispModeClock;
			break;
		case CMD_BTN_4_LONG:
			if (dispMode == MODE_EDIT_RPM) {
				break;
			}
			if (dispMode == MODE_CLOCK)
				dispModeClock = MODE_EDIT_H;
			else
				dispModeClock = MODE_CLOCK;
			dispMode = dispModeClock;
			break;
		case CMD_BTN_5:
			if (dispMode == MODE_STANDBY) {
				exitStby();
				dispMode = dispModePrevActive;
			} else {
				goStby();
				dispMode = MODE_STANDBY;
			}
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
		case MODE_EDIT_H:
			showClock(getClock(CLOCK_EDIT_H));
			break;
		case MODE_EDIT_M:
			showClock(getClock(CLOCK_EDIT_M));
			break;
		case MODE_RPM:
			showRPM(rpm);
			break;
		case MODE_BIG_RPM:
			showBigRPM(rpm);
			break;
		case MODE_EDIT_RPM:
			showEditRPM(rpm);
			break;
		case MODE_STANDBY:
			showClock(getClock(CLOCK_NOEDIT));
			break;
		}

		/* Save current mode */
		dispModePrev = dispMode;
		if (dispMode != MODE_STANDBY)
			dispModePrevActive = dispMode;

	}

	return 0;
}
