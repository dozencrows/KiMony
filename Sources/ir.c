/*
 * ir.c
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */

#include "ir.h"

#include <string.h>
#include "i2c.h"
#include "systick.h"

static const uint16_t powerCode1a[] = { 2691, 854, 467, 832, 467, 416, 467, 416, 467, 832, 934, 416, 467, 416, 467, 416, 467, 416, 467, 416, 934, 832, 467, 416, 934, 416, 467, 832, 467, 416, 467, 416, 934, 416, 467, 416, 467, 649 };

static const uint16_t powerCode1b[] = { 2691, 854, 467, 832, 467, 416, 467, 416, 1401, 1248, 467, 416, 467, 416, 467, 416, 467, 416, 934, 832, 467, 416, 934, 416, 467, 832, 467, 416, 467, 416, 934, 416, 467, 416, 467 };

static const uint16_t powerCode2[] = { 2440, 568, 1223, 565, 626, 565, 1223, 565, 626, 565, 1223, 565, 626, 565, 626, 565, 1223, 565, 626, 565, 626, 565, 626, 565, 626, 646 };

#define IR_I2C_ADDRESS	0x70
#define IR_STAGE_COUNT  64
#define IR_PACKET_SIZE(packet) (sizeof(packet->header) + sizeof(uint16_t) * packet->header.length)

#define END_DELAY_US	100000

typedef struct _IrPacketHeader {
	uint8_t		start;
    uint8_t     repeats;
    uint8_t     repeat_delay;
    uint8_t     length;
} IrPacketHeader;

typedef struct _IrPacket {
	IrPacketHeader	header;
    uint16_t    	timing[IR_STAGE_COUNT];
} IrPacket;

static void sendIrPacketAndWait(IrPacket* packet, uint32_t endDelayUs)
{
	uint32_t totalUs = 0;
	for (int i = 0; i < packet->header.length; i++) {
		totalUs += packet->timing[i];
	}
	totalUs += (totalUs + packet->header.repeat_delay) * (packet->header.repeats - 1);
	totalUs += endDelayUs;

	i2cSendBlock(IR_I2C_ADDRESS, &packet->header.start, IR_PACKET_SIZE(packet));

	uint32_t totalMs = (totalUs + 999) / 1000;
	sysTickDelayMs(totalMs);
}

void irTest()
{
	IrPacket irPacket;

	irPacket.header.start			= 1;
	irPacket.header.repeats			= 1;
	irPacket.header.repeat_delay	= 1;

	irPacket.header.length		 	= sizeof(powerCode1a) / sizeof(uint16_t);
	memcpy(irPacket.timing, powerCode1a, sizeof(powerCode1a));

	sendIrPacketAndWait(&irPacket, 74000);

	irPacket.header.length		 	= sizeof(powerCode1b) / sizeof(uint16_t);
	memcpy(irPacket.timing, powerCode1b, sizeof(powerCode1b));

	sendIrPacketAndWait(&irPacket, 74000);
}
