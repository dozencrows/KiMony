//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * flash.h
 *
 *  Created on: 1 May 2015
 *      Author: ntuckett
 */

#ifndef FLASH_H_
#define FLASH_H_
#include <stdint.h>

extern void spiFlashInit();
extern void spiFlashTest();

extern void cpuFlashDownload();

#define FLASH_DATA_WATERMARK 0xBABABEBE

typedef struct _FlashDataHeader
{
    uint32_t watermark;
} FlashDataHeader;

extern uint8_t __FlashStoreBase[];
extern uint8_t __FlashStoreLimit[];

#define GET_FLASH_PTR(x) (__FlashStoreBase + sizeof(FlashDataHeader) + x)
#define FLASH_DATA_HEADER ((const FlashDataHeader*)(__FlashStoreBase))

#endif /* FLASH_H_ */
