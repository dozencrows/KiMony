//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * uart.c
 *
 *  Created on: 25 May 2015
 *      Author: ntuckett
 */
#include "uart.h"
#include <stdint.h>
#include "MKL26Z4.h"

static UART_Type* uartChannels[] = UART_BASE_PTRS;

void uartInit(int channel, int sysclk, int baud)
{
	if (channel > 0) {
		register uint16_t sbr;
		uint8_t temp;
		UART_Type* uart = uartChannels[channel - 1];

		if (channel == 1) {
			SIM_SCGC4 |= SIM_SCGC4_UART1_MASK;
		}
		else {
			SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;
		}

		/* Make sure that the transmitter and receiver are disabled while we
		* change settings.
		*/
		UART_C2_REG(uart) &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK );

		/* Configure the uart for 8-bit mode, no parity */
		UART_C1_REG(uart) = 0;	/* We need all default settings, so entire register is cleared */

		/* Calculate baud settings */
		sbr = (uint16_t)((sysclk)/(baud * 16));

		/* Save off the current value of the uartx_BDH except for the SBR field */
		temp = UART_BDH_REG(uart) & ~(UART_BDH_SBR(0x1F));

		UART_BDH_REG(uart) = temp |  UART_BDH_SBR(((sbr & 0x1F00) >> 8));
		UART_BDL_REG(uart) = (uint8_t)(sbr & UART_BDL_SBR_MASK);

		/* Enable receiver and transmitter */
		UART_C2_REG(uart) |= (UART_C2_TE_MASK | UART_C2_RE_MASK );
	}
}

uint8_t uartGetchar(int channel)
{
	if (channel > 0) {
		UART_Type* uart = uartChannels[channel - 1];

		/* Wait until character has been received */
		while (!(UART_S1_REG(uart) & UART_S1_RDRF_MASK));

		/* Return the 8-bit data from the receiver */
		return UART_D_REG(uart);
	}
	else {
		return 0;
	}
}

void uartPutchar(int channel, uint8_t ch)
{
	if (channel > 0) {
		UART_Type* uart = uartChannels[channel - 1];

		/* Wait until space is available in the FIFO */
		while(!(UART_S1_REG(uart) & UART_S1_TDRE_MASK));

		/* Send the character */
		UART_D_REG(uart) = (uint8_t)ch;
	}
}

int uartCharReceived(int channel)
{
	if (channel > 0) {
		UART_Type* uart = uartChannels[channel - 1];
		return (UART_S1_REG(uart) & UART_S1_RDRF_MASK);
	}
	else {
		return 0;
	}
}
