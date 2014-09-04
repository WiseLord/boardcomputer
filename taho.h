#ifndef TAHO_H
#define TAHO_H

#include <avr/io.h>

/* Counter definitions */
#define TAHO_DDR			DDRD
#define TAHO_PIN			PIND
#define TAHO_PORT			PORTD

#define TAHO				(1<<PD3)

#define RPM_MAX				4800

void setPpt(uint8_t value);
void setAutooff(uint8_t value);
uint8_t getAutoff(void);
uint8_t getPpt(void);
void tahoInit(uint8_t ppt, uint8_t autooff);

uint16_t getTaho();

#endif /* TAHO_H */
