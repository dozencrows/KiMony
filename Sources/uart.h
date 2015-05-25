/*
 * uart.h
 *
 *  Created on: 25 May 2015
 *      Author: ntuckett
 */

#ifndef UART_H_
#define UART_H_

extern void uartInit(int channel, int sysclk, int baud);
extern char uartGetchar(int channel);
extern void uartPutchar (int channel, char ch);
extern int uartCharReceived(int channel);

#endif /* UART_H_ */
