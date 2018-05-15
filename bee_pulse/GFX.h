#ifndef _GFX_H
#define _GFX_H

#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "TouchScreen.h"
#include "LCD.h"


/* Define correct text width and height for the font in the GFX.c file,
 * so we can use it in the other project files. */
#define TXTW	(6)
#define TXTH	(8)

/* Corner definitions can be used in the
 * GFX_drawRoundCorner and GFX_fillRoundCorner functions */
#define CORNER_NW	(0b0001)
#define CORNER_NE	(0b0010)
#define CORNER_SE	(0b0100)
#define CORNER_SW	(0b1000)

/* Basic element definitions */
typedef struct gfx_btn_t{
	int16_t x;
	int16_t y;
	uint16_t outlinecolor;
	uint16_t fillcolor;
	uint16_t textcolor;
	uint8_t width;
	uint8_t height;
	uint8_t textsize;
	uint8_t debounce;
	bool currstate;
	bool laststate;
	char *label;
}gfx_btn;

typedef struct gfx_chkbox_t{
	int16_t x;
	int16_t y;
	uint8_t width;
	bool checked;
	uint8_t debounce;
}gfx_chkbox;

typedef struct gfx_led_t{
	int16_t x;
	int16_t y;
	uint16_t off_color;
	uint16_t on_color;
	uint8_t radius;
}gfx_led;

typedef struct gfx_radio_t{
	int16_t x;
	int16_t y;
	uint8_t radius;
}gfx_radiobtn;

/* Basic functions. These are recommended and minimum to use. The rest are optional. */



/* general functions */
extern uint16_t GFX_init( void );		// returns display ID
extern void GFX_setRotation(uint8_t r);
extern void GFX_drawPixel(int16_t x, int16_t y, uint16_t color);
extern void GFX_fillScreen(uint16_t color);

#ifdef SUPPORT_VERT_SCROLL
extern void GFX_vertScroll(int16_t top, int16_t scrollines, int16_t offset, uint16_t color);
#endif

/* Line functions */
extern void GFX_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
extern void GFX_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
extern void GFX_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

/* Rectangle functions */
extern void GFX_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
extern void GFX_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
extern void GFX_drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
extern void GFX_fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);

/* Radius functions */
extern void GFX_drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
extern void GFX_fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

/* Triangle functions */
extern void GFX_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
extern void GFX_fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

/* Text functions */
extern void GFX_setTextColor(uint16_t c, uint16_t b);
extern void GFX_setTextSize(uint8_t s);
extern void GFX_setTextWrap(bool w);
extern void GFX_setCursor(int16_t x, int16_t y);
extern void GFX_printStr(char *string);
extern void GFX_printChar(uint8_t c);
extern void GFX_printDec(uint16_t number);
extern void GFX_printHex(uint16_t number);

/* Button functions */
#ifdef SUPPORT_BUTTON
extern void GFX_btnDraw(gfx_btn *btn, bool inverted);
extern void GFX_btnUpdate(gfx_btn *btn, TSPoint *point);
extern bool GFX_btnJustReleased(gfx_btn *btn);
extern bool GFX_btnJustPressed(gfx_btn *btn);
#endif

/* Checkbox functions */
#ifdef SUPPORT_CHECKBOX
void GFX_chkBoxSetColor(uint16_t color, uint16_t bckGndColor);
extern void GFX_chkBoxDraw(gfx_chkbox *chk);
extern void GFX_chkBoxUpdate(gfx_chkbox *chk, TSPoint *point);
extern bool GFX_chkBoxChecked(gfx_chkbox *chk);
#endif

/* Light emitting diode (LED) */
#ifdef SUPPORT_LED
extern void GFX_LEDDraw(gfx_led *led, bool state);
#endif

/* Radio button */
#ifdef SUPPORT_RADIO_BUTTON
extern void GFX_radioBtnSetColor(uint16_t color, uint16_t bckGndColor);
extern void GFX_radioBtnDraw(gfx_radiobtn *radio, bool state);
extern bool GFX_radioBtnPressed(gfx_radiobtn *radio, TSPoint *point);
#endif


/* Additional optional functions. If the basic ones are not enough, you can use these too. */
/* general functions */
extern int16_t GFX_getHeight(void);
extern int16_t GFX_getWidth(void);
extern void GFX_setHeight(int16_t h);
extern void GFX_setWidth(int16_t w);
extern uint8_t GFX_getRotation(void);
extern bool GFX_screenPressed(TSPoint *point);

#ifdef SUPPORT_READ_PIXEL
extern uint16_t GFX_readPixel(int16_t x, int16_t y);
#endif

/* Radius functions */
extern void GFX_drawRoundCorner(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
extern void GFX_fillRoundCorner(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);

/* Text functions */
extern void GFX_drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
extern int16_t GFX_getCursorX(void);
extern int16_t GFX_getCursorY(void);
extern void GFX_cp437(bool x);

/* Button functions */
#ifdef SUPPORT_BUTTON
extern bool GFX_btnContains(gfx_btn *btn, int16_t x, int16_t y);
extern void GFX_btnPress(gfx_btn *btn, bool p);
extern bool GFX_btnIsPressed(gfx_btn *btn);
extern void GFX_setDebounceCount(uint8_t count);
#endif

/* Checkbox functions */
#ifdef SUPPORT_CHECKBOX
extern bool GFX_chkBoxContains(gfx_chkbox *chk, int16_t x, int16_t y);
extern void GFX_chkBoxPress(gfx_chkbox *chk, bool p);
#endif

/* Radio button */
#ifdef SUPPORT_RADIO_BUTTON
extern bool GFX_radioBtnContains(gfx_radiobtn *radio, int16_t x, int16_t y);
#endif

#endif // _ADAFRUIT_GFX_H
