#ifndef DEBUGUTILS_H_
#define DEBUGUTILS_H_
#include <stdint.h>

extern void debugRenderInit();
extern void debugClearOverlay(int overlayIdx);
extern void debugSetOverlayText(int overlayIdx, const char* text);
extern void debugRenderOverlays();

extern char* debugHex8ToAscii(uint8_t v, char* buffer);
extern char* debugHex32ToAscii(unsigned int v, char* buffer);
extern char* debugHex64ToAscii(uint64_t v, char* buffer);

#endif
