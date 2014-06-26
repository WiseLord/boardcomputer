#include "ds18x20.h"

#include "mtimer.h"

#include <util/delay.h>

static ds18x20Dev devs[DS18X20_MAX_DEV];
static uint8_t devCount = 0;

static uint8_t ds18x20IsOnBus(void)
{
	uint8_t ret;

	DS18X20_DDR |= DS18X20_WIRE;		/* Pin as output (0) */
	DS18X20_PORT &= ~DS18X20_WIRE;		/* Set active 0 */
	_delay_us(485);						/* Reset */
	DS18X20_DDR &= ~DS18X20_WIRE;		/* Pin as input (1) */
	_delay_us(65);						/* Wait for response */

	ret = !(DS18X20_PIN & DS18X20_WIRE);

	_delay_us(420);

	return ret;
}

static void ds18x20SendBit(uint8_t bit)
{
	DS18X20_DDR |= DS18X20_WIRE;		/* Pin as output (0) */
	DS18X20_PORT &= ~DS18X20_WIRE;		/* Set active 0 */
	_delay_us(5);
	if (!bit)
		_delay_us(60);
	DS18X20_DDR &= ~DS18X20_WIRE;		/* Pin as input (1) */
	_delay_us(5);
	if (bit)
		_delay_us(60);

	return;
}

static uint8_t ds18x20GetBit(void)
{
	uint8_t ret;

	DS18X20_DDR |= DS18X20_WIRE;		/* pin as output */
	DS18X20_PORT &= ~DS18X20_WIRE;		/* Set active 0 */
	_delay_us(2);						/* strob */
	DS18X20_DDR &= ~DS18X20_WIRE;		/* pin as input */
	_delay_us(7);

	ret = DS18X20_PIN & DS18X20_WIRE;

	_delay_us(50);

	return ret;
}

static void ds18x20SendByte(uint8_t byte)
{
	uint8_t i;

	for (i = 0; i < 8; i++)
		ds18x20SendBit(byte & (1<<i));

	return;
}

static uint8_t ds18x20GetByte(void)
{
	uint8_t i, ret = 0;

	for (i = 0; i < 8; i++)
		if (ds18x20GetBit())
			ret |= (1<<i);
		else
			ret &= ~(1<<i);

	return ret;
}

static void ds18x20Select(ds18x20Dev *dev)
{
	uint8_t i;

	ds18x20SendByte(DS18X20_CMD_MATCH_ROM);

	for (i = 0; i < 8; i++)
		ds18x20SendByte(dev->id[i]);

	return;
}

static void getAllTemps()
{
	uint8_t i;

	for (i = 0; i < devCount; i++) {
		if (ds18x20IsOnBus()) {
			ds18x20Select(&devs[i]);
			ds18x20SendByte(DS18X20_CMD_READ_SCRATCH);	// Get temperature from device
			devs[i].tempData = ds18x20GetByte() | (ds18x20GetByte() << 8);
		}
	}

	return;
}

static void convertTemp(void)
{
	ds18x20SendByte(DS18X20_CMD_SKIP_ROM);
	ds18x20SendByte(DS18X20_CMD_CONVERT);

#ifdef DS18X20_PARASITE_POWER
	/* Set active 1 on port for 750ms as parasitic power */
	DS18X20_PORT |= DS18X20_WIRE;
	DS18X20_DDR |= DS18X20_WIRE;
#endif

	return;
}

static uint8_t ds18x20SearchRom(uint8_t *bitPattern, uint8_t lastDeviation)
{
	uint8_t currBit;
	uint8_t newDeviation = 0;
	uint8_t bitMask = 0x01;
	uint8_t bitA;
	uint8_t bitB;

	/* Send SEARCH ROM command on the bus */
	ds18x20SendByte(DS18X20_CMD_SEARCH_ROM);

	/* Walk through all 64 bits */
	for (currBit = 0; currBit < 64; currBit++)
	{
		// Read bit from bus twice.
		bitA = ds18x20GetBit();
		bitB = ds18x20GetBit();

		if (bitA && bitB) { /* Both bits 1 = ERROR */
			return 0xFF;
		} else if (!(bitA || bitB)) { /* Both bits 0 */
			if (currBit == lastDeviation) { /* Select 1 if device was selected previously */
				*bitPattern |= bitMask;
			} else if (currBit > lastDeviation) { /* Select 0 if no, and remember device */
				(*bitPattern) &= ~bitMask;
				newDeviation = currBit;
			} else if (!(*bitPattern & bitMask)) { /* Otherwise just remember device */
				newDeviation = currBit;
			}
		} else { /* Bits differ */
			if (bitA)
				*bitPattern |= bitMask;
			else
				*bitPattern &= ~bitMask;
		}

		// Send the selected bit to the bus.
		ds18x20SendBit(*bitPattern & bitMask);

		// Adjust bitMask and bitPattern pointer.
		bitMask <<= 1;
		if (!bitMask)
		{
			bitMask = 0x01;
			bitPattern++;
		}
	}

	return newDeviation;
}

uint8_t ds18x20Process(void)
{
	uint8_t i, j;
	uint8_t * newID;
	uint8_t * currentID;
	uint8_t lastDeviation;
	uint8_t count = 0;

	if (getTempTimer() == 0) {

		getAllTemps();

		/* Reset addresses */
		for (i = 0; i < DS18X20_MAX_DEV; i++)
			for (j = 0; j < 8; j++)
				devs[i].id[j] = 0x00;

		/* Search all sensors */
		newID = devs[0].id;
		lastDeviation = 0;
		currentID = newID;

		do {
			for (j = 0; j < 8; j++)
				newID[j] = currentID[j];

			if (!ds18x20IsOnBus()) {
				devCount = 0;

				return devCount;
			}

			lastDeviation = ds18x20SearchRom(newID, lastDeviation);

			currentID = newID;
			count++;
			newID=devs[count].id;

		} while (lastDeviation != 0);

		devCount = count;

		/* Convert temperature */
		if (ds18x20IsOnBus()) {
			convertTemp();
			setTempTimer(750);
		}
	}

	return devCount;
}

int16_t ds18x20GetTemp(uint8_t num)
{
	if (devs[num].id[0] == 0x28)
		return devs[num].tempData * 5 / 8;

	return devs[num].tempData * 2;
}
