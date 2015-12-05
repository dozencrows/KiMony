//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

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
extern void spiSetBitRate(int prescaler, int divider);
extern void spiWrite(uint8_t byte);
extern uint8_t spiRead();
extern void spiPinsDisconnect();
extern void spiPinsConnect();

#endif /* SPI_H_ */

