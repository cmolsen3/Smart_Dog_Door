/*
 * Clint Olsen and Matt Zarekani
 * ECEN 3360
 * Final Project: Automatic Dog Door
 *
 */

#ifndef UART_H_
#define UART_H_

#include <stdint.h>


void configure_serial_portA0(void);
void configure_serial_portA3(void);
void uart_putchar_nA0(uint8_t * data, uint32_t length);
void uart_putcharA0(uint8_t tx_data);
void checkBLEResponse(uint8_t *response, int counter);
void uart_putcharA3(uint8_t tx_data);


//BLE AT Commands
void restore();
void storeChanges();
void scanDog1();
void setScanTimeout();
void setScanInterval();
void cconOff(void);
void connect();
void reset();
void centralOff();
void centralOn();
void scanOff();
void scanOn();
void config(void);
void status(void);
void advOff(void);
void advOn(void);




#endif /* UART_H_ */
