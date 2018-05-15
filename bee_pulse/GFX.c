#include <avr/pgmspace.h>
#include <stdlib.h>
#include <string.h>
#include "GFX.h"
#include <util/delay.h>

#ifndef min
 #define min(a,b) ((a < b) ? a : b)
#endif

#define swap(a, b) { int16_t t = a; a = b; b = t; }

int16_t _width = TFTWIDTH;
int16_t _height = TFTHEIGHT; // Display w/h as modified by current rotation
int16_t cursor_x = 0;
int16_t cursor_y = 0;
uint16_t textcolor = 0xFFFF;
uint16_t textbgcolor = 0xFFFF;
uint8_t textsize = 1;
uint8_t rotation = 0;
bool wrap = true;   // If set, 'wrap' text at right edge of display
bool _cp437 = false; // If set, use correct CP437 charset (default is off)
uint16_t chkBoxBckGndColor = 0xff;
uint16_t chkBoxColor = 0;
uint16_t radioBckGndColor = 0xff;
uint16_t radioColor = 0;
uint8_t _debounce_count = 40;
uint8_t screen_debounce_count = 0;

// Standard ASCII 5x7 font

static const unsigned char font[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x3E, 0x5B, 0x4F, 0x5B, 0x3E,
	0x3E, 0x6B, 0x4F, 0x6B, 0x3E,
	0x1C, 0x3E, 0x7C, 0x3E, 0x1C,
	0x18, 0x3C, 0x7E, 0x3C, 0x18,
	0x1C, 0x57, 0x7D, 0x57, 0x1C,
	0x1C, 0x5E, 0x7F, 0x5E, 0x1C,
	0x00, 0x18, 0x3C, 0x18, 0x00,
	0xFF, 0xE7, 0xC3, 0xE7, 0xFF,
	0x00, 0x18, 0x24, 0x18, 0x00,
	0xFF, 0xE7, 0xDB, 0xE7, 0xFF,
	0x30, 0x48, 0x3A, 0x06, 0x0E,
	0x26, 0x29, 0x79, 0x29, 0x26,
	0x40, 0x7F, 0x05, 0x05, 0x07,
	0x40, 0x7F, 0x05, 0x25, 0x3F,
	0x5A, 0x3C, 0xE7, 0x3C, 0x5A,
	0x7F, 0x3E, 0x1C, 0x1C, 0x08,
	0x08, 0x1C, 0x1C, 0x3E, 0x7F,
	0x14, 0x22, 0x7F, 0x22, 0x14,
	0x5F, 0x5F, 0x00, 0x5F, 0x5F,
	0x06, 0x09, 0x7F, 0x01, 0x7F,
	0x00, 0x66, 0x89, 0x95, 0x6A,
	0x60, 0x60, 0x60, 0x60, 0x60,
	0x94, 0xA2, 0xFF, 0xA2, 0x94,
	0x08, 0x04, 0x7E, 0x04, 0x08,
	0x10, 0x20, 0x7E, 0x20, 0x10,
	0x08, 0x08, 0x2A, 0x1C, 0x08,
	0x08, 0x1C, 0x2A, 0x08, 0x08,
	0x1E, 0x10, 0x10, 0x10, 0x10,
	0x0C, 0x1E, 0x0C, 0x1E, 0x0C,
	0x30, 0x38, 0x3E, 0x38, 0x30,
	0x06, 0x0E, 0x3E, 0x0E, 0x06,
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x5F, 0x00, 0x00,
	0x00, 0x07, 0x00, 0x07, 0x00,
	0x14, 0x7F, 0x14, 0x7F, 0x14,
	0x24, 0x2A, 0x7F, 0x2A, 0x12,
	0x23, 0x13, 0x08, 0x64, 0x62,
	0x36, 0x49, 0x56, 0x20, 0x50,
	0x00, 0x08, 0x07, 0x03, 0x00,
	0x00, 0x1C, 0x22, 0x41, 0x00,
	0x00, 0x41, 0x22, 0x1C, 0x00,
	0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
	0x08, 0x08, 0x3E, 0x08, 0x08,
	0x00, 0x80, 0x70, 0x30, 0x00,
	0x08, 0x08, 0x08, 0x08, 0x08,
	0x00, 0x00, 0x60, 0x60, 0x00,
	0x20, 0x10, 0x08, 0x04, 0x02,
	0x3E, 0x51, 0x49, 0x45, 0x3E,
	0x00, 0x42, 0x7F, 0x40, 0x00,
	0x72, 0x49, 0x49, 0x49, 0x46,
	0x21, 0x41, 0x49, 0x4D, 0x33,
	0x18, 0x14, 0x12, 0x7F, 0x10,
	0x27, 0x45, 0x45, 0x45, 0x39,
	0x3C, 0x4A, 0x49, 0x49, 0x31,
	0x41, 0x21, 0x11, 0x09, 0x07,
	0x36, 0x49, 0x49, 0x49, 0x36,
	0x46, 0x49, 0x49, 0x29, 0x1E,
	0x00, 0x00, 0x14, 0x00, 0x00,
	0x00, 0x40, 0x34, 0x00, 0x00,
	0x00, 0x08, 0x14, 0x22, 0x41,
	0x14, 0x14, 0x14, 0x14, 0x14,
	0x00, 0x41, 0x22, 0x14, 0x08,
	0x02, 0x01, 0x59, 0x09, 0x06,
	0x3E, 0x41, 0x5D, 0x59, 0x4E,
	0x7C, 0x12, 0x11, 0x12, 0x7C,
	0x7F, 0x49, 0x49, 0x49, 0x36,
	0x3E, 0x41, 0x41, 0x41, 0x22,
	0x7F, 0x41, 0x41, 0x41, 0x3E,
	0x7F, 0x49, 0x49, 0x49, 0x41,
	0x7F, 0x09, 0x09, 0x09, 0x01,
	0x3E, 0x41, 0x41, 0x51, 0x73,
	0x7F, 0x08, 0x08, 0x08, 0x7F,
	0x00, 0x41, 0x7F, 0x41, 0x00,
	0x20, 0x40, 0x41, 0x3F, 0x01,
	0x7F, 0x08, 0x14, 0x22, 0x41,
	0x7F, 0x40, 0x40, 0x40, 0x40,
	0x7F, 0x02, 0x1C, 0x02, 0x7F,
	0x7F, 0x04, 0x08, 0x10, 0x7F,
	0x3E, 0x41, 0x41, 0x41, 0x3E,
	0x7F, 0x09, 0x09, 0x09, 0x06,
	0x3E, 0x41, 0x51, 0x21, 0x5E,
	0x7F, 0x09, 0x19, 0x29, 0x46,
	0x26, 0x49, 0x49, 0x49, 0x32,
	0x03, 0x01, 0x7F, 0x01, 0x03,
	0x3F, 0x40, 0x40, 0x40, 0x3F,
	0x1F, 0x20, 0x40, 0x20, 0x1F,
	0x3F, 0x40, 0x38, 0x40, 0x3F,
	0x63, 0x14, 0x08, 0x14, 0x63,
	0x03, 0x04, 0x78, 0x04, 0x03,
	0x61, 0x59, 0x49, 0x4D, 0x43,
	0x00, 0x7F, 0x41, 0x41, 0x41,
	0x02, 0x04, 0x08, 0x10, 0x20,
	0x00, 0x41, 0x41, 0x41, 0x7F,
	0x04, 0x02, 0x01, 0x02, 0x04,
	0x40, 0x40, 0x40, 0x40, 0x40,
	0x00, 0x03, 0x07, 0x08, 0x00,
	0x20, 0x54, 0x54, 0x78, 0x40,
	0x7F, 0x28, 0x44, 0x44, 0x38,
	0x38, 0x44, 0x44, 0x44, 0x28,
	0x38, 0x44, 0x44, 0x28, 0x7F,
	0x38, 0x54, 0x54, 0x54, 0x18,
	0x00, 0x08, 0x7E, 0x09, 0x02,
	0x18, 0xA4, 0xA4, 0x9C, 0x78,
	0x7F, 0x08, 0x04, 0x04, 0x78,
	0x00, 0x44, 0x7D, 0x40, 0x00,
	0x20, 0x40, 0x40, 0x3D, 0x00,
	0x7F, 0x10, 0x28, 0x44, 0x00,
	0x00, 0x41, 0x7F, 0x40, 0x00,
	0x7C, 0x04, 0x78, 0x04, 0x78,
	0x7C, 0x08, 0x04, 0x04, 0x78,
	0x38, 0x44, 0x44, 0x44, 0x38,
	0xFC, 0x18, 0x24, 0x24, 0x18,
	0x18, 0x24, 0x24, 0x18, 0xFC,
	0x7C, 0x08, 0x04, 0x04, 0x08,
	0x48, 0x54, 0x54, 0x54, 0x24,
	0x04, 0x04, 0x3F, 0x44, 0x24,
	0x3C, 0x40, 0x40, 0x20, 0x7C,
	0x1C, 0x20, 0x40, 0x20, 0x1C,
	0x3C, 0x40, 0x30, 0x40, 0x3C,
	0x44, 0x28, 0x10, 0x28, 0x44,
	0x4C, 0x90, 0x90, 0x90, 0x7C,
	0x44, 0x64, 0x54, 0x4C, 0x44,
	0x00, 0x08, 0x36, 0x41, 0x00,
	0x00, 0x00, 0x77, 0x00, 0x00,
	0x00, 0x41, 0x36, 0x08, 0x00,
	0x02, 0x01, 0x02, 0x04, 0x02,
	0x3C, 0x26, 0x23, 0x26, 0x3C,
	0x1E, 0xA1, 0xA1, 0x61, 0x12,
	0x3A, 0x40, 0x40, 0x20, 0x7A,
	0x38, 0x54, 0x54, 0x55, 0x59,
	0x21, 0x55, 0x55, 0x79, 0x41,
	0x22, 0x54, 0x54, 0x78, 0x42, // a-umlaut
	0x21, 0x55, 0x54, 0x78, 0x40,
	0x20, 0x54, 0x55, 0x79, 0x40,
	0x0C, 0x1E, 0x52, 0x72, 0x12,
	0x39, 0x55, 0x55, 0x55, 0x59,
	0x39, 0x54, 0x54, 0x54, 0x59,
	0x39, 0x55, 0x54, 0x54, 0x58,
	0x00, 0x00, 0x45, 0x7C, 0x41,
	0x00, 0x02, 0x45, 0x7D, 0x42,
	0x00, 0x01, 0x45, 0x7C, 0x40,
	0x7D, 0x12, 0x11, 0x12, 0x7D, // A-umlaut
	0xF0, 0x28, 0x25, 0x28, 0xF0,
	0x7C, 0x54, 0x55, 0x45, 0x00,
	0x20, 0x54, 0x54, 0x7C, 0x54,
	0x7C, 0x0A, 0x09, 0x7F, 0x49,
	0x32, 0x49, 0x49, 0x49, 0x32,
	0x3A, 0x44, 0x44, 0x44, 0x3A, // o-umlaut
	0x32, 0x4A, 0x48, 0x48, 0x30,
	0x3A, 0x41, 0x41, 0x21, 0x7A,
	0x3A, 0x42, 0x40, 0x20, 0x78,
	0x00, 0x9D, 0xA0, 0xA0, 0x7D,
	0x3D, 0x42, 0x42, 0x42, 0x3D, // O-umlaut
	0x3D, 0x40, 0x40, 0x40, 0x3D,
	0x3C, 0x24, 0xFF, 0x24, 0x24,
	0x48, 0x7E, 0x49, 0x43, 0x66,
	0x2B, 0x2F, 0xFC, 0x2F, 0x2B,
	0xFF, 0x09, 0x29, 0xF6, 0x20,
	0xC0, 0x88, 0x7E, 0x09, 0x03,
	0x20, 0x54, 0x54, 0x79, 0x41,
	0x00, 0x00, 0x44, 0x7D, 0x41,
	0x30, 0x48, 0x48, 0x4A, 0x32,
	0x38, 0x40, 0x40, 0x22, 0x7A,
	0x00, 0x7A, 0x0A, 0x0A, 0x72,
	0x7D, 0x0D, 0x19, 0x31, 0x7D,
	0x26, 0x29, 0x29, 0x2F, 0x28,
	0x26, 0x29, 0x29, 0x29, 0x26,
	0x30, 0x48, 0x4D, 0x40, 0x20,
	0x38, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x38,
	0x2F, 0x10, 0xC8, 0xAC, 0xBA,
	0x2F, 0x10, 0x28, 0x34, 0xFA,
	0x00, 0x00, 0x7B, 0x00, 0x00,
	0x08, 0x14, 0x2A, 0x14, 0x22,
	0x22, 0x14, 0x2A, 0x14, 0x08,
	0x55, 0x00, 0x55, 0x00, 0x55, // #176 (25% block) missing in old code
	0xAA, 0x55, 0xAA, 0x55, 0xAA, // 50% block
	0xFF, 0x55, 0xFF, 0x55, 0xFF, // 75% block
	0x00, 0x00, 0x00, 0xFF, 0x00,
	0x10, 0x10, 0x10, 0xFF, 0x00,
	0x14, 0x14, 0x14, 0xFF, 0x00,
	0x10, 0x10, 0xFF, 0x00, 0xFF,
	0x10, 0x10, 0xF0, 0x10, 0xF0,
	0x14, 0x14, 0x14, 0xFC, 0x00,
	0x14, 0x14, 0xF7, 0x00, 0xFF,
	0x00, 0x00, 0xFF, 0x00, 0xFF,
	0x14, 0x14, 0xF4, 0x04, 0xFC,
	0x14, 0x14, 0x17, 0x10, 0x1F,
	0x10, 0x10, 0x1F, 0x10, 0x1F,
	0x14, 0x14, 0x14, 0x1F, 0x00,
	0x10, 0x10, 0x10, 0xF0, 0x00,
	0x00, 0x00, 0x00, 0x1F, 0x10,
	0x10, 0x10, 0x10, 0x1F, 0x10,
	0x10, 0x10, 0x10, 0xF0, 0x10,
	0x00, 0x00, 0x00, 0xFF, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x10, 0xFF, 0x10,
	0x00, 0x00, 0x00, 0xFF, 0x14,
	0x00, 0x00, 0xFF, 0x00, 0xFF,
	0x00, 0x00, 0x1F, 0x10, 0x17,
	0x00, 0x00, 0xFC, 0x04, 0xF4,
	0x14, 0x14, 0x17, 0x10, 0x17,
	0x14, 0x14, 0xF4, 0x04, 0xF4,
	0x00, 0x00, 0xFF, 0x00, 0xF7,
	0x14, 0x14, 0x14, 0x14, 0x14,
	0x14, 0x14, 0xF7, 0x00, 0xF7,
	0x14, 0x14, 0x14, 0x17, 0x14,
	0x10, 0x10, 0x1F, 0x10, 0x1F,
	0x14, 0x14, 0x14, 0xF4, 0x14,
	0x10, 0x10, 0xF0, 0x10, 0xF0,
	0x00, 0x00, 0x1F, 0x10, 0x1F,
	0x00, 0x00, 0x00, 0x1F, 0x14,
	0x00, 0x00, 0x00, 0xFC, 0x14,
	0x00, 0x00, 0xF0, 0x10, 0xF0,
	0x10, 0x10, 0xFF, 0x10, 0xFF,
	0x14, 0x14, 0x14, 0xFF, 0x14,
	0x10, 0x10, 0x10, 0x1F, 0x00,
	0x00, 0x00, 0x00, 0xF0, 0x10,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0xFF, 0xFF, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xFF, 0xFF,
	0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	0x38, 0x44, 0x44, 0x38, 0x44,
	0xFC, 0x4A, 0x4A, 0x4A, 0x34, // sharp-s or beta
	0x7E, 0x02, 0x02, 0x06, 0x06,
	0x02, 0x7E, 0x02, 0x7E, 0x02,
	0x63, 0x55, 0x49, 0x41, 0x63,
	0x38, 0x44, 0x44, 0x3C, 0x04,
	0x40, 0x7E, 0x20, 0x1E, 0x20,
	0x06, 0x02, 0x7E, 0x02, 0x02,
	0x99, 0xA5, 0xE7, 0xA5, 0x99,
	0x1C, 0x2A, 0x49, 0x2A, 0x1C,
	0x4C, 0x72, 0x01, 0x72, 0x4C,
	0x30, 0x4A, 0x4D, 0x4D, 0x30,
	0x30, 0x48, 0x78, 0x48, 0x30,
	0xBC, 0x62, 0x5A, 0x46, 0x3D,
	0x3E, 0x49, 0x49, 0x49, 0x00,
	0x7E, 0x01, 0x01, 0x01, 0x7E,
	0x2A, 0x2A, 0x2A, 0x2A, 0x2A,
	0x44, 0x44, 0x5F, 0x44, 0x44,
	0x40, 0x51, 0x4A, 0x44, 0x40,
	0x40, 0x44, 0x4A, 0x51, 0x40,
	0x00, 0x00, 0xFF, 0x01, 0x03,
	0xE0, 0x80, 0xFF, 0x00, 0x00,
	0x08, 0x08, 0x6B, 0x6B, 0x08,
	0x36, 0x12, 0x36, 0x24, 0x36,
	0x06, 0x0F, 0x09, 0x0F, 0x06,
	0x00, 0x00, 0x18, 0x18, 0x00,
	0x00, 0x00, 0x10, 0x10, 0x00,
	0x30, 0x40, 0xFF, 0x01, 0x01,
	0x00, 0x1F, 0x01, 0x01, 0x1E,
	0x00, 0x19, 0x1D, 0x17, 0x12,
	0x00, 0x3C, 0x3C, 0x3C, 0x3C,
	0x00, 0x00, 0x00, 0x00, 0x00  // #255 NBSP
};



// void GFX_drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h,  uint16_t color) {

//   int16_t i, j, byteWidth = (w + 7) / 8;

//   for(j=0; j<h; j++) {
//     for(i=0; i<w; i++ ) {
//       if(pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
//         TFTLCD_drawPixel(x+i, y+j, color);
//       }
//     }
//   }
// }

// Draw a 1-bit color bitmap at the specified x, y position from the
// provided bitmap buffer (must be PROGMEM memory) using color as the
// foreground color and bg as the background color.
// void GFX_drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg) {

//   int16_t i, j, byteWidth = (w + 7) / 8;
  
//   for(j=0; j<h; j++) {
//     for(i=0; i<w; i++ ) {
//       if(pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
//         TFTLCD_drawPixel(x+i, y+j, color);
//       }
//       else {
//       	TFTLCD_drawPixel(x+i, y+j, bg);
//       }
//     }
//   }
// }

// Draw XBitMap Files (*.xbm), exported from GIMP,
// Usage: Export from GIMP to *.xbm, rename *.xbm to *.c and open in editor.
// C Array can be directly used with this function
//void GFX_drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
//
//  int16_t i, j, byteWidth = (w + 7) / 8;
//
//  for(j=0; j<h; j++) {
//    for(i=0; i<w; i++ ) {
//      if(pgm_read_byte(bitmap + j * byteWidth + i / 8) & (1 << (i % 8))) {
//        TFTLCD_drawPixel(x+i, y+j, color);
//      }
//    }
//  }
//}


uint16_t GFX_init( void ){
  _width = TFTWIDTH;
  _height = TFTHEIGHT; // Display w/h as modified by current rotation
  cursor_x = 0;
  cursor_y = 0;
  textcolor = 0xFFFF;
  textbgcolor = 0xFFFF;
  textsize = 1;
  rotation = 0;
  wrap = true;   // If set, 'wrap' text at right edge of display
  _cp437 = false; // If set, use correct CP437 charset (default is off)

  TFTLCD_begin();
  TFTLCD_setRotation(0);             //PORTRAIT

  return TFTLCD_readID();
}

void GFX_drawPixel(int16_t x, int16_t y, uint16_t color){
	TFTLCD_drawPixel(x, y, color);
}

// Bresenham's algorithm - thx wikpedia
void GFX_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      TFTLCD_drawPixel(y0, x0, color);
    } else {
      TFTLCD_drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}


void GFX_fillScreen(uint16_t color) {
	TFTLCD_fillRect(0, 0, _width, _height, color);
}

void GFX_fillRect(int16_t x, int16_t y, int16_t w, int16_t h,  uint16_t color) {
	TFTLCD_fillRect(x, y, w, h, color);
}

void GFX_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
	TFTLCD_fillRect(x, y, 1, h, color);
}

void GFX_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
	TFTLCD_fillRect(x, y, w, 1, color);
}

// Draw a rectangle
void GFX_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	TFTLCD_fillRect(x, y, w, 1, color);
	TFTLCD_fillRect(x, y+h-1, w, 1, color);
	TFTLCD_fillRect(x, y, 1, h, color);
	TFTLCD_fillRect(x+w-1, y, 1, h, color);;
}

// Draw a circle outline
void GFX_drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  TFTLCD_drawPixel(x0  , y0+r, color);
  TFTLCD_drawPixel(x0  , y0-r, color);
  TFTLCD_drawPixel(x0+r, y0  , color);
  TFTLCD_drawPixel(x0-r, y0  , color);

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    TFTLCD_drawPixel(x0 + x, y0 + y, color);
    TFTLCD_drawPixel(x0 - x, y0 + y, color);
    TFTLCD_drawPixel(x0 + x, y0 - y, color);
    TFTLCD_drawPixel(x0 - x, y0 - y, color);
    TFTLCD_drawPixel(x0 + y, y0 + x, color);
    TFTLCD_drawPixel(x0 - y, y0 + x, color);
    TFTLCD_drawPixel(x0 + y, y0 - x, color);
    TFTLCD_drawPixel(x0 - y, y0 - x, color);
  }
}


void GFX_drawRoundCorner( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    if (cornername & 0x4) {
      TFTLCD_drawPixel(x0 + x, y0 + y, color);
      TFTLCD_drawPixel(x0 + y, y0 + x, color);
    }
    if (cornername & 0x2) {
      TFTLCD_drawPixel(x0 + x, y0 - y, color);
      TFTLCD_drawPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      TFTLCD_drawPixel(x0 - y, y0 + x, color);
      TFTLCD_drawPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      TFTLCD_drawPixel(x0 - y, y0 - x, color);
      TFTLCD_drawPixel(x0 - x, y0 - y, color);
    }
  }
}

void GFX_fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  GFX_drawFastVLine(x0, y0-r, 2*r+1, color);
  GFX_fillRoundCorner(x0, y0, r, 3, 0, color);
}

// Used to do circles and roundrects
void GFX_fillRoundCorner(int16_t x0, int16_t y0, int16_t r,
    uint8_t cornername, int16_t delta, uint16_t color) {

  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1) {
      GFX_drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
      GFX_drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
    }
    if (cornername & 0x2) {
      GFX_drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
      GFX_drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}

// Draw a triangle
void GFX_drawTriangle(int16_t x0, int16_t y0,	int16_t x1, int16_t y1,
				int16_t x2, int16_t y2, uint16_t color) {
  GFX_drawLine(x0, y0, x1, y1, color);
  GFX_drawLine(x1, y1, x2, y2, color);
  GFX_drawLine(x2, y2, x0, y0, color);
}


// Fill a triangle
void GFX_fillTriangle( int16_t x0, int16_t y0, int16_t x1, int16_t y1,
				  int16_t x2, int16_t y2, uint16_t color) {

  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1); swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)      a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    GFX_drawFastHLine(a, y0, b-a+1, color);
    return;
  }

  int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1;
  int32_t
    sa   = 0,
    sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1;   // Include y1 scanline
  else         last = y1-1; // Skip it

  for(y=y0; y<=last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    GFX_drawFastHLine(a, y, b-a+1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    GFX_drawFastHLine(a, y, b-a+1, color);
  }
}

// Draw a rounded rectangle
void GFX_drawRoundRect(int16_t x, int16_t y, int16_t w,  int16_t h, int16_t r, uint16_t color) {
  // smarter version
  GFX_drawFastHLine(x+r  , y    , w-2*r, color); // Top
  GFX_drawFastHLine(x+r  , y+h-1, w-2*r, color); // Bottom
  GFX_drawFastVLine(x    , y+r  , h-2*r, color); // Left
  GFX_drawFastVLine(x+w-1, y+r  , h-2*r, color); // Right
  // draw four corners
  GFX_drawRoundCorner(x+r    , y+r    , r, 1, color);
  GFX_drawRoundCorner(x+w-r-1, y+r    , r, 2, color);
  GFX_drawRoundCorner(x+w-r-1, y+h-r-1, r, 4, color);
  GFX_drawRoundCorner(x+r    , y+h-r-1, r, 8, color);
}

// Fill a rounded rectangle
void GFX_fillRoundRect(int16_t x, int16_t y, int16_t w,
				 int16_t h, int16_t r, uint16_t color) {
  // smarter version
  GFX_fillRect(x+r, y, w-2*r, h, color);

  // draw four corners
  GFX_fillRoundCorner(x+w-r-1, y+r, r, 1, h-2*r-1, color);
  GFX_fillRoundCorner(x+r    , y+r, r, 2, h-2*r-1, color);
}

/* Draw a character
 * For a transparent character use same color for background
 */
void GFX_drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {

  if((x >= _width)            || // Clip right
     (y >= _height)           || // Clip bottom
     ((x + 6 * size - 1) < 0) || // Clip left
     ((y + 8 * size - 1) < 0))   // Clip top
    return;

  if(!_cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

  for (int8_t i=0; i<6; i++ ) {
    uint8_t line;
    if (i == 5)
      line = 0x0;
    else
      line = pgm_read_byte(font+(c*5)+i);
    for (int8_t j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          TFTLCD_drawPixel(x+i, y+j, color);
        else {  // big size
          GFX_fillRect(x+(i*size), y+(j*size), size, size, color);
        }
      } else if (bg != color) {
        if (size == 1) // default size
          TFTLCD_drawPixel(x+i, y+j, bg);
        else {  // big size
          GFX_fillRect(x+i*size, y+j*size, size, size, bg);
        }
      }
      line >>= 1;
    }
  }
}

/* Sets default text color and background color for printing text.
 * For transparent background use same color for background as text
 * */
void GFX_setTextColor(uint16_t c, uint16_t b) {
	textcolor   = c;
	textbgcolor = b;
}

void GFX_setTextSize(uint8_t s) {
	textsize = (s > 0) ? s : 1;
}

void GFX_printStr(char *string){
	uint8_t i = 0;

	while(string[i] != 0){
		GFX_printChar(string[i]);
		i++;
	}
}

void GFX_printDec(uint16_t number){
	uint16_t value = number;
	char snum[6] = {0};
	uint8_t i = 0;
	uint8_t digit;
	uint16_t divider = 10000;

	if(number == 0){
		snum[0] = '0';
	}else{
		while(divider > 0){
			digit = value / divider;
			if((i > 0) || (digit > 0)){
				snum[i] = digit + '0';
				i++;
			}
			value = value % divider;
			divider = divider / 10;
		}
	}

	GFX_printStr(snum);
}

void GFX_printHex(uint16_t number){
	uint16_t value = number;
	char snum[5] = {'0', '0', '0', '0', 0};
	uint8_t digit;
	uint8_t i;

	for(i=0; i<4; i++){
		digit = (value >> (i * 4)) & 0xf;

		if(digit < 10){
			snum[3 - i] = '0' + digit;
		}else{
			snum[3 - i] = 'A' + digit - 10;
		}
	}

	GFX_printStr(snum);
}

void GFX_setCursor(int16_t x, int16_t y) {
	cursor_x = x;
	cursor_y = y;
}

int16_t GFX_getCursorX(void) {
	return cursor_x;
}

int16_t GFX_getCursorY(void) {
	return cursor_y;
}

void GFX_setTextWrap(bool w) {
	wrap = w;
}

uint8_t GFX_getRotation(void){
	return rotation;
}

void GFX_setRotation(uint8_t x) {
	rotation = (x & 3);
	switch(rotation) {
		case 0:
		case 2:
			_width  = TFTWIDTH;
			_height = TFTHEIGHT;
			break;
		case 1:
		case 3:
			_width  = TFTHEIGHT;
			_height = TFTWIDTH;
		break;
	}

	TFTLCD_setRotation(x);
}

// Enable (or disable) Code Page 437-compatible charset.
// There was an error in glcdfont.c for the longest time -- one character
// (#176, the 'light shade' block) was missing -- this threw off the index
// of every character that followed it.  But a TON of code has been written
// with the erroneous character indices.  By default, the library uses the
// original 'wrong' behavior and old sketches will still work.  Pass 'true'
// to this function to use correct CP437 character values in your code.
void GFX_cp437(bool x) {
  _cp437 = x;
}

// Return the size of the display (per current rotation)
int16_t GFX_getWidth(void) {
  return _width;
}

int16_t GFX_getHeight(void) {
  return _height;
}

void GFX_setWidth(int16_t w) {
	_width = w;
}

void GFX_setHeight(int16_t h) {
	_height = h;
}

void GFX_printChar(uint8_t c) {

  if (c == '\n') {
    cursor_y += textsize*TXTH;
    cursor_x  = 0;
  } else if (c == '\r') {
    // skip em
  } else {
    GFX_drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
    cursor_x += textsize*TXTW;
    if (wrap && (cursor_x > (_width - textsize*TXTW))) {
      cursor_y += textsize*TXTH;
      cursor_x = 0;
    }
  }
}

#ifdef SUPPORT_VERT_SCROLL
void GFX_vertScroll(int16_t top, int16_t scrollines, int16_t offset, uint16_t color){
	uint16_t i;

	for(i=0; i < scrollines; i++){
		TFTLCD_vertScroll(top, scrollines, i);
		GFX_drawFastHLine(0, i, TFTWIDTH, color);
		_delay_ms(1);
	}
	_delay_ms(500); /* Wait for rotation to end */
}
#endif

#ifdef SUPPORT_READ_PIXEL
uint16_t GFX_readPixel(int16_t x, int16_t y){
	return TFTLCD_readPixel(x, y);
}
#endif

/***************************************************************************/
// GFX button UI element

#ifdef SUPPORT_BUTTON
void GFX_btnDraw(gfx_btn *btn, bool inverted) {
	/* Save current setting so we can restore it */
	uint16_t current_text_color = textcolor;
	uint16_t current_back_color = textbgcolor;
	uint8_t current_textsize = textsize;

	if (! inverted) {
		GFX_fillRoundRect(btn->x - (btn->width/2), btn->y - (btn->height/2), btn->width, btn->height,
				min(btn->width, btn->height)/4, btn->fillcolor);
		GFX_setTextColor(btn->textcolor, btn->fillcolor);
	} else {
		GFX_fillRoundRect(btn->x - (btn->width/2), btn->y - (btn->height/2), btn->width, btn->height,
				min(btn->width, btn->height)/4, btn->textcolor);
		GFX_setTextColor(btn->fillcolor, btn->textcolor);
	}

	GFX_drawRoundRect(btn->x - (btn->width/2), btn->y - (btn->height/2), btn->width, btn->height, min(btn->width, btn->height)/4, btn->outlinecolor);
	GFX_setCursor(btn->x - strlen(btn->label)*(TXTW/2)*btn->textsize, btn->y-(TXTH/2)*btn->textsize);
	GFX_setTextSize(btn->textsize);
	GFX_printStr(btn->label);

	/* Restore current settings */
	textcolor = current_text_color;
	textbgcolor = current_back_color;
	textsize = current_textsize;
}

bool GFX_btnContains(gfx_btn *btn, int16_t x, int16_t y) {
	if ((x < (btn->x - btn->width/2)) || (x > (btn->x + btn->width/2))) return false;
	if ((y < (btn->y - btn->height/2)) || (y > (btn->y + btn->height/2))) return false;
	return true;
}

void GFX_btnPress(gfx_btn *btn, bool p) {

	if(p){
		if(btn->debounce == 0){
			GFX_btnDraw(btn, true);
			btn->laststate = false;
			btn->currstate = true;
		}else{
			btn->laststate = true;
			btn->currstate = true;
		}

		btn->debounce = _debounce_count;
	}else{
		if(btn->debounce > 0){
			btn->debounce--;
		}else{
			btn->laststate = false;
			btn->currstate = false;
		}

		if(btn->debounce == 1){
			GFX_btnDraw(btn, false);
			btn->laststate = true;
			btn->currstate = false;
		}
	}
}

bool GFX_btnIsPressed(gfx_btn *btn) {
	return btn->currstate;
}

bool GFX_btnJustReleased(gfx_btn *btn) {
	bool res = (!(btn->currstate)) && btn->laststate;
	return (res);
}

bool GFX_btnJustPressed(gfx_btn *btn) {
	bool res = btn->currstate && (!btn->laststate) && (btn->debounce == _debounce_count);
	return res;
}

void GFX_btnUpdate(gfx_btn *btn, TSPoint *point){

	if(point->z > 0){
		if(GFX_btnContains(btn, point->x, point->y)){
			GFX_btnPress(btn, true);
		}else{
			GFX_btnPress(btn, false);
		}

		screen_debounce_count = _debounce_count;
	}else{
		GFX_btnPress(btn, false);
	}
}
#endif
/***************************************************************************/
// GFX check box UI element
#ifdef SUPPORT_CHECKBOX

void GFX_chkBoxSetColor(uint16_t color, uint16_t bckGndColor){
	chkBoxColor = color;
	chkBoxBckGndColor = bckGndColor;
}

void GFX_chkBoxDraw(gfx_chkbox *chk) {
	uint16_t x = chk->x - chk->width/2;
	uint16_t y = chk->y - chk->width/2;
	uint16_t r = chk->width/4;

	GFX_fillRoundRect(x, y, chk->width, chk->width, r, chkBoxBckGndColor);
	GFX_drawRoundRect(x, y, chk->width, chk->width, r, chkBoxColor);


	if(chk->checked){
		GFX_drawLine(x + chk->width/4 - 1, y + chk->width/4 - 1,
				x + chk->width - chk->width/4, y + chk->width - chk->width/4, chkBoxColor);
		GFX_drawLine(x + chk->width/4 - 1, y + chk->width - chk->width/4,
				x + chk->width - chk->width/4, y + chk->width/4 - 1, chkBoxColor);
	}
	chk->debounce = 0;
}

bool GFX_chkBoxContains(gfx_chkbox *chk, int16_t x, int16_t y) {
	if ((x < (chk->x - chk->width/2)) || (x > (chk->x + chk->width/2))) return false;
	if ((y < (chk->y - chk->width/2)) || (y > (chk->y + chk->width/2))) return false;
	return true;
}

void GFX_chkBoxPress(gfx_chkbox *chk, bool p) {
	if(p){
		if(chk->debounce == 0){
			chk->checked = !chk->checked;
			GFX_chkBoxDraw(chk);
		}

		chk->debounce = 0xff;
	}else{
		if(chk->debounce > 0){
			chk->debounce--;
		}
	}
}

void GFX_chkBoxUpdate(gfx_chkbox *chk, TSPoint *point){
	if(point->z > 0){
		if(GFX_chkBoxContains(chk, point->x, point->y)){
			GFX_chkBoxPress(chk, true);
		}else{
			GFX_chkBoxPress(chk, false);
		}

		screen_debounce_count = _debounce_count;

	}else{
		GFX_chkBoxPress(chk, false);
	}
}

bool GFX_chkBoxChecked(gfx_chkbox *chk){
	return chk->checked;
}
#endif

/***************************************************************************/
// GFX LED element
#ifdef SUPPORT_LED
void GFX_LEDDraw(gfx_led *led, bool state) {

	if(state){
		GFX_fillCircle(led->x, led->y, led->radius, led->on_color );
	}else{
		GFX_fillCircle(led->x, led->y, led->radius, led->off_color );
		GFX_drawCircle(led->x, led->y, led->radius, led->on_color );
	}
}
#endif

/***************************************************************************/
// GFX radio button element

#ifdef SUPPORT_RADIO_BUTTON
void GFX_radioBtnSetColor(uint16_t color, uint16_t bckGndColor){
	radioColor = color;
	radioBckGndColor = bckGndColor;
}

void GFX_radioBtnDraw(gfx_radiobtn *radio, bool state) {
	if(radio->radius < 3){
		radio->radius = 3;
	}

	GFX_fillCircle(radio->x, radio->y, radio->radius, radioBckGndColor );
	GFX_drawCircle(radio->x, radio->y, radio->radius, radioColor );

	if(state){
		GFX_fillCircle(radio->x, radio->y, radio->radius - 2, radioColor );
	}
}

bool GFX_radioBtnContains(gfx_radiobtn *radio, int16_t x, int16_t y) {
	int xd = radio->x - x;
	int yd = radio->y - y;

	if (((xd * xd) + (yd * yd)) > (radio->radius * radio->radius)) return false;

	return true;
}

bool GFX_radioBtnPressed(gfx_radiobtn *radio, TSPoint *point) {
	if(point->z > 0){
		if(GFX_radioBtnContains(radio, point->x, point->y)){
			return true;
		}
	}
	return false;
}
#endif

void GFX_setDebounceCount(uint8_t count){
	_debounce_count = count;
}

bool GFX_screenPressed(TSPoint *point){
	bool retVal = false;

	if(point->z > 0){
		screen_debounce_count = _debounce_count;
	}else{
		if(screen_debounce_count > 0){
			screen_debounce_count--;
		}
	}

	if(screen_debounce_count > 0){
		retVal = true;
	}

	return retVal;
}
