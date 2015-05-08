/*
 * spi.c
 *
 *  Created on: 7 May 2015
 *      Author: ntuckett
 */
#include "spi.h"

#include "MKL26Z4.h"
#include "ports.h"

// Pins to initialise
// PTD: 5 (SCLK)
//		6 (MOSI)
//		7 (MISO)

static const PortConfig portDPins =
{
	PORTD_BASE_PTR,
	~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
	PORT_PCR_MUX(2),
	3,
	{ 5, 6, 7 }
};

void spiInit()
{
	/* SIM_SCGC4: SPI1=1 */
	SIM_SCGC4 |= SIM_SCGC4_SPI1_MASK;
	/* SPI1_C1: SPIE=0,SPE=0,SPTIE=0,MSTR=0,CPOL=0,CPHA=1,SSOE=0,LSBFE=0 */
	SPI1_C1 = SPI_C1_CPHA_MASK;
	/* SPI1_C2: SPMIE=0,SPIMODE=0,TXDMAE=0,MODFEN=0,BIDIROE=0,RXDMAE=0,SPISWAI=0,SPC0=0 */
	SPI1_C2 = 0x00U;
	/* SPI1_BR: ??=0,SPPR=1,SPR=0 */
	SPI1_BR = (SPI_BR_SPPR(0x01) | SPI_BR_SPR(0x00));
	/* SPI1_MH: Bits=0 */
	SPI1_MH = SPI_MH_Bits(0x00);
	/* SPI1_ML: Bits=0 */
	SPI1_ML = SPI_ML_Bits(0x00);
	/* SPI1_C3: ??=0,??=0,TNEAREF_MARK=0,RNFULLF_MARK=0,INTCLR=0,TNEARIEN=0,RNFULLIEN=0,FIFOMODE=0 */
	SPI1_C3 = 0x00U;
	/* SPI1_C1: SPIE=0,SPE=1,SPTIE=0,MSTR=1,CPOL=0,CPHA=0,SSOE=0,LSBFE=0 */
	SPI1_C1 = (SPI_C1_SPE_MASK | SPI_C1_MSTR_MASK);

	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
	portInitialise(&portDPins);
}

void spiWrite(uint8_t byte)
{
	while (!(SPI_S_REG(SPI1) & SPI_S_SPTEF_MASK));
	SPI_DL_REG(SPI1) = byte;
	while ((SPI_S_REG(SPI1) & SPI_S_SPRF_MASK) == 0);
	SPI_DL_REG(SPI1);
}

uint8_t spiRead()
{
	while ((SPI_S_REG(SPI1) & SPI_S_SPTEF_MASK) == 0);
	SPI_DL_REG(SPI1) = 0;
	while ((SPI_S_REG(SPI1) & SPI_S_SPRF_MASK) == 0);
	return SPI_DL_REG(SPI1);
}

