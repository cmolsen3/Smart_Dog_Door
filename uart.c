/*
 * Clint Olsen and Matt Zarekani
 * ECEN 3360
 * Final Project: Automatic Dog Door
 *
 */
#include "uart.h"
#include "msp.h"
#include <stdint.h>

//UART preprocessor directives
//Baud Rate
#define br0 0x13 //9600 Baud
#define br1 0x00


#define CR 0x0D //carriage return

volatile uint8_t rssiPassDog1 = 0;
extern volatile int8_t rssiDog1Prev;
extern volatile int8_t rssiDog2Prev;
volatile uint8_t rssiPassDog2 = 0;


extern uint8_t dog1Blocked;
extern uint8_t dog2Blocked;


//Configure the serial port for the reciever specifications (BLE Module)
void configure_serial_portA0(){
	UCA0CTLW0 |= UCSWRST;       // Put eUSCI in reset
	P1SEL0 |= BIT2;				//Rx Primary mode
	P1SEL1 &= ~BIT2;

	P1SEL0 |= BIT3;				//Tx Primary mode
	P1SEL1 &= ~BIT3;
	// Select Frame parameters and clock source
	UCA0CTLW0 |= EUSCI_A_CTLW0_SSEL__SMCLK;
	UCA0CTLW0 &= ~EUSCI_A_CTLW0_PEN; //parity disabled
	UCA0CTLW0 &= ~EUSCI_A_CTLW0_MODE0; // set to uart mode
	UCA0CTLW0 &= ~EUSCI_A_CTLW0_MODE1;
	UCA0CTLW0 &= ~EUSCI_A_CTLW0_MSB;//LSB first
	UCA0CTLW0 &= ~EUSCI_A_CTLW0_SEVENBIT;//8 bit character length
	UCA0CTLW0 &= ~EUSCI_A_CTLW0_SPB; //one stop bit one start bit is default
	UCA0MCTLW |= EUSCI_A_MCTLW_OS16;
	UCA0MCTLW |= 0x0080;
	UCA0MCTLW &= 0x00FF;
	UCA0BR0 = br0;    // Set Baud Rate
	UCA0BR1 = br1;
	UCA0CTLW0 &= ~UCSWRST;      // Initialize eUSCI
	UCA0IE |= EUSCI_A__RXIE; // Enable USCI_A0 RX interrupts


	NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);
}

//Configure the serial port for the reciever specifications (HC-05 for User input from app)
void configure_serial_portA3(void){
	UCA3CTLW0 |= UCSWRST;       // Put eUSCI in reset
	P9SEL0 |= BIT6;				//Rx Primary mode
	P9SEL1 &= ~BIT6;

	P9SEL0 |= BIT7;				//Tx Primary mode
	P9SEL1 &= ~BIT7;
	// Select Frame parameters and clock source
	UCA3CTLW0 |= EUSCI_A_CTLW0_SSEL__SMCLK;
	UCA3CTLW0 &= ~EUSCI_A_CTLW0_PEN; //parity disabled
	UCA3CTLW0 &= ~EUSCI_A_CTLW0_MODE0; // set to uart mode
	UCA3CTLW0 &= ~EUSCI_A_CTLW0_MODE1;
	UCA3CTLW0 &= ~EUSCI_A_CTLW0_MSB;//LSB first
	UCA3CTLW0 &= ~EUSCI_A_CTLW0_SEVENBIT;//8 bit character length
	UCA3CTLW0 &= ~EUSCI_A_CTLW0_SPB; //one stop bit one start bit is default
	UCA3MCTLW |= EUSCI_A_MCTLW_OS16;
	UCA3MCTLW |= 0x0080;
	UCA3MCTLW &= 0x00FF;
	UCA3BR0 = br0;    // Set Baud Rate
	UCA3BR1 = br1;
	UCA3CTLW0 &= ~UCSWRST;      // Initialize eUSCI
	UCA3IE |= EUSCI_A__RXIE; // Enable USCI_A0 RX interrupts


	//NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);
	NVIC_EnableIRQ(EUSCIA3_IRQn);

}


//sends a single 8 bit value to the tx buffer
void uart_putcharA0(uint8_t tx_data){
	while(!(UCA0IFG & UCTXIFG));// Blockuntil transmitter is ready
	UCA0TXBUF = tx_data;    // Load data onto buffer
}

//sends a single 8 bit value to the tx buffer
void uart_putcharA3(uint8_t tx_data){
	while(!(UCA3IFG & UCTXIFG));// Blockuntil transmitter is ready
	UCA3TXBUF = tx_data;    // Load data onto buffer
}

//sends a array of 8 bit values to tx buffer
void uart_putchar_nA0(uint8_t * data, uint32_t length){
	// Code to iterate through transmit data
	uint32_t i;
	for(i = 0; i < length; i++){
		uart_putcharA0(data[i]);
	}
}

//checks each line of scanned input to find the addresses of the dog modules
void checkBLEResponse(uint8_t *response, int counter){
	int8_t rssi;
	int i;
	if(response[0] == 'S'){
		//check dog 1 address
	    	if(response[6] == 'C' && response[10] == '9' && response[17] == '6'){
	   			if(response[26] >= (uint8_t)65 && response[26] <= (uint8_t)70){
	  					rssi = response[26] - (uint8_t)55;
   				}
	    		else if(response[26] >= (uint8_t)48 && response[26] <= (uint8_t)57){
	    			rssi = response[26] - (uint8_t)48;
	    		}
	    		if(response[25] >= (uint8_t)65 && response[25] <= (uint8_t)70){
	    			rssi |= ((response[25] - (uint8_t)55) << 0x04);
	    		}
	    		else if(response[25] >= (uint8_t)48 && response[25] <= (uint8_t)57){
	   				rssi |= ((response[25] - (uint8_t)48) << 0x04);

   				}
	  		   		for(i = 0; i < counter; i++){
	    		    	printf("%c", response[i]);
	    		    }

	    		   printf("RSSI: %d", rssi);
	    		   printf("\n");
	    		   rssiDog1Prev = rssi;
	    		   if(rssi > -50 && rssiDog2Prev < -60){
	    			   rssiPassDog1 = 1;
	    		   }
	    		   else if(rssi > -50 && dog2Blocked == 0){
	    			   rssiPassDog1 = 1;
	    		   }
	    	}
			//check dog 2 address
	    	if(response[6] == 'D' && response[10] == '8' && response[17] == '6'){
	   			if(response[26] >= (uint8_t)65 && response[26] <= (uint8_t)70){
	  					rssi = response[26] - (uint8_t)55;
   				}
	    		else if(response[26] >= (uint8_t)48 && response[26] <= (uint8_t)57){
	    			rssi = response[26] - (uint8_t)48;
	    		}
	    		if(response[25] >= (uint8_t)65 && response[25] <= (uint8_t)70){
	    			rssi |= ((response[25] - (uint8_t)55) << 0x04);
	    		}
	    		else if(response[25] >= (uint8_t)48 && response[25] <= (uint8_t)57){
	   				rssi |= ((response[25] - (uint8_t)48) << 0x04);

   				}
	  		   		for(i = 0; i < counter; i++){
	    		    	printf("%c", response[i]);
	    		    }
	    		   printf("RSSI: %d", rssi);
	    		   printf("\n");
	    		   rssiDog2Prev = rssi;
	    		   if(rssi > -50 && rssiDog1Prev < -60){
	    			   rssiPassDog2 = 1;
	    		   }
	    		   else if(rssi > -50 && dog1Blocked == 0){
	    			   rssiPassDog2 = 1;
	    		   }
	    	}
	    }
}

//BLE AT Command Set
void advOn(void){
	uart_putcharA0('A');
	uart_putcharA0('D');
	uart_putcharA0('V');
	uart_putcharA0(' ');
	uart_putcharA0('O');
	uart_putcharA0('N');
	uart_putcharA0('\r');
}

void advOff(void){
	uart_putcharA0('A');
	uart_putcharA0('D');
	uart_putcharA0('V');
	uart_putcharA0(' ');
	uart_putcharA0('O');
	uart_putcharA0('F');
	uart_putcharA0('F');
	uart_putcharA0('\r');
}

void status(void){
    uart_putcharA0('S');
    uart_putcharA0('T');
    uart_putcharA0('S');
    uart_putcharA0('\r');
}

void config(void){
    uart_putcharA0('C');
    uart_putcharA0('F');
    uart_putcharA0('G');
    uart_putcharA0('\r');
}

void scanOn(){
	uart_putcharA0('S');
	uart_putcharA0('C');
	uart_putcharA0('N');
	uart_putcharA0(' ');
	uart_putcharA0('O');
	uart_putcharA0('N');
	uart_putcharA0('\r');
}

void scanOff(){
	uart_putcharA0('S');
	uart_putcharA0('C');
	uart_putcharA0('N');
	uart_putcharA0(' ');
	uart_putcharA0('O');
	uart_putcharA0('F');
	uart_putcharA0('F');
	uart_putcharA0('\r');
}

void centralOn(){
	uart_putcharA0('S');
	uart_putcharA0('E');
	uart_putcharA0('T');
	uart_putcharA0(' ');
    uart_putcharA0('C');
    uart_putcharA0('E');
    uart_putcharA0('N');
    uart_putcharA0('T');
    uart_putcharA0('=');
    uart_putcharA0('O');
    uart_putcharA0('N');
    uart_putcharA0('\r');

}

void centralOff(){
	uart_putcharA0('S');
	uart_putcharA0('E');
	uart_putcharA0('T');
	uart_putcharA0(' ');
    uart_putcharA0('C');
    uart_putcharA0('E');
    uart_putcharA0('N');
    uart_putcharA0('T');
    uart_putcharA0('=');
    uart_putcharA0('O');
    uart_putcharA0('F');
    uart_putcharA0('F');
    uart_putcharA0('\r');

}

void reset(){
    uart_putcharA0('R');
    uart_putcharA0('S');
    uart_putcharA0('T');
    uart_putcharA0('\r');
}

void connect(){
	uart_putcharA0('C');
	uart_putcharA0('O');
	uart_putcharA0('N');
	uart_putcharA0(' ');
    uart_putcharA0('C');
    uart_putcharA0('1');
    uart_putcharA0('B');
    uart_putcharA0('C');
    uart_putcharA0('9');
    uart_putcharA0('C');
    uart_putcharA0('9');
    uart_putcharA0('B');
    uart_putcharA0('5');
    uart_putcharA0('7');
    uart_putcharA0('D');
    uart_putcharA0('6');
    uart_putcharA0(' ');
    uart_putcharA0('0');
    uart_putcharA0('\r');
}

void cconOff(void){
	uart_putcharA0('S');
	uart_putcharA0('E');
	uart_putcharA0('T');
	uart_putcharA0(' ');
    uart_putcharA0('C');
    uart_putcharA0('C');
    uart_putcharA0('O');
    uart_putcharA0('N');
    uart_putcharA0('=');
    uart_putcharA0('O');
    uart_putcharA0('F');
    uart_putcharA0('F');
    uart_putcharA0('\r');
}

void setScanInterval(){
	uart_putcharA0('S');
	uart_putcharA0('E');
	uart_putcharA0('T');
	uart_putcharA0(' ');
    uart_putcharA0('S');
    uart_putcharA0('C');
    uart_putcharA0('N');
    uart_putcharA0('P');
    uart_putcharA0('=');
    uart_putcharA0('0');
    uart_putcharA0('0');
    uart_putcharA0('1');
    uart_putcharA0('F');
    uart_putcharA0('4');
    uart_putcharA0('0');
    uart_putcharA0('0');
    uart_putcharA0('0');
    uart_putcharA0(' ');
    uart_putcharA0('0');
    uart_putcharA0('0');
    uart_putcharA0('0');
    uart_putcharA0('F');
    uart_putcharA0('A');
    uart_putcharA0('0');
    uart_putcharA0('0');
    uart_putcharA0('0');
    uart_putcharA0('\r');
}

void setScanTimeout(){
	uart_putcharA0('S');
	uart_putcharA0('E');
	uart_putcharA0('T');
	uart_putcharA0(' ');
    uart_putcharA0('S');
    uart_putcharA0('C');
    uart_putcharA0('N');
    uart_putcharA0('T');
    uart_putcharA0('=');
    uart_putcharA0('0');
    uart_putcharA0('0');
    uart_putcharA0('0');
    uart_putcharA0('A');
    uart_putcharA0('\r');
}

void scanDog1(){
	uart_putcharA0('S');
	uart_putcharA0('C');
	uart_putcharA0('N');
	uart_putcharA0(' ');
    uart_putcharA0('B');
    uart_putcharA0(' ');
    uart_putcharA0('C');
    uart_putcharA0('1');
    uart_putcharA0('B');
    uart_putcharA0('C');
    uart_putcharA0('9');
    uart_putcharA0('C');
    uart_putcharA0('9');
    uart_putcharA0('B');
    uart_putcharA0('5');
    uart_putcharA0('7');
    uart_putcharA0('D');
    uart_putcharA0('6');
    uart_putcharA0(' ');
    uart_putcharA0('T');
    uart_putcharA0(' ');
	uart_putcharA0('0');
	uart_putcharA0(' ');
    uart_putcharA0('N');
    uart_putcharA0(' ');
	uart_putcharA0('B');
	uart_putcharA0('C');
    uart_putcharA0('0');
	uart_putcharA0('0');
	uart_putcharA0('0');
	uart_putcharA0('0');
    uart_putcharA0('1');
    uart_putcharA0(' ');
	uart_putcharA0('F');
	uart_putcharA0(' ');
	uart_putcharA0('6');
    uart_putcharA0(' ');
    uart_putcharA0('R');
    uart_putcharA0(' ');
	uart_putcharA0('E');
	uart_putcharA0('3');
    uart_putcharA0(' ');
	uart_putcharA0('M');
	uart_putcharA0(' ');
	uart_putcharA0('0');
	uart_putcharA0('\r');
}

void storeChanges(){
    uart_putcharA0('W');
	uart_putcharA0('R');
	uart_putcharA0('T');
	uart_putcharA0('\r');
}
void restore(){
    uart_putcharA0('R');
	uart_putcharA0('T');
	uart_putcharA0('R');
	uart_putcharA0('\r');
}


