#ifndef ADCVOLT_H
#define ADCVOLT_H

#include <avr/io.h>

void adcInit(void);
void adcSetMux(uint8_t mux);
uint16_t adcGetVoltage(uint8_t mux);

#endif /* ADCVOLT_H */
