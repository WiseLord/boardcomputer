#include "ks0066.h"
#include <util/delay.h>

static uint8_t strBuf[] = "                ";	/* String buffer */

uint8_t *mkNumString(int16_t number, uint8_t width)
{
	uint8_t sign = ' ';
	int8_t pos;

	if (number < 0) {
		sign = '-';
		number = -number;
	}

	/* Clear buffer and go to it's tail */
	for (pos = 0; pos < width; pos++)
		strBuf[pos] = ' ';
	strBuf[width] = '\0';
	pos = width - 1;

	/* Fill buffer from right to left */
	do {
		strBuf[pos] = number % 10 + 0x30;
		pos--;
		number /= 10;
	} while (number > 0);

	if (pos >= 0)
		strBuf[pos] = sign;

	return strBuf;
}

int main(void)
{
	ks0066Init();

	uint8_t max = 48;
	uint8_t i;

	while(1) {
		for (i = 0; i <= max; i++) {
			ks0066SetXY(13, 0);
			ks0066WriteString(mkNumString(i, 3));
			ks0066ShowBar(i, max);
			_delay_ms(100);
		}
	}

	return 0;
}
