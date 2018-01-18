/*
 * Clint Olsen and Matt Zarekani
 * ECEN 3360
 * Final Project: Automatic Dog Door
 *
 */

#include "msp.h"
#include "uart.h"
#include "timer.h"
#include "gpio.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

//Interrupt handlers
void EUSCIA0_IRQHandler(void);
void EUSCIA3_IRQHandler(void);
void TA0_0_IRQHandler(void);

volatile uint32_t period = 10; //time in milliseconds
volatile uint32_t duty_cycle = 10; //i.e. 50 indicates 50% duty cycle
volatile uint32_t timerA0_counter = 0;
volatile uint32_t temp = 0; //for timer


//door variables
uint8_t smartLockOn = 0;
uint8_t locked = 0;
uint8_t unlocked = 1;
uint8_t servoOn = 0;

//dog variables
uint8_t dog1Blocked = 0;
uint8_t dog2Blocked = 0;
uint8_t dog1Inside = 1;
uint8_t dog2Inside = 1;
uint8_t changeDog1 = 0;
uint8_t changeDog2 = 0;
volatile int8_t rssiDog1Prev = -80;
volatile int8_t rssiDog2Prev = -80;
extern volatile uint8_t rssiPassDog1;
extern volatile uint8_t rssiPassDog2;


int counter = 0;
const int size = 400;

int startCollection = 0;

uint8_t response[size];

int done = 0;


void main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    //Peripheral configuration
    configure_serial_portA0();
    configure_serial_portA3();
    configure_pins();
    configure_timer();

    centralOn();
    scanOn();

    int i;
    while(1){
    	//if a full string was recieved, check for the correct addresses
    	if(done == 1){
    		checkBLEResponse(response, counter);
    		//In this case we want BLE to open the dogs only for specific dogs
    		if(rssiPassDog1 == 1 && dog1Blocked == 0 && smartLockOn == 1){
    			//scanOff();
    			if(dog1Inside == 1){
    				//dog 1 went outside
    				dog1Inside = 0;
    				uart_putcharA3('1');
    				uart_putcharA3('O');
    				uart_putcharA3('\r');
    			}
    			else{
    				//dog 1 went inside
    				dog1Inside = 1;
    				uart_putcharA3('1');
    				uart_putcharA3('I');
    				uart_putcharA3('\r');
    			}
    			rssiPassDog1 = 0;
    			duty_cycle = 10; //set unlock duty cycle
    			unlocked = 1;
    			locked = 0;
    			servoOn = 1;
    			TA0CCTL0 |= TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
    			for(i = 0; i < 1000000; i++); //delay for dog to walk through
    			servoOn = 1; //indicate servo turn to timer
    			duty_cycle = 20; //set lock duty cycle
    			unlocked = 0;
    			locked = 1;
    			TA0CCTL0 |= TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
    		}
    		else if(rssiPassDog2 == 1 && dog2Blocked == 0 && smartLockOn == 1){
        			//scanOff();
					if(dog2Inside == 1){
						//dog 2 went outside
						dog2Inside = 0;
						uart_putcharA3('2');
						uart_putcharA3('O');
						uart_putcharA3('\r');
					}
					else{
						//dog 2 went inside
						dog2Inside = 1;
						uart_putcharA3('2');
						uart_putcharA3('I');
						uart_putcharA3('\r');
					}
        			rssiPassDog2 = 0;
        			duty_cycle = 10; //set unlock duty cycle
        			unlocked = 1;
        			locked = 0;
        			servoOn = 1;
        			TA0CCTL0 |= TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
        			for(i = 0; i < 1000000; i++); //delay for dog to walk through
        			servoOn = 1; //indicate servo turn to timer
        			duty_cycle = 20; //set lock duty cycle
        			unlocked = 0;
        			locked = 1;
        			TA0CCTL0 |= TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
        		}
    		else if(rssiPassDog1 == 1 && smartLockOn == 0){
    			//send command to say dog went in or out
    			rssiPassDog1 = 0;
    			if(dog1Inside == 1){
    				//dog 1 went outside
    				dog1Inside = 0;
    				uart_putcharA3('1');
    				uart_putcharA3('O');
    				uart_putcharA3('\r');
    			}
    			else{
    				//dog 1 went inside
    				dog1Inside = 1;
    				uart_putcharA3('1');
    				uart_putcharA3('I');
    				uart_putcharA3('\r');
    			}
    			for(i = 0; i < 500000; i++); //delay for dog to walk through

    		}
    		else if(rssiPassDog2 == 1 && smartLockOn == 0){
    			//send command to say dog went in or out
    			rssiPassDog2 = 0;
    			if(dog2Inside == 1){
    				//dog 2 went outside
    				dog2Inside = 0;
    				uart_putcharA3('2');
    				uart_putcharA3('O');
    				uart_putcharA3('\r');
    			}
    			else{
    				//dog 2 went inside
    				dog2Inside = 1;
    				uart_putcharA3('2');
    				uart_putcharA3('I');
    				uart_putcharA3('\r');
    			}
    			for(i = 0; i < 500000; i++); //delay for dog to walk through
    		}
    		counter = 0;
    		done = 0;
    		//centralOn();
    		//scanOn();
    		UCA0IE |= EUSCI_A__RXIE; // Enable USCI_A0 RX interrupts
    	}
    }
}
// BLE communication
void EUSCIA0_IRQHandler(void){
	uint8_t data;

	//recieve interrupts
	if (UCA0IFG & UCRXIFG){
		//code to handle RX interrupts
		//prints recived data back out to terminal
		data = UCA0RXBUF;
		if(data == '\r'){
			//printf("r");
			response[counter] = data;
			done = 1;
			UCA0IE &= ~(EUSCI_A__RXIE); // Disable USCI_A0 RX interrupts

		}
		else{
			response[counter] = data;
			counter++;
			}
	}
	//transmit interrupts
	if (UCA0IFG & UCTXIFG){

	}
}

//For Bluetooth module to talk to Andorid App
void EUSCIA3_IRQHandler(void){
	uint8_t data;

	//recieve interrupts
	if (UCA3IFG & UCRXIFG){
		//code to handle RX interrupts
		//prints recived data back out to terminal
		data = UCA3RXBUF;
		//lock on command from smart phone
		if(smartLockOn == 0 && data == 'L'){
			if(locked == 0){
				uart_putcharA3('L');
				uart_putcharA3('\r');
				locked = 1;
				servoOn = 1;
				unlocked = 0;
				duty_cycle = 20;
				TA0CCTL0 |= TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
			}
		}
		//lock on command from smart phone
		else if(smartLockOn == 0 && data == 'U'){
			if(unlocked == 0){
				uart_putcharA3('U');
				uart_putcharA3('\r');
				unlocked = 1;
				servoOn = 1;
				locked = 0;
				duty_cycle = 10;
				TA0CCTL0 |= TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
			}

		}
		//lock on command from smart phone
		else if(data == 'S'){
			if(locked == 0 && smartLockOn == 0){
				smartLockOn = 1;
				servoOn = 1;
				locked = 1;
				unlocked = 0;
				duty_cycle = 20;
				TA0CCTL0 |= TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
			    centralOn();
			    scanOn();
			}
			else if(locked == 1 && smartLockOn == 0){
				smartLockOn = 1;
			}
			else if(smartLockOn == 1){
				smartLockOn = 0;
			}

		}
		else if(data == 'C'){
			if(dog1Inside == 1){
				uart_putcharA3('1');
				uart_putcharA3('I');
				uart_putcharA3('\r');

			}
			else{
				uart_putcharA3('1');
				uart_putcharA3('O');
				uart_putcharA3('\r');

			}
			if(dog2Inside == 1){
				uart_putcharA3('2');
				uart_putcharA3('I');
				uart_putcharA3('\r');

			}
			else{
				uart_putcharA3('2');
				uart_putcharA3('O');
				uart_putcharA3('\r');

			}
		}
		else if(changeDog1 == 1 && data == 'A'){
			dog1Blocked = 0;
			changeDog1 = 0;
		}
		else if(changeDog1 == 1 && data == 'B'){
			dog1Blocked = 1;
			changeDog1 = 0;
		}
		else if(changeDog1 == 1 && data == 'I'){
			dog1Inside = 1;
			changeDog1 = 0;
		}
		else if(changeDog1 == 1 && data == 'O'){
			dog1Inside = 0;
			changeDog1 = 0;
		}
		else if(changeDog2 == 1 && data == 'A'){
			dog2Blocked = 0;
			changeDog2 = 0;
		}
		else if(changeDog2 == 1 && data == 'B'){
			dog2Blocked = 1;
			changeDog2 = 0;
		}
		else if(changeDog2 == 1 && data == 'I'){
			dog2Inside = 1;
			changeDog2 = 0;
		}
		else if(changeDog2 == 1 && data == 'O'){
			dog2Inside = 0;
			changeDog2 = 0;
		}
		else if(data == '1'){
			changeDog1 = 1;
		}
		//change dog 2 settings
		else if(data == '2'){
			changeDog2 = 1;
		}
		printf("%c", data);

	}
	//transmit interrupts
	if (UCA3IFG & UCTXIFG){

	}
}

//PWM output for servos
void TA0_0_IRQHandler(void) {
	//turn off timer interrupt
	TA0CCTL0 &= ~CCIFG;

	timerA0_counter++;

	//PWM production from Port 2 Pin 0
	if (servoOn == 1 && timerA0_counter % period == 0) {
		temp = timerA0_counter; //store the time a new period starts
		P2OUT |= BIT5;
		P3OUT |= BIT0;
	} else if (servoOn == 1 && timerA0_counter == (temp + (uint32_t) ((double) period * (double) duty_cycle/ (double) 100))) {
		P2OUT &= ~BIT5;
		P3OUT &= ~BIT0;

	}

	if(servoOn == 1 && timerA0_counter >= 200 && locked == 1){
		TA0CCTL0 &= ~TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
		timerA0_counter = 0;
		servoOn = 0;
	}

	if(servoOn == 1 && timerA0_counter >= 250 && unlocked == 1){
		TA0CCTL0 &= ~TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
		timerA0_counter = 0;
		servoOn = 0;
	}

}


