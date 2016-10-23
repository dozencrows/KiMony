#ifdef _DEBUG

#include <stdint.h>
#include <string.h>
#include "ports.h"
#include "debugutils.h"
#include "renderer.h"
#include "fontdata.h"
#include "mathutil.h"

#define DEBUG_LED_PIN			20
#define DEBUG_LED_MASK			(1 << DEBUG_LED_PIN)

#define DEBUG_OVERLAY_COUNT		4
#define DEBUG_OVERLAY_TEXT_LEN	16
#define DEBUG_OVERLAY_HEIGHT	18
#define DEBUG_OVERLAY_BASE_X	0
#define DEBUG_OVERLAY_BASE_Y	(SCREEN_HEIGHT - (DEBUG_OVERLAY_COUNT * DEBUG_OVERLAY_HEIGHT))

#define DEBUG_OVERLAY_FLAG_RENDER	0x01
#define DEBUG_OVERLAY_FLAG_CLEAR	0x02

static const PortConfig portEPins = {
    PORTE_BASE_PTR,
    ~(PORT_PCR_ISF_MASK | PORT_PCR_MUX_MASK), PORT_PCR_MUX(1),
    1,
    { DEBUG_LED_PIN }
};

typedef struct _DebugOverlay
{
    uint8_t flags;
    char text[DEBUG_OVERLAY_TEXT_LEN + 1];
    uint16_t width;
    uint16_t height;
    uint16_t clear_width;
    uint16_t clear_height;
} DebugOverlay;

static DebugOverlay overlays[DEBUG_OVERLAY_COUNT];

static const char hexdigit[] = "0123456789abcdef";

static char* debugHex8ToAscii(uint8_t v, char* buffer)
{
    *buffer++ = hexdigit[(v >> 4) & 0xf];
    *buffer++ = hexdigit[v & 0xf];
    return buffer;
}

static char* debugHex32ToAscii(unsigned int v, char* buffer)
{
    for (int i = 28; i >= 0; i -= 4) {
        *buffer++ = hexdigit[(v >> i) & 0xf];
    }
    return buffer;
}

static char* debugHex64ToAscii(uint64_t v, char* buffer)
{
    for (int i = 60; i >= 0; i -= 4) {
        *buffer++ = hexdigit[(v >> i) & 0xf];
    }
    return buffer;
}

void debugUtilsInit()
{
    memset(overlays, 0, sizeof(overlays));

    SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;

    portInitialise(&portEPins);
    portAsGPIOOutput(&portEPins);
    debugLEDOff();
}

void debugLEDOn()
{
    FGPIO_PSOR_REG(FGPIOE) = DEBUG_LED_MASK;
}

void debugLEDOff()
{
    FGPIO_PCOR_REG(FGPIOE) = DEBUG_LED_MASK;
}

void debugClearOverlay(int idx)
{
    if (idx >= 0 && idx < DEBUG_OVERLAY_COUNT) {
        overlays[idx].flags = DEBUG_OVERLAY_FLAG_CLEAR;

        overlays[idx].clear_width = SCREEN_WIDTH;
        overlays[idx].clear_height = overlays[idx].height;
        overlays[idx].text[0] = '\0';
        overlays[idx].width = 0;
        overlays[idx].height = 0;
    }
}

void debugSetOverlayText(int idx, const char* text)
{
    if (idx >= 0 && idx < DEBUG_OVERLAY_COUNT) {
        overlays[idx].flags = DEBUG_OVERLAY_FLAG_RENDER | DEBUG_OVERLAY_FLAG_CLEAR;

        overlays[idx].clear_width = overlays[idx].width;
        overlays[idx].clear_height = overlays[idx].height;

        strcpy(overlays[idx].text, text);
        rendererGetStringBounds(text, &KiMony, &overlays[idx].width, &overlays[idx].height);
    }
}

void debugSetOverlayHex(int idx, uint32_t value)
{
    char debugHex[12];
    debugHex32ToAscii(value, debugHex)[0] = '\0';
    debugSetOverlayText(idx, debugHex);
}

void debugRenderOverlays()
{
    rendererNewDrawList();
    for (int i = 0; i < DEBUG_OVERLAY_COUNT; i++) {
        int x = DEBUG_OVERLAY_BASE_X;
        int y = DEBUG_OVERLAY_BASE_Y + i * DEBUG_OVERLAY_HEIGHT;
        uint16_t clear_width = 0;
        uint16_t clear_height = 0;

        if (overlays[i].flags & DEBUG_OVERLAY_FLAG_RENDER) {
            clear_width = overlays[i].width;
            clear_height = overlays[i].height;
        }

        if (overlays[i].flags & DEBUG_OVERLAY_FLAG_CLEAR) {
            clear_width = MAX(clear_width, overlays[i].clear_width);
            clear_height = MAX(clear_height, overlays[i].clear_height);
        }

        if (overlays[i].flags && clear_width > 0 && clear_height > 0) {
            rendererDrawRect(x, y, clear_width, clear_height, 0x0000);
        }

        if (overlays[i].flags & DEBUG_OVERLAY_FLAG_RENDER) {
            rendererDrawString(overlays[i].text, x, y, &KiMony, 0xffff);
        }

        overlays[i].flags &= ~(DEBUG_OVERLAY_FLAG_RENDER | DEBUG_OVERLAY_FLAG_CLEAR);
    }
    rendererRenderDrawList();
}

#endif // #ifdef _DEBUG
