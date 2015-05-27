/*
 * uart.h
 *
 *  Created on: 25 May 2015
 *      Author: ntuckett
 */

#ifndef UART_H_
#define UART_H_
#include <stdint.h>

extern void uartInit(int channel, int sysclk, int baud);
extern uint8_t uartGetchar(int channel);
extern void uartPutchar (int channel, uint8_t ch);
extern int uartCharReceived(int channel);

#endif /* UART_H_ */
