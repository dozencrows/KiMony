/*
 * spi.h
 *
 *  Created on: 7 May 2015
 *      Author: ntuckett
 */

#ifndef SPI_H_
#define SPI_H_
#include <stdint.h>

extern void spiInit();
extern void spiWrite(uint8_t byte);
extern uint8_t spiRead();

#endif /* SPI_H_ */
