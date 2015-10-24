#ifndef DEBUGUTILS_H_
#define DEBUGUTILS_H_
#ifdef _DEBUG
#include <stdint.h>

extern void debugUtilsInit();
extern void debugClearOverlay(int overlayIdx);
extern void debugSetOverlayText(int overlayIdx, const char* text);
extern void debugSetOverlayHex(int overlayIdx, uint32_t value);
extern void debugRenderOverlays();
extern void debugLEDOn();
extern void debugLEDOff();

#else

#define debugUtilsInit()
#define debugClearOverlay(a)
#define debugSetOverlayText(a, b)
#define debugSetOverlayHex(a, b)
#define debugRenderOverlays()
#define debugLEDOn()
#define debugLEDOff()

#endif

#endif
