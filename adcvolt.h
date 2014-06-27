#ifndef ADCVOLT_H
#define ADCVOLT_H

#include <avr/io.h>

void adcInit(void);
void adcSetMux(uint8_t mux);
uint8_t adcGetVoltage(void);

#endif /* ADCVOLT_H */
