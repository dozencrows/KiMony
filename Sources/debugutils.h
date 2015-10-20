#ifndef DEBUGUTILS_H_
#define DEBUGUTILS_H_
#ifdef _DEBUG
#include <stdint.h>

extern void debugRenderInit();
extern void debugClearOverlay(int overlayIdx);
extern void debugSetOverlayText(int overlayIdx, const char* text);
extern void debugSetOverlayHex(int overlayIdx, uint32_t value);
extern void debugRenderOverlays();
#else

#define debugRenderInit()
#define debugClearOverlay(a)
#define debugSetOverlayText(a, b)
#define debugSetOverlayHex(a, b)
#define debugRenderOverlays()

#endif

#endif
