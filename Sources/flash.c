/*
 * spiflash.c
 *
 *  Created on: 1 May 2015
 *      Author: ntuckett
 */
#include "flash.h"
#include <stdio.h>
#include "spi.h"
#include "MKL26Z4.h"
#include "port_util.h"

// Pins to initialise
// PTB: 18 (Chip Select)

static const PortConfig portBPins =
{
	PORTB_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(1),
	1,
	{ 18 }
};

#define CMD_PAGEPROG     0x02
#define CMD_READDATA     0x03
#define CMD_WRITEDISABLE 0x04
#define CMD_READSTAT1    0x05
#define CMD_WRITEENABLE  0x06
#define CMD_SECTORERASE  0x20
#define CMD_CHIPERASE    0x60
#define CMD_ID           0x90

#define STAT_BUSY        0x01
#define STAT_WRTEN       0x02

int spiFlashWaitForReady(unsigned int timeout)
{
	unsigned char status;

	FGPIOB_PCOR = (1 << 18);
	do {
		spiWrite(CMD_READSTAT1);
		status = spiRead();;
		//printf("WFR: %02x\n", buffer[1]);
	} while(status & STAT_BUSY);
	FGPIOB_PSOR = (1 << 18);

	return 1;
}

int spiFlashWriteEnable()
{
	FGPIOB_PCOR = (1 << 18);
	spiWrite(CMD_WRITEENABLE);
	FGPIOB_PSOR = (1 << 18);
	FGPIOB_PCOR = (1 << 18);
	spiWrite(CMD_READSTAT1);
	unsigned char status = spiRead();
	FGPIOB_PSOR = (1 << 18);
	//printf("WEN: %02x\n", buffer[1]);
	return status & STAT_WRTEN ? 1 : 0;
}

int spiFlashWriteDisable()
{
	FGPIOB_PCOR = (1 << 18);
	spiWrite(CMD_WRITEDISABLE);
	FGPIOB_PSOR = (1 << 18);
	FGPIOB_PCOR = (1 << 18);
	spiWrite(CMD_READSTAT1);
	unsigned char status = spiRead();
	FGPIOB_PSOR = (1 << 18);
	//printf("WDS: %02x\n", buffer[1]);
	return status & STAT_WRTEN ? 0 : 1;
}

int spiFlashEraseSector(unsigned int addr)
{
	if (!spiFlashWaitForReady(0) || !spiFlashWriteEnable()) {
		return 0;
	}

	FGPIOB_PCOR = (1 << 18);
	spiWrite(CMD_SECTORERASE);
	spiWrite(addr >> 16);
	spiWrite(addr >> 8);
	spiWrite(0);
	FGPIOB_PSOR = (1 << 18);

	return spiFlashWaitForReady(1000);
}

int spiFlashWritePage(unsigned int addr, unsigned char* data)
{
	if (!spiFlashWaitForReady(0) || !spiFlashWriteEnable()) {
	  return 0;
	}

	FGPIOB_PCOR = (1 << 18);
	spiWrite(CMD_PAGEPROG);
	spiWrite(addr >> 16);
	spiWrite(addr >> 8);
	spiWrite(0);

	for (int i = 0; i < 256; i++) {
		spiWrite(data[i]);
	}
	FGPIOB_PSOR = (1 << 18);

	return spiFlashWaitForReady(0);
}

int spiFlashReadPage(unsigned int addr, unsigned char* data)
{
	unsigned char buffer[260];

	if (!spiFlashWaitForReady(0)) {
		return 0;
	}

	FGPIOB_PCOR = (1 << 18);
	spiWrite(CMD_READDATA);
	spiWrite(addr >> 16);
	spiWrite(addr >> 8);
	spiWrite(0);

	for (int i = 0; i < 256; i++) {
		data[i] = spiRead();
	}
	FGPIOB_PSOR = (1 << 18);

	return 1;
}

void flashInit()
{
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;

	portInitialise(&portBPins);

	FGPIOB_PDDR |= (1 << 18);
	FGPIOB_PSOR = (1 << 18);
}

void flashTest()
{
	unsigned char id[5];
	FGPIOB_PCOR = (1 << 18);
	spiWrite(CMD_ID);
	id[0] = spiRead();
	id[1] = spiRead();
	id[2] = spiRead();
	id[3] = spiRead();
	id[4] = spiRead();
	FGPIOB_PSOR = (1 << 18);

	printf("SPIFlash ID: %02x %02x %02x %02x %02x\n", id[0], id[1], id[2], id[3], id[4]);

	if (id[3] == 0xef && id[4] == 0x13) {
		printf("Performing memory write/read test...\n");
		printf("Erasing...\n");
		if (spiFlashEraseSector(0)) {
			char data[256];
			for(int i = 0; i < 256; i++) {
				data[i] = i;
			}
			printf("Writing...\n");
			if (spiFlashWritePage(0, data)) {
				printf("Reading...\n");
				char data2[256];
				if (spiFlashReadPage(0, data2)) {
					for(int i = 0; i < 256; i++) {
						if (data[i] != data2[i]) {
							printf("Bad byte %d: %02x %02x\n", i, data[i], data2[i]);
						}
					}
					printf("Read ok\n");
				}
				else {
					printf("Error!\n");
				}
			}
			else {
				printf("Error!\n");
			}
		}
		else {
			printf("Error!\n");
		}
	}

}
