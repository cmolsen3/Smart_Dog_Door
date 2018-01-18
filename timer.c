/*
 * Clint Olsen and Matt Zarekani
 * ECEN 3360
 * Final Project: Automatic Dog Door
 *
 */

#include "timer.h"
#include "msp.h"
#include <stdint.h>


void configure_timer(void) {
	TA0CCR0 = 374; // Capture Compare Value (1 ms)
	//TA0CCTL0 = TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
	// up mode, SMCLK, enable
	TA0CTL = TIMER_A_CTL_MC__UP | TIMER_A_CTL_TASSEL_2  | TIMER_A_CTL_ID__8; //|  TIMER_A_CTL_IE
	TA0R = 0;
	/* NVIC Port Interrupt Enable Code */  // Enable interrupt in NVIC
	NVIC_EnableIRQ(TA0_0_IRQn);

}

void enable_timer_interrupts(void){
	TA0CCTL0 |= TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
}

void disable_timer_interrrupts(void){
	TA0CCTL0 &= ~TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
}



