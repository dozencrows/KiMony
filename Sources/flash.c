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
#include "ports.h"
#include "uart.h"
#include "codeutil.h"
#include "renderer.h"
#include "fontdata.h"

#define CPU_FLASH_SECTOR_SIZE 0x400

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

void spiFlashInit()
{
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;

	portInitialise(&portBPins);

	FGPIOB_PDDR |= (1 << 18);
	FGPIOB_PSOR = (1 << 18);
}

void spiFlashTest()
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

void RAM_FUNCTION cpuFlashCommand()
{
	// Trigger command, and wait for completion
	__disable_irq();
	FTFA->FSTAT = 0x80;
	while(((FTFA->FSTAT)&(1UL << 7))==0x00);
	__enable_irq();
}

void cpuFlashEraseSector(uint8_t* sector)
{
	uint8_t addr1, addr2, addr3;

	addr1 = ((int)sector & 0xff0000) >> 16;
	addr2 = ((int)sector & 0x00ff00) >> 8;
	addr3 = ((int)sector & 0x0000ff);

	// Ensure flash no command running
	while(((FTFA->FSTAT)&(1UL << 7))==0x00);

	FTFA->FCCOB0	= 0x09;
	FTFA->FCCOB1	= addr1;
	FTFA->FCCOB2	= addr2;
	FTFA->FCCOB3	= addr3;

	cpuFlashCommand();
}

uint8_t* cpuFlashCopyLongWord(uint8_t* src, uint8_t* dst)
{
	uint8_t addr1, addr2, addr3;

	addr1 = ((int)dst & 0xff0000) >> 16;
	addr2 = ((int)dst & 0x00ff00) >> 8;
	addr3 = ((int)dst & 0x0000ff);

	// Ensure flash no command running
	while(((FTFA->FSTAT)&(1UL << 7))==0x00);

	// Clear error bits
	if(!((FTFA->FSTAT)==0x80)) {
		FTFA->FSTAT = 0x30;
	}

	FTFA->FCCOB0	= 0x06;
	FTFA->FCCOB1	= addr1;
	FTFA->FCCOB2	= addr2;
	FTFA->FCCOB3	= addr3;
	FTFA->FCCOB7	= *src++;
	FTFA->FCCOB6	= *src++;
	FTFA->FCCOB5	= *src++;
	FTFA->FCCOB4	= *src++;

	cpuFlashCommand();

	return src;
}

static void cpuFlashCopy(uint8_t* src, uint8_t* dst, size_t count)
{
	count = (count + 3) & 0xfffffffc;
	while (count > 0) {
		src = cpuFlashCopyLongWord(src, dst);
		dst += 4;
		count -= 4;
	}
}

static void renderMessage(const char* message, uint16_t colour)
{
	rendererClearScreen();
	uint16_t width, height, x, y;

	rendererGetStringBounds(message, &KiMony, &width, &height);
	x = (SCREEN_WIDTH - width) / 2;
	y = (SCREEN_HEIGHT - height) / 2;

	rendererNewDrawList();
	rendererDrawRect(x, y, width, height, 0);
	rendererDrawString(message, x, y, &KiMony, colour);
	rendererRenderDrawList();
}

void cpuFlashDownload()
{
	PORTE_PCR22 = (uint32_t)((PORTE_PCR22 & (uint32_t)~(uint32_t)(
				 PORT_PCR_ISF_MASK |
				 PORT_PCR_MUX(0x07)
				)) | (uint32_t)(
				 PORT_PCR_MUX(0x04)
				));
	PORTE_PCR23 = (uint32_t)((PORTE_PCR23 & (uint32_t)~(uint32_t)(
				 PORT_PCR_ISF_MASK |
				 PORT_PCR_MUX(0x07)
				)) | (uint32_t)(
				 PORT_PCR_MUX(0x04)
				));

	uartInit(2, DEFAULT_SYSTEM_CLOCK / 2, 115200);

	int downloadComplete = 0;


	renderMessage("Waiting for data...", 0xffff);
	while (!downloadComplete) {
		// Wait for transfer initiation:
		uint8_t uartCh = uartGetchar(2);

		switch (uartCh) {
			case 0x10: {
				// Number of longwords
				size_t transferSize = uartGetchar(2) | (uartGetchar(2) << 8);

				renderMessage("Downloading...", 0xffff);

				int sectors = (__FlashStoreLimit - __FlashStoreBase) / CPU_FLASH_SECTOR_SIZE;
				uint8_t* sector = __FlashStoreBase;

				while (sectors--) {
					cpuFlashEraseSector(sector);
					sector += CPU_FLASH_SECTOR_SIZE;
				}

				// Indicate readiness for data
				uartPutchar(2, 0x10);

				uint8_t 	flashData[4];
				uint8_t*	flashStore = __FlashStoreBase;

				while (transferSize-- > 0) {
					flashData[0] = uartGetchar(2);
					flashData[1] = uartGetchar(2);
					flashData[2] = uartGetchar(2);
					flashData[3] = uartGetchar(2);

					cpuFlashCopyLongWord(flashData, flashStore);
					flashStore += 4;
				}

				// Indicate transfer end
				uartPutchar(2, 0x10);
				break;
			}

			case 0x20: {
				// Number of bytes
				size_t dataSize = (uartGetchar(2) | (uartGetchar(2) << 8)) * 4;

				uint8_t 	flashData;
				uint8_t*	flashStore = __FlashStoreBase;
				int 		errors = 0;

				while (dataSize-- > 0) {
					flashData = uartGetchar(2);

					if (flashData != *flashStore++) {
						errors++;
					}
				}

				if (errors == 0) {
					downloadComplete = 1;
				}
				else {
					renderMessage("ERRORS!", 0xf800);
				}
				break;
			}
		}
	}

	PORTE_PCR22 = (uint32_t)((PORTE_PCR22 & (uint32_t)~(uint32_t)(
				 PORT_PCR_ISF_MASK |
				 PORT_PCR_MUX(0x07)
				)) | (uint32_t)(
				 PORT_PCR_MUX(0x01)
				));
	PORTE_PCR23 = (uint32_t)((PORTE_PCR23 & (uint32_t)~(uint32_t)(
				 PORT_PCR_ISF_MASK |
				 PORT_PCR_MUX(0x07)
				)) | (uint32_t)(
				 PORT_PCR_MUX(0x01)
				));
}
