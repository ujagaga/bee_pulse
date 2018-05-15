#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>
#include <stdbool.h>

#include "config.h"

extern void TFTLCD_begin();
extern void TFTLCD_reset(void);
extern uint16_t TFTLCD_readID(void);
extern void TFTLCD_drawPixel(int16_t x, int16_t y, uint16_t color);
extern void TFTLCD_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
extern void TFTLCD_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
extern void TFTLCD_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
extern void TFTLCD_setRotation(uint8_t r);

#ifdef SUPPORT_READ_PIXEL
extern uint16_t TFTLCD_readPixel(int16_t x, int16_t y);
#endif

#ifdef SUPPORT_VERT_SCROLL
extern void TFTLCD_vertScroll(int16_t top, int16_t scrollines, int16_t offset);
#endif

#endif
