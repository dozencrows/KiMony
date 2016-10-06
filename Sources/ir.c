//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * ir.c
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */

#include "ir.h"

#include <string.h>
#include <stdio.h>

#include "i2c.h"
#include "systick.h"

#define IR_I2C_ADDRESS	0x71
#define IR_STAGE_COUNT  64
#define IR_PACKET_SIZE(packet) (sizeof(packet->header) + sizeof(uint16_t) * packet->header.length)
#define IR_MAX_DURATION	65535

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

#define IRENCODER_STATE_UNDEFINED		0
#define IRENCODER_STATE_ON				1
#define IRENCODER_STATE_OFF				2

#define IRENCODER_RESULT_OK					0
#define IRENCODER_RESULT_BUFFER_OVERFLOW	1
#define IRENCODER_RESULT_TIMER_OVERFLOW		2

typedef struct _IrEncoder {
	int			state;
	uint16_t	counter;
	IrPacket*	packet;
	uint32_t	duration;
} IrEncoder;

static void irEncoderBegin(IrEncoder* encoder, IrPacket* packet)
{
	encoder->state 					= IRENCODER_STATE_UNDEFINED;
	encoder->counter				= 0;
	encoder->duration				= 0;
	encoder->packet					= packet;
	encoder->packet->header.length	= 0;
}

static int irEncoderRecordTiming(IrEncoder* encoder)
{
	uint8_t packetLength = encoder->packet->header.length;
	if (packetLength == IR_STAGE_COUNT) {
		return IRENCODER_RESULT_BUFFER_OVERFLOW;
	}

	encoder->packet->timing[packetLength++] = encoder->counter;
	encoder->counter = 0;
	encoder->packet->header.length = packetLength;

	return IRENCODER_RESULT_OK;
}

static int irEncoderMark(IrEncoder* encoder, uint16_t duration)
{
	int result = IRENCODER_RESULT_OK;

	if (encoder->state == IRENCODER_STATE_OFF) {
		 result = irEncoderRecordTiming(encoder);
		 if (result != IRENCODER_RESULT_OK) {
			 return result;
		 }
	}

	encoder->state = IRENCODER_STATE_ON;
	if ((uint32_t)encoder->counter + (uint32_t)duration > IR_MAX_DURATION) {
		result = IRENCODER_RESULT_TIMER_OVERFLOW;
	}
	else {
		encoder->counter += duration;
		encoder->duration += duration;
	}
	return result;
}

static int irEncoderSpace(IrEncoder* encoder, uint16_t duration)
{
	int result = IRENCODER_RESULT_OK;

	if (encoder->state == IRENCODER_STATE_ON) {
		 result = irEncoderRecordTiming(encoder);
		 if (result != IRENCODER_RESULT_OK) {
			 return result;
		 }
	}

	encoder->state = IRENCODER_STATE_OFF;
	if ((uint32_t)encoder->counter + (uint32_t)duration > IR_MAX_DURATION) {
		result = IRENCODER_RESULT_TIMER_OVERFLOW;
	}
	else {
		encoder->counter += duration;
		encoder->duration += duration;
	}
	return result;
}

static int irEncoderEnd(IrEncoder* encoder, uint16_t duration)
{
	int result = irEncoderSpace(encoder, duration);
	if (result == IRENCODER_RESULT_OK) {
		result = irEncoderRecordTiming(encoder);
	}
	encoder->state = IRENCODER_STATE_UNDEFINED;
	return result;
}

#define RC6_HDR_MARK	2666
#define RC6_HDR_SPACE	889
#define RC6_T1			444
#define RC6_RPT_LENGTH	46000
#define RC6_END_QUIET	2666
#define TOP_BIT			0x80000000

static int irEncodeRC6(IrPacket* packet, uint32_t data, int bitCount)
{
	int result = IRENCODER_RESULT_OK;
	IrEncoder encoder;

	packet->header.start		= 1;
	packet->header.repeats		= 1;
	packet->header.repeat_delay	= 1;
	irEncoderBegin(&encoder, packet);

	irEncoderMark(&encoder, RC6_HDR_MARK);
	irEncoderSpace(&encoder, RC6_HDR_SPACE);

	data <<= 32 - bitCount;

	for (int i = 0; i < bitCount && !result; i++, data <<= 1) {
		uint16_t t = RC6_T1;
		if (i == 4) {
			t += t;
		}

		if (data & TOP_BIT) {
			result = !result ? irEncoderSpace(&encoder, t) : result;
			result = !result ? irEncoderMark(&encoder, t) : result;
		}
		else {
			result = !result ? irEncoderMark(&encoder, t) : result;
			result = !result ? irEncoderSpace(&encoder, t) : result;
		}
	}

	result = !result ? irEncoderEnd(&encoder, RC6_END_QUIET) : result;
	return result;
}

#define SIRC_HDR_MARK	2440
#define SIRC_HDR_SPACE	568
#define SIRC_ONE_MARK	1223
#define SIRC_ONE_SPACE	565
#define SIRC_ZERO_MARK	626
#define SIRC_ZERO_SPACE	565
#define SIRC_GAP_MS		45
#define SIRC_REPEATS	3
#define SIRC_REPEATS_20	6

static int irEncodeSIRC(IrPacket* packet, uint32_t data, int bitCount, int repeats)
{
	int result = IRENCODER_RESULT_OK;
	IrEncoder encoder;

	packet->header.start		= 1;
	packet->header.repeats		= repeats;
	packet->header.repeat_delay	= SIRC_GAP_MS;
	irEncoderBegin(&encoder, packet);

	irEncoderMark(&encoder, SIRC_HDR_MARK);
	irEncoderSpace(&encoder, SIRC_HDR_SPACE);

	data <<= 32 - bitCount;

	for (int i = 0; i < bitCount && !result; i++, data <<= 1) {
		if (data & TOP_BIT) {
			result = !result ? irEncoderMark(&encoder, SIRC_ONE_MARK) : result;
			result = !result ? irEncoderSpace(&encoder, SIRC_ONE_SPACE) : result;
		}
		else {
			result = !result ? irEncoderMark(&encoder, SIRC_ZERO_MARK) : result;
			result = !result ? irEncoderSpace(&encoder, SIRC_ZERO_SPACE) : result;
		}
	}

	result = !result ? irEncoderEnd(&encoder, 0) : result;

	uint32_t durationMs = (encoder.duration + 999) / 1000;

	if (durationMs < packet->header.repeat_delay) {
		packet->header.repeat_delay -= durationMs;
	}
	else {
		packet->header.repeat_delay = 10;
	}

	return result;
}

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

static IrPacket irPacket;

static void irSendRC6Code(uint32_t data, int bitCount)
{
	irEncodeRC6(&irPacket, data, bitCount);
	sendIrPacketAndWait(&irPacket, 74000);
}

static void irSendSIRCCode(uint32_t data, int bitCount)
{
	irEncodeSIRC(&irPacket, data, bitCount, SIRC_REPEATS);
	sendIrPacketAndWait(&irPacket, 45000);
}

void irDoAction(const IrAction* action, uint8_t* toggleFlag)
{
	const IrCode* code = action->codes;

	for (int i = 0; i < action->codeCount; i++, code++) {
		unsigned int completedCode = code->code;

		if (code->toggleMask) {
			*toggleFlag = !*toggleFlag;

			if (*toggleFlag) {
				completedCode |= code->toggleMask;
			}
		}

		switch (code->encoding) {
			case IRCODE_NOP: {
				sysTickDelayMs(code->code);
				break;
			}
			case IRCODE_RC6: {
				irSendRC6Code(completedCode, code->bits);
				break;
			}
			case IRCODE_SIRC: {
				irSendSIRCCode(completedCode, code->bits);
				break;
			}
		}
	}
}
