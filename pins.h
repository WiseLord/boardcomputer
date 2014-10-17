#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define CONCAT(x,y)			x ## y

#define DDR(x)				CONCAT(DDR,x)
#define PORT(x)				CONCAT(PORT,x)
#define PIN(x)				CONCAT(PIN,x)

/* KS0066 Data port*/
#define KS0066_D4			B
#define KS0066_D4_LINE		(1<<0)
#define KS0066_D5			B
#define KS0066_D5_LINE		(1<<1)
#define KS0066_D6			B
#define KS0066_D6_LINE		(1<<2)
#define KS0066_D7			B
#define KS0066_D7_LINE		(1<<3)
/* KS0066 control port*/
#define KS0066_RS			D
#define KS0066_RS_LINE		(1<<5)
#define KS0066_RW			D
#define KS0066_RW_LINE		(1<<6)
#define KS0066_E			D
#define KS0066_E_LINE		(1<<7)

/* Backlight port */
#define BCKL				D
#define BCKL_LINE			(1<<4)

#endif /* PINS_H */
