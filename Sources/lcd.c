//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * lcd.c
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */
#include "lcd.h"

#include "MKL26Z4.h"
#include "systick.h"
#include "ports.h"

// Ports & pins to initialise:
//  All as GPIO outputs
//  PTB: 3		(power switch)
//	PTC: 0->7	(parallel data)
//  PTD: 3		(backlight)
//	PTE: 24		(chip select)
//       25		(write)
//		 30		(data/command)
//		 31		(reset)

static const PortConfig portBPins = {
    PORTB_BASE_PTR,
    ~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
    PORT_PCR_MUX(1),
    1,
    { 3 }
};

static const PortConfig portCPins = {
    PORTC_BASE_PTR,
    ~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
    PORT_PCR_MUX(1),
    8,
    { 0, 1, 2, 3, 4, 5, 6, 7 }
};

static const PortConfig portDPins = {
    PORTD_BASE_PTR,
    ~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
    PORT_PCR_MUX(1),
    1,
    { 3 }
};

static const PortConfig portEPins = {
    PORTE_BASE_PTR,
    ~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
    PORT_PCR_MUX(1),
    4,
    { 24, 25, 30, 31 }
};

static const PortConfig portCPinsOff = {
    PORTC_BASE_PTR,
    ~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
    PORT_PCR_MUX(0),
    8,
    { 0, 1, 2, 3, 4, 5, 6, 7 }
};

static const PortConfig portDPinsOff = {
    PORTD_BASE_PTR,
    ~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
    PORT_PCR_MUX(0),
    1,
    { 3 }
};

static const PortConfig portEPinsOff = {
    PORTE_BASE_PTR,
    ~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK),
    PORT_PCR_MUX(0),
    4,
    { 24, 25, 30, 31 }
};

static int backlightState = 0;
static int sleepState = 0;

#define TFT_PWR_MASK	(1 << 3)
#define TFT_DC_MASK 	0x40000000U
#define TFT_RS_MASK 	0x80000000U
#define TFT_BL_MASK 	0x00000008U
#define TFT_CS_MASK		0x01000000U
#define TFT_WR_MASK		0x02000000U

#define ILI9341_TFTWIDTH  240
#define ILI9341_TFTHEIGHT 320

#define ILI9341_NOP     0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID   0x04
#define ILI9341_RDDST   0x09

#define ILI9341_SLPIN   0x10
#define ILI9341_SLPOUT  0x11
#define ILI9341_PTLON   0x12
#define ILI9341_NORON   0x13

#define ILI9341_RDMODE  	0x0A
#define ILI9341_RDMADCTL  	0x0B
#define ILI9341_RDPIXFMT  	0x0C
#define ILI9341_RDIMGFMT  	0x0A
#define ILI9341_RDSELFDIAG  0x0F

#define ILI9341_INVOFF  	0x20
#define ILI9341_INVON   	0x21
#define ILI9341_GAMMASET 	0x26
#define ILI9341_DISPOFF 	0x28
#define ILI9341_DISPON  	0x29

#define ILI9341_CASET   0x2A
#define ILI9341_PASET   0x2B
#define ILI9341_RAMWR   0x2C
#define ILI9341_RAMRD   0x2E

#define ILI9341_PTLAR   0x30
#define ILI9341_MADCTL  0x36
#define ILI9341_PIXFMT  0x3A

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_FRMCTR2 0xB2
#define ILI9341_FRMCTR3 0xB3
#define ILI9341_INVCTR  0xB4
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1  0xC0
#define ILI9341_PWCTR2  0xC1
#define ILI9341_PWCTR3  0xC2
#define ILI9341_PWCTR4  0xC3
#define ILI9341_PWCTR5  0xC4
#define ILI9341_VMCTR1  0xC5
#define ILI9341_VMCTR2  0xC7

#define ILI9341_RDID1   0xDA
#define ILI9341_RDID2   0xDB
#define ILI9341_RDID3   0xDC
#define ILI9341_RDID4   0xDD

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1

void parallelWrite(unsigned char byte)
{
    FGPIO_PCOR_REG(FGPIOE) = TFT_CS_MASK;
    FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
    FGPIO_PDOR_REG(FGPIOC) = byte;
    FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;
    FGPIO_PSOR_REG(FGPIOE) = TFT_CS_MASK;
}

void tftWriteCmd(unsigned char byte)
{
    FGPIO_PCOR_REG(FGPIOE) = TFT_DC_MASK;
    FGPIO_PCOR_REG(FGPIOE) = TFT_CS_MASK;
    FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
    FGPIO_PDOR_REG(FGPIOC) = byte;
    FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;
    FGPIO_PSOR_REG(FGPIOE) = TFT_CS_MASK;
    FGPIO_PSOR_REG(FGPIOE) = TFT_DC_MASK;
}

void tftWriteData(unsigned char byte)
{
    parallelWrite(byte);
}

void tftReset()
{
    FGPIO_PCOR_REG(FGPIOE) = TFT_RS_MASK;	// LCD hardware reset
    FGPIO_PSOR_REG(FGPIOE) = TFT_RS_MASK;

    tftWriteCmd(0x01);	// SW reset
    sysTickDelayMs(5);
    tftWriteCmd(0x28);	// display off

    tftWriteCmd(0xEF);
    tftWriteData(0x03);
    tftWriteData(0x80);
    tftWriteData(0x02);

    tftWriteCmd(0xCF);
    tftWriteData(0x00);
    tftWriteData(0XC1);
    tftWriteData(0X30);

    tftWriteCmd(0xED);
    tftWriteData(0x64);
    tftWriteData(0x03);
    tftWriteData(0X12);
    tftWriteData(0X81);

    tftWriteCmd(0xE8);
    tftWriteData(0x85);
    tftWriteData(0x00);
    tftWriteData(0x78);

    tftWriteCmd(0xCB);
    tftWriteData(0x39);
    tftWriteData(0x2C);
    tftWriteData(0x00);
    tftWriteData(0x34);
    tftWriteData(0x02);

    tftWriteCmd(0xF7);
    tftWriteData(0x20);

    tftWriteCmd(0xEA);
    tftWriteData(0x00);
    tftWriteData(0x00);

    tftWriteCmd(ILI9341_PWCTR1);    //Power control
    tftWriteData(0x23);             //VRH[5:0]

    tftWriteCmd(ILI9341_PWCTR2);    //Power control
    tftWriteData(0x10);             //SAP[2:0];BT[3:0]

    tftWriteCmd(ILI9341_VMCTR1);    //VCM control
    tftWriteData(0x3e);
    tftWriteData(0x28);

    tftWriteCmd(ILI9341_VMCTR2);    //VCM control2
    tftWriteData(0x86);             //--

    tftWriteCmd(ILI9341_MADCTL);    // Memory Access Control
    tftWriteData(0x88);				// x -> x, y -> h - y

    tftWriteCmd(ILI9341_PIXFMT);
    tftWriteData(0x55);

    tftWriteCmd(ILI9341_FRMCTR1);
    tftWriteData(0x00);
    tftWriteData(0x18);

    tftWriteCmd(ILI9341_DFUNCTR);   // Display Function Control
    tftWriteData(0x08);
    tftWriteData(0x82);
    tftWriteData(0x27);

    tftWriteCmd(0xF2);              // 3Gamma Function Disable
    tftWriteData(0x00);

    tftWriteCmd(ILI9341_GAMMASET);  //Gamma curve selected
    tftWriteData(0x01);

    tftWriteCmd(ILI9341_GMCTRP1);   //Set Gamma
    tftWriteData(0x0F);
    tftWriteData(0x31);
    tftWriteData(0x2B);
    tftWriteData(0x0C);
    tftWriteData(0x0E);
    tftWriteData(0x08);
    tftWriteData(0x4E);
    tftWriteData(0xF1);
    tftWriteData(0x37);
    tftWriteData(0x07);
    tftWriteData(0x10);
    tftWriteData(0x03);
    tftWriteData(0x0E);
    tftWriteData(0x09);
    tftWriteData(0x00);

    tftWriteCmd(ILI9341_GMCTRN1);   //Set Gamma
    tftWriteData(0x00);
    tftWriteData(0x0E);
    tftWriteData(0x14);
    tftWriteData(0x03);
    tftWriteData(0x11);
    tftWriteData(0x07);
    tftWriteData(0x31);
    tftWriteData(0xC1);
    tftWriteData(0x48);
    tftWriteData(0x08);
    tftWriteData(0x0F);
    tftWriteData(0x0C);
    tftWriteData(0x31);
    tftWriteData(0x36);
    tftWriteData(0x0F);

    tftWriteCmd(0xF6);
    tftWriteData(0x01);
    tftWriteData(0x00);
    tftWriteData(0x20);

    tftWriteCmd(ILI9341_SLPOUT);    //Exit Sleep

    sysTickDelayMs(120);

    tftWriteCmd(ILI9341_DISPON);

    sleepState = 0;
}

struct TFTPixel
{
    union
    {
        struct
        {
            uint8_t lo;
            uint8_t hi;
        };
        uint16_t lohi;
    };
};

struct TFTPixel pixelBuffer[240];
struct TFTPixel pixelBuffer2[240];

void fillPixelBuffer(struct TFTPixel* buffer, uint16_t colour)
{
    for (int i = 0; i < 240; i++) {
        buffer[i].lohi = colour;
    }
}

void drawTestRect_PEInline_FGPIO(uint16_t colour)
{
    uint16_t x0 = 0;
    uint16_t y0 = 0;
    uint16_t x1 = 239;
    uint16_t y1 = 319;

    tftWriteCmd(ILI9341_CASET); // Column addr set
    tftWriteData(x0 >> 8);
    tftWriteData(x0 & 0xFF);     // XSTART
    tftWriteData(x1 >> 8);
    tftWriteData(x1 & 0xFF);     // XEND

    tftWriteCmd(ILI9341_PASET); // Row addr set
    tftWriteData(y0 >> 8);
    tftWriteData(y0);     // YSTART
    tftWriteData(y1 >> 8);
    tftWriteData(y1);     // YEND

    tftWriteCmd(ILI9341_RAMWR); // write to RAM

    uint32_t colour_hi = (colour & 0xff00) >> 8;
    uint32_t colour_lo = (colour & 0xff);

    FGPIO_PCOR_REG(FGPIOE) = TFT_CS_MASK;

    for (int i = 0; i < 76800; i++) {
        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        uint32_t tog_val = (FGPIO_PDOR_REG(FGPIOC) ^ colour_hi) & 0x00FFU;
        FGPIO_PTOR_REG(FGPIOC) = tog_val;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        tog_val = ((uint32_t) FGPIO_PDOR_REG(FGPIOC) ^ colour_lo) & 0x00FFU;
        FGPIO_PTOR_REG(FGPIOC) = tog_val;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;
    }

    FGPIO_PSOR_REG(FGPIOE) = TFT_CS_MASK;
}

void drawTestRect_PEInline_BME(uint16_t colour)
{
    uint16_t x0 = 0;
    uint16_t y0 = 0;
    uint16_t x1 = 239;
    uint16_t y1 = 319;

    tftWriteCmd(ILI9341_CASET); // Column addr set
    tftWriteData(x0 >> 8);
    tftWriteData(x0 & 0xFF);    // XSTART
    tftWriteData(x1 >> 8);
    tftWriteData(x1 & 0xFF);    // XEND

    tftWriteCmd(ILI9341_PASET); // Row addr set
    tftWriteData(y0 >> 8);
    tftWriteData(y0);           // YSTART
    tftWriteData(y1 >> 8);
    tftWriteData(y1);           // YEND

    tftWriteCmd(ILI9341_RAMWR); // write to RAM

    uint32_t colour_hi = (colour & 0xff00) >> 8;
    uint32_t colour_lo = (colour & 0xff);

    FGPIO_PCOR_REG(FGPIOE) = TFT_CS_MASK;

    for (int i = 0; i < 76800; i++) {
        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        ((((GPIO_MemMapPtr) 0x5238F080))->PDOR) = colour_hi;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        ((((GPIO_MemMapPtr) 0x5238F080))->PDOR) = colour_lo;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;
    }

    FGPIO_PSOR_REG(FGPIOE) = TFT_CS_MASK;
}

void drawTestRect_PEInline_WROnly(uint16_t colour)
{
    uint16_t x0 = 0;
    uint16_t y0 = 0;
    uint16_t x1 = 239;
    uint16_t y1 = 319;

    tftWriteCmd(ILI9341_CASET); // Column addr set
    tftWriteData(x0 >> 8);
    tftWriteData(x0 & 0xFF);    // XSTART
    tftWriteData(x1 >> 8);
    tftWriteData(x1 & 0xFF);    // XEND

    tftWriteCmd(ILI9341_PASET); // Row addr set
    tftWriteData(y0 >> 8);
    tftWriteData(y0);           // YSTART
    tftWriteData(y1 >> 8);
    tftWriteData(y1);           // YEND

    tftWriteCmd(ILI9341_RAMWR); // write to RAM

    uint32_t colour_hi = (colour & 0xff00) >> 8;
    uint32_t colour_lo = (colour & 0xff);

    ((((GPIO_MemMapPtr) 0x5238F080))->PDOR) = colour_lo;

    FGPIO_PCOR_REG(FGPIOE) = TFT_CS_MASK;

    for (int i = 0; i < 76800; i++) {
        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;
    }

    FGPIO_PSOR_REG(FGPIOE) = TFT_CS_MASK;
}

void drawTestRect_PEInline_SRAM(uint16_t colour)
{
    uint16_t x0 = 0;
    uint16_t y0 = 0;
    uint16_t x1 = 239;
    uint16_t y1 = 319;

    tftWriteCmd(ILI9341_CASET); // Column addr set
    tftWriteData(x0 >> 8);
    tftWriteData(x0 & 0xFF);    // XSTART
    tftWriteData(x1 >> 8);
    tftWriteData(x1 & 0xFF);    // XEND

    tftWriteCmd(ILI9341_PASET); // Row addr set
    tftWriteData(y0 >> 8);
    tftWriteData(y0);           // YSTART
    tftWriteData(y1 >> 8);
    tftWriteData(y1);           // YEND

    tftWriteCmd(ILI9341_RAMWR); // write to RAM

    FGPIO_PCOR_REG(FGPIOE) = TFT_CS_MASK;

    fillPixelBuffer(pixelBuffer, 0x1ff8);

    for (int y = 0; y < 320; y++) {
        uint8_t* pixel_ptr = &pixelBuffer[0].lo;

        for (int x = 0; x < 240; x++, pixel_ptr += 2) {
            FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
            uint32_t tog_val = (FGPIO_PDOR_REG(FGPIOC) ^ *pixel_ptr + 1) & 0x00FFU;
            FGPIO_PTOR_REG(FGPIOC) = tog_val;
            FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

            FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
            tog_val = (FGPIO_PDOR_REG(FGPIOC) ^ *(pixel_ptr)) & 0x00FFU;
            FGPIO_PTOR_REG(FGPIOC) = tog_val;
            FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;
        }
    }

    FGPIO_PSOR_REG(FGPIOE) = TFT_CS_MASK;
}

void drawTestRect_PEInline_SRAM_PDOR(uint16_t colour)
{
    uint16_t x0 = 0;
    uint16_t y0 = 0;
    uint16_t x1 = 239;
    uint16_t y1 = 319;

    tftWriteCmd(ILI9341_CASET); // Column addr set
    tftWriteData(x0 >> 8);
    tftWriteData(x0 & 0xFF);    // XSTART
    tftWriteData(x1 >> 8);
    tftWriteData(x1 & 0xFF);    // XEND

    tftWriteCmd(ILI9341_PASET); // Row addr set
    tftWriteData(y0 >> 8);
    tftWriteData(y0);           // YSTART
    tftWriteData(y1 >> 8);
    tftWriteData(y1);           // YEND

    tftWriteCmd(ILI9341_RAMWR); // write to RAM

    FGPIO_PCOR_REG(FGPIOE) = TFT_CS_MASK;

    for (int y = 0; y < 320; y++) {
        uint8_t* pixel_ptr = &pixelBuffer[0].lo;

        for (int x = 0; x < 240; x++, pixel_ptr += 2) {
            FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
            FGPIO_PDOR_REG(FGPIOC) = *(pixel_ptr + 1);
            FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

            FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
            FGPIO_PDOR_REG(FGPIOC) = *(pixel_ptr);
            FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;
        }
    }

    FGPIO_PSOR_REG(FGPIOE) = TFT_CS_MASK;
}

void drawTestRect_PEInline_SRAM_PDOR_BufferFill(uint16_t colour)
{
    uint16_t x0 = 0;
    uint16_t y0 = 0;
    uint16_t x1 = 239;
    uint16_t y1 = 319;

    tftWriteCmd(ILI9341_CASET); // Column addr set
    tftWriteData(x0 >> 8);
    tftWriteData(x0 & 0xFF);    // XSTART
    tftWriteData(x1 >> 8);
    tftWriteData(x1 & 0xFF);    // XEND

    tftWriteCmd(ILI9341_PASET); // Row addr set
    tftWriteData(y0 >> 8);
    tftWriteData(y0);           // YSTART
    tftWriteData(y1 >> 8);
    tftWriteData(y1);           // YEND

    tftWriteCmd(ILI9341_RAMWR); // write to RAM

    FGPIO_PCOR_REG(FGPIOE) = TFT_CS_MASK;

    for (int y = 0; y < 320; y++) {
        fillPixelBuffer(pixelBuffer, colour);
        colour ^= 0xffff;

        uint8_t* pixel_ptr = &pixelBuffer[0].lo;

        for (int x = 0; x < 240; x++, pixel_ptr += 2) {
            FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
            FGPIO_PDOR_REG(FGPIOC) = *(pixel_ptr + 1);
            FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

            FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
            FGPIO_PDOR_REG(FGPIOC) = *(pixel_ptr);
            FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;
        }
    }

    FGPIO_PSOR_REG(FGPIOE) = TFT_CS_MASK;
}

extern unsigned char image_raw[];
extern unsigned int image_raw_len;

void drawTestImage(uint16_t x0, uint16_t y0)
{
    uint16_t x1 = x0 + 119;
    uint16_t y1 = y0 + 159;

    tftWriteCmd(ILI9341_CASET); // Column addr set
    tftWriteData(x0 >> 8);
    tftWriteData(x0 & 0xFF);    // XSTART
    tftWriteData(x1 >> 8);
    tftWriteData(x1 & 0xFF);    // XEND

    tftWriteCmd(ILI9341_PASET); // Row addr set
    tftWriteData(y0 >> 8);
    tftWriteData(y0);           // YSTART
    tftWriteData(y1 >> 8);
    tftWriteData(y1);           // YEND

    tftWriteCmd(ILI9341_RAMWR); // write to RAM

    FGPIO_PCOR_REG(FGPIOE) = TFT_CS_MASK;

    unsigned char* pixel_ptr = image_raw;

    for (unsigned int i = 0; i < image_raw_len; i += 4, pixel_ptr += 4) {
        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        uint32_t tog_val = ((uint32_t) FGPIO_PDOR_REG(FGPIOC) ^ ((uint32_t) *(pixel_ptr))) & 0x00FFU;
        FGPIO_PTOR_REG(FGPIOC) = tog_val;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        tog_val = ((uint32_t) FGPIO_PDOR_REG(FGPIOC) ^ ((uint32_t) *(pixel_ptr + 1))) & 0x00FFU;
        FGPIO_PTOR_REG(FGPIOC) = tog_val;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        tog_val = ((uint32_t) FGPIO_PDOR_REG(FGPIOC) ^ ((uint32_t) *(pixel_ptr + 2))) & 0x00FFU;
        FGPIO_PTOR_REG(FGPIOC) = tog_val;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        tog_val = ((uint32_t) FGPIO_PDOR_REG(FGPIOC) ^ ((uint32_t) *(pixel_ptr + 3))) & 0x00FFU;
        FGPIO_PTOR_REG(FGPIOC) = tog_val;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;
    }

    FGPIO_PSOR_REG(FGPIOE) = TFT_CS_MASK;
}

volatile uint32_t tftDmaWriteBitMask = 0x2000;
volatile uint32_t tftDmaFlag = 0;

void DMA2_IRQHandler()
{
    tftDmaFlag = 1;
    TPM1_SC = (TPM1_SC & ~TPM_SC_CMOD(0x03));
    DMA_DSR_BCR0 |= DMA_DSR_BCR_DONE_MASK;
    DMA_DSR_BCR1 |= DMA_DSR_BCR_DONE_MASK;
    DMA_DSR_BCR2 |= DMA_DSR_BCR_DONE_MASK;
}

void tftInitDma()
{
    // Configure DMA 2 interrupt
    NVIC_SetPriority(DMA2_IRQn, 0);
    NVIC_EnableIRQ(DMA2_IRQn);

    // Turn on DMA and DMAMUX clocks
    SIM_SCGC7 |= SIM_SCGC7_DMA_MASK;
    SIM_SCGC6 |= SIM_SCGC6_DMAMUX_MASK;

    // Turn on TPM1 clock
    SIM_SCGC6 |= SIM_SCGC6_TPM1_MASK;

    // Reset DMAMUX
    DMAMUX0_CHCFG0 = DMAMUX_CHCFG_SOURCE(0x00);
    DMAMUX0_CHCFG1 = DMAMUX_CHCFG_SOURCE(0x00);
    DMAMUX0_CHCFG2 = DMAMUX_CHCFG_SOURCE(0x00);
    DMAMUX0_CHCFG3 = DMAMUX_CHCFG_SOURCE(0x00);

    // Configure DMA0 to drive write low then link to DMA1. Triggered by PIT0 or TPM1
    DMA_DCR0 = DMA_DCR_CS_MASK |
    DMA_DCR_ERQ_MASK |
    DMA_DCR_D_REQ_MASK |
    DMA_DCR_EADREQ_MASK | DMA_DCR_SSIZE(0x04) | DMA_DCR_DSIZE(0x04) | DMA_DCR_LINKCC(0x02) | DMA_DCR_LCH1(0x01);
    DMA_SAR0 = (uint32_t) &tftDmaWriteBitMask;
    DMA_DAR0 = (uint32_t) &((((GPIO_MemMapPtr) 0x400FF000u))->PCOR);
    DMAMUX0_CHCFG0 = DMAMUX_CHCFG_SOURCE(55) | DMAMUX_CHCFG_ENBL_MASK;

    // Configure DMA1 to load data into GPIO, then link to DMA2.
    DMA_DCR1 = DMA_DCR_CS_MASK |
    DMA_DCR_SINC_MASK |
    DMA_DCR_EADREQ_MASK | DMA_DCR_SSIZE(0x02) | DMA_DCR_DSIZE(0x02) | DMA_DCR_LINKCC(0x02) | DMA_DCR_LCH1(0x02);
    DMA_DAR1 = (uint32_t) &((((GPIO_MemMapPtr) 0x400FF080u))->PDOR);
    DMAMUX0_CHCFG1 = DMAMUX_CHCFG_SOURCE(61) | DMAMUX_CHCFG_ENBL_MASK;

    // Configure DMA2 to drive write high, fire interrupt when count zero
    DMA_DCR2 = DMA_DCR_CS_MASK |
    DMA_DCR_EINT_MASK |
    DMA_DCR_EADREQ_MASK | DMA_DCR_SSIZE(0x04) | DMA_DCR_DSIZE(0x04);
    DMA_SAR2 = (uint32_t) &tftDmaWriteBitMask;
    DMA_DAR2 = (uint32_t) &((((GPIO_MemMapPtr) 0x400FF000u))->PSOR);
    DMAMUX0_CHCFG2 = DMAMUX_CHCFG_SOURCE(62) | DMAMUX_CHCFG_ENBL_MASK;

    // Configure TPM1 to drive DMA on overflow
    TPM1_SC = 0;
    TPM1_C0SC = TPM_CnSC_CHF_MASK;
    TPM1_C1SC = TPM_CnSC_CHF_MASK;
    TPM1_CONF &= (uint32_t) ~(uint32_t) (
    TPM_CONF_CROT_MASK |
    TPM_CONF_CSOO_MASK |
    TPM_CONF_CSOT_MASK |
    TPM_CONF_GTBEEN_MASK | TPM_CONF_DBGMODE(0x03) |
    TPM_CONF_DOZEEN_MASK);
    TPM1_MOD = TPM_MOD_MOD(8);	// 4Mhz
    TPM1_SC = TPM_SC_DMA_MASK;

    tftDmaFlag = 1;
}

void tftTriggerDma(uint32_t source, uint32_t length)
{
    tftDmaFlag = 0;
    DMA_SAR1 = source;
    DMA_DSR_BCR0 = length * 4;
    DMA_DSR_BCR1 = length * 2;
    DMA_DSR_BCR2 = length * 4;
    DMA_DCR0 |= DMA_DCR_ERQ_MASK;

    TPM1_CNT &= (uint32_t) ~(uint32_t) (TPM_CNT_COUNT(0xFFFF));
    TPM1_SC = (TPM1_SC & ~TPM_SC_CMOD(0x03)) | TPM_SC_CMOD(0x01);
}

void drawTestRectDma()
{
    uint16_t x0 = 0;
    uint16_t y0 = 0;
    uint16_t x1 = 239;
    uint16_t y1 = 319;

    tftWriteCmd(ILI9341_CASET); // Column addr set
    tftWriteData(x0 >> 8);
    tftWriteData(x0 & 0xFF);    // XSTART
    tftWriteData(x1 >> 8);
    tftWriteData(x1 & 0xFF);    // XEND

    tftWriteCmd(ILI9341_PASET); // Row addr set
    tftWriteData(y0 >> 8);
    tftWriteData(y0);           // YSTART
    tftWriteData(y1 >> 8);
    tftWriteData(y1);           // YEND

    tftWriteCmd(ILI9341_RAMWR); // write to RAM

    FGPIO_PCOR_REG(FGPIOE) = TFT_CS_MASK;

    fillPixelBuffer(pixelBuffer, 0x1ff8);

    for (unsigned int y = 0; y < 160; y++) {
        tftTriggerDma((uint32_t) pixelBuffer, sizeof(pixelBuffer) / sizeof(pixelBuffer[0]) * 2);
        fillPixelBuffer(pixelBuffer2, 0xf81f);
        while (!tftDmaFlag) {
            __asm("wfi");
        }
        tftTriggerDma((uint32_t) pixelBuffer2, sizeof(pixelBuffer2) / sizeof(pixelBuffer2[0]) * 2);
        fillPixelBuffer(pixelBuffer, 0x1ff8);
        while (!tftDmaFlag) {
            __asm("wfi");
        }
    }

    FGPIO_PSOR_REG(FGPIOE) = TFT_CS_MASK;
}

void tftSetupPorts()
{
    portInitialise(&portBPins);
    portInitialise(&portCPins);
    portInitialise(&portDPins);
    portInitialise(&portEPins);

    FGPIOB_PDDR |= TFT_PWR_MASK;
    FGPIOB_PSOR = TFT_PWR_MASK;
    FGPIOC_PDDR |= 0xffU;
    FGPIOD_PDDR |= TFT_BL_MASK;
    FGPIOE_PDDR |= TFT_CS_MASK | TFT_WR_MASK | TFT_RS_MASK | TFT_DC_MASK;

    FGPIOE_PSOR = TFT_CS_MASK | TFT_WR_MASK | TFT_RS_MASK | TFT_DC_MASK;
    FGPIOD_PCOR = TFT_BL_MASK;
    backlightState = 0;
}

void tftInit()
{
    SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;
    tftSetupPorts();
    sysTickDelayMs(5);
    tftReset();
    //tftInitDma();
}

void tftSetBacklight(int status)
{
    if (sleepState) {
        return;
    }

    if (status) {
        FGPIO_PSOR_REG(FGPIOD) = TFT_BL_MASK;
    } else {
        FGPIO_PCOR_REG(FGPIOD) = TFT_BL_MASK;
    }

    backlightState = status;
}

int tftGetBacklight()
{
    return backlightState;
}

void tftPowerOff()
{
    FGPIOB_PCOR = TFT_PWR_MASK;
    FGPIOC_PCOR = 0xff;
    FGPIOD_PCOR = TFT_BL_MASK;
    FGPIOE_PCOR = TFT_CS_MASK | TFT_WR_MASK | TFT_RS_MASK | TFT_DC_MASK;

    portInitialise(&portCPinsOff);
    portInitialise(&portDPinsOff);
    portInitialise(&portEPinsOff);
}

void tftPowerOn()
{
    tftSetupPorts();
    sysTickDelayMs(5);
    tftReset();
}

void tftStartBlit(int x, int y, int width, int height)
{
    int x1 = x + width - 1;
    int y1 = y + height - 1;

    tftWriteCmd(ILI9341_CASET); // Column addr set
    tftWriteData(x >> 8);
    tftWriteData(x & 0xFF);     // XSTART
    tftWriteData(x1 >> 8);
    tftWriteData(x1 & 0xFF);    // XEND

    tftWriteCmd(ILI9341_PASET); // Row addr set
    tftWriteData(y >> 8);
    tftWriteData(y);            // YSTART
    tftWriteData(y1 >> 8);
    tftWriteData(y1);           // YEND

    tftWriteCmd(ILI9341_RAMWR); // write to RAM
    FGPIO_PCOR_REG(FGPIOE) = TFT_CS_MASK;
}

void tftBlit(uint16_t* buffer, size_t pixels)
{
    uint8_t* pixel_ptr = (uint8_t*) buffer;

    while (pixels >= 8) {
        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        ;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        ;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        ;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        ;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        ;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        ;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        ;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        ;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        pixels -= 8;
    }

    while (pixels >= 4) {
        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        ;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        ;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        ;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        ;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        pixels -= 4;
    }

    while (pixels >= 2) {
        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        ;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        ;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        pixels -= 2;
    }

    while (pixels--) {
        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PDOR_REG(FGPIOC) = ((uint32_t) *(pixel_ptr++)) & 0x00ffU;
        ;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;
    }
}

void tftClear(size_t pixels)
{
    FGPIO_PDOR_REG(FGPIOC) = 0;
    for (size_t x = 0; x < pixels; x++) {
        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;

        FGPIO_PCOR_REG(FGPIOE) = TFT_WR_MASK;
        FGPIO_PSOR_REG(FGPIOE) = TFT_WR_MASK;
    }
}

void tftEndBlit()
{
    FGPIO_PSOR_REG(FGPIOE) = TFT_CS_MASK;
    FGPIO_PDOR_REG(FGPIOC) = 0;
}

void tftSleep()
{
    tftWriteCmd(ILI9341_SLPIN);
    sysTickDelayMs(120);
    sleepState = 1;
}

void tftWake()
{
    tftWriteCmd(ILI9341_SLPOUT);
    sysTickDelayMs(120);
    sleepState = 0;
}
