/*
 * Clint Olsen and Matt Zarekani
 * ECEN 3360
 * Final Project: Automatic Dog Door
 *
 */
#include "gpio.h"
#include "msp.h"
#include <stdint.h>

void configure_pins(void) {
	/* Port configuration code here for the switches or LEDs */

	//Configure LED
		P2DIR |= BIT5;
		P2OUT &= ~BIT5;
		P3DIR |= BIT0;
		P3OUT &= ~BIT0;

}


