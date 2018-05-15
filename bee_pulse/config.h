#ifndef _HW_CFG_
#define _HW_CFG_

/* Configure your TFTLCD screen here */
#define DRIVER_ID 	9341
//#define DRIVER_ID 	6767
#define TFTWIDTH   	240
#define TFTHEIGHT  	320

/* Touch digitizer configuration */
#define TS_MINX 100
#define TS_MAXX 920
#define TS_MINY 100
#define TS_MAXY 950

/* Comment out optional graphical components if not needed (Saves up to 2568 bytes)
 * */
#define SUPPORT_READ_PIXEL
#define SUPPORT_VERT_SCROLL
#define SUPPORT_CHECKBOX
#define SUPPORT_RADIO_BUTTON
#define SUPPORT_LED
#define SUPPORT_BUTTON

/* Color codes RGB565*/
#define BLACK       0x0000      /*   0,   0,   0 */
#define NAVY        0x000F      /*   0,   0, 128 */
#define DARKGREEN   0x03E0      /*   0, 128,   0 */
#define DARKCYAN    0x03EF      /*   0, 128, 128 */
#define MAROON      0x7800      /* 128,   0,   0 */
#define PURPLE      0x780F      /* 128,   0, 128 */
#define OLIVE       0x7BE0      /* 128, 128,   0 */
#define LIGHTGREY   0xC618      /* 192, 192, 192 */
#define DARKGREY    0x7BEF      /* 128, 128, 128 */
#define BLUE        0x001F      /*   0,   0, 255 */
#define GREEN       0x07E0      /*   0, 255,   0 */
#define CYAN        0x07FF      /*   0, 255, 255 */
#define RED         0xF800      /* 255,   0,   0 */
#define PINK     	0xF81F      /* 255,   0, 255 */
#define YELLOW      0xFFE0      /* 255, 255,   0 */
#define WHITE       0xFFFF      /* 255, 255, 255 */
#define ORANGE      0xFD20      /* 255, 165,   0 */
#define GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define BORDO       (5 << 11)
#define BROWN       0x1861

// LCD control lines: RD (read), WR (write), CD (command/data), CS (chip select)
#define RD_PORT     PORTC
#define WR_PORT     PORTC
#define CD_PORT     PORTC
#define CS_PORT     PORTC
#define RESET_PORT  PORTC

#define RD_PIN      PC0				    /*pin A0 */
#define WR_PIN      PC1				    /*pin A1 */
#define CD_PIN      PC2				    /*pin A2 */
#define CS_PIN      PC3				    /*pin A3 */
#define RESET_PIN   PC4                 /*pin A4 */

// Touch detector lines
#define YP_PIN 		PC3  	// must be an analog
#define XM_PIN 		PC2  	// must be an analog
#define YM_PIN 		PB1   	// can be a digital pin
#define XP_PIN 		PB0   	// can be a digital pin

#define YP_PORT		PORTC
#define XM_PORT		PORTC
#define YM_PORT		PORTB
#define XP_PORT		PORTB

/* Heater relay pin */
#define OUT_PORT	PORTB
#define OUT_PIN		PB5

/* UART pins PD0 and PD1 are left unused */

#endif  //_HW_CFG_

