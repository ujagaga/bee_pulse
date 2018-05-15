#include "LCD.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "reg_helper.h"
#include "GFX.h"

#define MIPI_DCS_REV1   (1<<0)
#define AUTO_READINC    (1<<1)
#define READ_BGR        (1<<2)
#define READ_LOWHIGH    (1<<3)
#define READ_24BITS     (1<<4)
#define XSA_XEA_16BIT   (1<<5)
#define READ_NODUMMY    (1<<6)
#define INVERT_GS       (1<<8)
#define INVERT_SS       (1<<9)
#define MV_AXIS         (1<<10)
#define INVERT_RGB      (1<<11)
#define REV_SCREEN      (1<<12)
#define FLIP_VERT       (1<<13)
#define FLIP_HORIZ      (1<<14)

#define TFTLCD_DELAY 0xFFFF
#define TFTLCD_DELAY8 0xFF

/* Forward declarations */
static uint16_t TFTLCD_readReg(uint16_t reg);
static uint32_t TFTLCD_readReg32(uint16_t reg);
static void TFTLCD_setAddrWindow(int16_t x, int16_t y, int16_t x1, int16_t y1);

uint16_t _MC, _MP, _MW, _SC, _EC, _SP, _EP;
static uint8_t done_reset;

#if (DRIVER_ID==9341)
static const uint8_t regValues[] PROGMEM = {        // BOE 2.4"
	0x01, 0,            // software reset
	TFTLCD_DELAY8, 50,  // .kbv will power up with ONLY reset, sleep out, display on
	0x28, 0,            //Display Off
	0xF6, 3, 0x01, 0x01, 0x00,  //Interface Control needs EXTC=1 MV_EOR=0, TM=0, RIM=0
	0xCF, 3, 0x00, 0x81, 0x30,  //Power Control B [00 81 30]
	0xED, 4, 0x64, 0x03, 0x12, 0x81,    //Power On Seq [55 01 23 01]
	0xE8, 3, 0x85, 0x10, 0x78,  //Driver Timing A [04 11 7A]
	0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,      //Power Control A [39 2C 00 34 02]
	0xF7, 1, 0x20,      //Pump Ratio [10]
	0xEA, 2, 0x00, 0x00,        //Driver Timing B [66 00]
	0xB0, 1, 0x00,      //RGB Signal [00]
	0xB1, 2, 0x00, 0x1B,        //Frame Control [00 1B]
	//            0xB6, 2, 0x0A, 0xA2, 0x27, //Display Function [0A 82 27 XX]    .kbv SS=1
	0xB4, 1, 0x00,      //Inversion Control [02] .kbv NLA=1, NLB=1, NLC=1
	0xC0, 1, 0x21,      //Power Control 1 [26]
	0xC1, 1, 0x11,      //Power Control 2 [00]
	0xC5, 2, 0x3F, 0x3C,        //VCOM 1 [31 3C]
	0xC7, 1, 0xB5,      //VCOM 2 [C0]
	0x36, 1, 0x48,      //Memory Access [00]
	0xF2, 1, 0x00,      //Enable 3G [02]
	0x26, 1, 0x01,      //Gamma Set [01]
	0xE0, 15, 0x0f, 0x26, 0x24, 0x0b, 0x0e, 0x09, 0x54, 0xa8, 0x46, 0x0c, 0x17, 0x09, 0x0f, 0x07, 0x00,
	0xE1, 15, 0x00, 0x19, 0x1b, 0x04, 0x10, 0x07, 0x2a, 0x47, 0x39, 0x03, 0x06, 0x06, 0x30, 0x38, 0x0f,
	0x11, 0,            //Sleep Out
	TFTLCD_DELAY8, 150,
	0x29, 0,            //Display On
	0x3A, 1, 0x55,      //Pixel Format [66]
};
#elif (DRIVER_ID==6767)
static const uint8_t regValues[] PROGMEM = {
	0xEA, 2, 0x00, 0x20,        //PTBA[15:0]
	0xEC, 2, 0x0C, 0xC4,        //STBA[15:0]
	0xE8, 1, 0x38,      //OPON[7:0]
	0xE9, 1, 0x10,      //OPON1[7:0]
	0xF1, 1, 0x01,      //OTPS1B
	0xF2, 1, 0x10,      //GEN
	//Gamma 2.2 Setting
	0x40, 13, 0x01, 0x00, 0x00, 0x10, 0x0E, 0x24, 0x04, 0x50, 0x02, 0x13, 0x19, 0x19, 0x16,
	0x50, 14, 0x1B, 0x31, 0x2F, 0x3F, 0x3F, 0x3E, 0x2F, 0x7B, 0x09, 0x06, 0x06, 0x0C, 0x1D, 0xCC,
	//Power Voltage Setting
	0x1B, 1, 0x1B,      //VRH=4.65V
	0x1A, 1, 0x01,      //BT (VGH~15V,VGL~-10V,DDVDH~5V)
	0x24, 1, 0x2F,      //VMH(VCOM High voltage ~3.2V)
	0x25, 1, 0x57,      //VML(VCOM Low voltage -1.2V)
	//****VCOM offset**///
	0x23, 1, 0x88,      //for Flicker adjust //can reload from OTP
	//Power on Setting
	0x18, 1, 0x34,      //I/P_RADJ,N/P_RADJ, Normal mode 60Hz
	0x19, 1, 0x01,      //OSC_EN='1', start Osc
	0x01, 1, 0x00,      //DP_STB='0', out deep sleep
	0x1F, 1, 0x88,      // GAS=1, VOMG=00, PON=0, DK=1, XDK=0, DVDH_TRI=0, STB=0
	TFTLCD_DELAY8, 5,
	0x1F, 1, 0x80,      // GAS=1, VOMG=00, PON=0, DK=0, XDK=0, DVDH_TRI=0, STB=0
	TFTLCD_DELAY8, 3,
	0x1F, 1, 0x90,      // GAS=1, VOMG=00, PON=1, DK=0, XDK=0, DVDH_TRI=0, STB=0
	TFTLCD_DELAY8, 5,
	0x1F, 1, 0xD0,      // GAS=1, VOMG=10, PON=1, DK=0, XDK=0, DDVDH_TRI=0, STB=0
	TFTLCD_DELAY8, 5,
	//262k/65k color selection
	0x17, 1, 0x05,      //default 0x06 262k color // 0x05 65k color
	//SET PANEL
	0x36, 1, 0x00,      //SS_P, GS_P,REV_P,BGR_P
	//Display ON Setting
	0x28, 1, 0x38,      //GON=1, DTE=1, D=1000
	TFTLCD_DELAY8, 40,
	0x28, 1, 0x3F,      //GON=1, DTE=1, D=1100

	0x16, 1, 0x18,
};
#else
#error ********** Unsupported LCD ID ************
#endif

void wait_ms(uint16_t ms){
	uint8_t i = ms;

	while(i > 0){
		_delay_us(900);
		i--;
	}
}


static void WriteCmdData(uint16_t cmd, uint16_t dat)
{
    CS_ACTIVE();
    WriteCmd(cmd);
    WriteData(dat);
    CS_IDLE();
}

static void WriteCmdParamN(uint16_t cmd, int8_t N, uint8_t * block)
{
    CS_ACTIVE();
    WriteCmd(cmd);
    while (N-- > 0) {
        uint8_t u8 = *block++;
        CD_DATA();
        write8(u8);
#if (DRIVER_ID==6767)
        cmd++;
        WriteCmd(cmd);
#endif
    }
    CS_IDLE();
}

static inline void WriteCmdParam4(uint8_t cmd, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4)
{
    uint8_t d[4];
    d[0] = d1, d[1] = d2, d[2] = d3, d[3] = d4;
    WriteCmdParamN(cmd, 4, d);
}



static uint16_t read16bits(void)
{
    uint16_t ret;
    uint8_t lo;

    READ_8(ret);
    //all MIPI_DCS_REV1 style params are 8-bit
    READ_8(lo);
    return (ret << 8) | lo;
}

static uint32_t readReg40(uint16_t reg)
{
    uint16_t h, m, l;
    CS_ACTIVE();
    WriteCmd(reg);
    setReadDir();
    CD_DATA();
    h = read16bits();
    m = read16bits();
    l = read16bits();
    RD_IDLE();
    CS_IDLE();
    setWriteDir();
    return ((uint32_t) h << 24) | (m << 8) | (l >> 8);
}

static uint16_t TFTLCD_readReg(uint16_t reg)
{
    uint16_t ret;

    if (!done_reset)
        TFTLCD_reset();
    CS_ACTIVE();
    WriteCmd(reg);
    setReadDir();
    CD_DATA();
    //    READ_16(ret);
    ret = read16bits();
    RD_IDLE();
    CS_IDLE();
    setWriteDir();
    return ret;
}

static uint32_t TFTLCD_readReg32(uint16_t reg)
{
    uint16_t h, l;
    CS_ACTIVE();
    WriteCmd(reg);
    setReadDir();
    CD_DATA();
    h = read16bits();
    l = read16bits();
    RD_IDLE();
    CS_IDLE();
    setWriteDir();
    return ((uint32_t) h << 16) | (l);
}
#ifdef SUPPORT_READ_PIXEL

#if (DRIVER_ID==9341)
static uint16_t TFTLCD_color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}
#endif

 // independent cursor and window registers.   S6D0154, ST7781 increments.  ILI92320/5 do not.
static int16_t TFTLCD_readGRAM(int16_t x, int16_t y, uint16_t * block, int16_t w, int16_t h)
{
    uint16_t ret, _MR = _MW;
    int16_t n = w * h, row = 0, col = 0;
    uint8_t r;

#if (DRIVER_ID==9341)
     uint8_t g, b;
    _MR = 0x2E;
#elif (DRIVER_ID==6767)
    /* TEST THIS */
#endif

    TFTLCD_setAddrWindow(x, y, x + w - 1, y + h - 1);
    while (n > 0) {
        CS_ACTIVE();
        WriteCmd(_MR);
        setReadDir();
        CD_DATA();

        READ_8(r);

        while (n) {
#if (DRIVER_ID==9341)
        	READ_8(r);
			READ_8(g);
			READ_8(b);
			ret = TFTLCD_color565(r, g, b);
#elif (DRIVER_ID==6767)
			READ_16(ret);
#endif
			*block++ = ret;
			n--;

        }
        if (++col >= w) {
            col = 0;
            if (++row >= h)
                row = 0;
        }
        RD_IDLE();
        CS_IDLE();
        setWriteDir();
    }

    return 0;
}

uint16_t TFTLCD_readPixel(int16_t x, int16_t y) {
	uint16_t color;
	TFTLCD_readGRAM(x, y, &color, 1, 1);
	return color;
}

#endif

#ifdef SUPPORT_VERT_SCROLL

void TFTLCD_vertScroll(int16_t top, int16_t scrollines, int16_t offset)
{

    int16_t bfa = TFTHEIGHT - top - scrollines;  // bottom fixed area
    int16_t vsp;

    if (offset <= -scrollines || offset >= scrollines) offset = 0; //valid scroll
	vsp = top + offset; // vertical start position
    if (offset < 0)
        vsp += scrollines;          //keep in unsigned range


	uint8_t d[6];           // for multi-byte parameters

	d[0] = top >> 8;        //TFA
	d[1] = top;
	d[2] = scrollines >> 8; //VSA
	d[3] = scrollines;
	d[4] = bfa >> 8;        //BFA
	d[5] = bfa;

#if (DRIVER_ID==9341)
	WriteCmdParamN(0x33, 6, d);
#elif (DRIVER_ID==6767)
	WriteCmdParamN(0x0E, 6, d);
#endif

	d[0] = vsp >> 8;        //VSP
	d[1] = vsp;

#if (DRIVER_ID==9341)
	WriteCmdParamN(0x37, 2, d);
	if (offset == 0) {
		uint8_t dummy = 0;
		WriteCmdParamN(0x13, 0, &dummy);    //NORMAL i.e. disable scroll
	}
#elif (DRIVER_ID==6767)
	WriteCmdParamN(0x14, 2, d);
	d[0] = (offset != 0) ? 0x02 : 0;
	WriteCmdParamN(0x18, 1, d);
#endif
	return;
}
#endif

void TFTLCD_setRotation(uint8_t r)
{
    uint8_t val;

    switch (r & 3) {
    case 0:                    //PORTRAIT:
        val = 0x48;             //MY=0, MX=1, MV=0, ML=0, BGR=1
        break;
    case 1:                    //LANDSCAPE: 90 degrees
        val = 0x28;             //MY=0, MX=0, MV=1, ML=0, BGR=1
        break;
    case 2:                    //PORTRAIT_REV: 180 degrees
        val = 0x98;             //MY=1, MX=0, MV=0, ML=1, BGR=1
        break;
    case 3:                    //LANDSCAPE_REV: 270 degrees
        val = 0xF8;             //MY=1, MX=1, MV=1, ML=1, BGR=1
        break;
    }
#if (DRIVER_ID==9341)
    _MC = 0x2A; _MP = 0x2B; _MW = 0x2C; _SC = 0x2A; _EC = 0x2A; _SP = 0x2B; _EP = 0x2B;
    WriteCmdParamN(0x36, 1, &val);
#elif (DRIVER_ID==6767)
    _MC = 0x02, _MP = 0x06, _MW = 0x22, _SC = 0x02, _EC = 0x04, _SP = 0x06, _EP = 0x08;
    WriteCmdParamN(0x16, 1, &val);
#endif

	int16_t width = GFX_getWidth();
	int16_t height = GFX_getHeight();
	TFTLCD_setAddrWindow(0, 0, width - 1, height - 1);
	TFTLCD_vertScroll(0, TFTHEIGHT, 0);   //reset scrolling after a rotation
}


static void TFTLCD_setAddrWindow(int16_t x, int16_t y, int16_t x1, int16_t y1)
{
	WriteCmdParam4(_MC, x >> 8, x, x1 >> 8, x1);
	WriteCmdParam4(_MP, y >> 8, y, y1 >> 8, y1);
}

/* Initializes the display
 * */
void TFTLCD_begin()
{
    TFTLCD_reset();

	uint8_t *p = (uint8_t *) regValues;
	uint8_t dat[16];
	int16_t size = sizeof(regValues);

	while (size > 0) {
		uint8_t cmd = pgm_read_byte(p++);
		uint8_t len = pgm_read_byte(p++);
		if (cmd == TFTLCD_DELAY8) {
			wait_ms(len);
			len = 0;
		} else {
			for (uint8_t i = 0; i < len; i++)
				dat[i] = pgm_read_byte(p++);
			WriteCmdParamN(cmd, len, dat);
		}
		size -= len + 2;
	}
}


void TFTLCD_reset(void)
{
    done_reset = 1;
    setWriteDir();
    CTL_INIT();
    CS_IDLE();
    RD_IDLE();
    WR_IDLE();
    RESET_IDLE();
    wait_ms(50);
    RESET_ACTIVE();
    wait_ms(100);
    RESET_IDLE();
    wait_ms(100);
    WriteCmdData(0xB0, 0x0000);   //R61520 needs this to read ID
}


uint16_t TFTLCD_readID(void)
{
    uint8_t msb;
    uint16_t ret;

    ret = TFTLCD_readReg(0);           //forces a reset() if called before begin()
    if (ret == 0x5408)          //the SPFD5408 fails the 0xD3D3 test.
        return 0x5408;
    if (ret == 0x5420)          //the SPFD5420 fails the 0xD3D3 test.
        return 0x5420;
    if (ret == 0x8989)          //SSD1289 is always 8989
        return 0x1289;

    ret = TFTLCD_readReg(0x67);        //HX8347-A
    if (ret == 0x4747)
        return 0x8347;

	ret = readReg40(0xBF);
                                //HX8357B: [xx 01 62 83 57 FF] unsupported
	if (ret == 0x9481)          //ILI9481: [xx 02 04 94 81 FF]
        return 0x9481;
    if (ret == 0x1511)          //?R61511: [xx 02 04 15 11] not tested yet
        return 0x1511;
    if (ret == 0x1520)          //?R61520: [xx 01 22 15 20]
        return 0x1520;
    if (ret == 0x1581)          //R61581:  [xx 01 22 15 81]
        return 0x1581;
    if (ret == 0x1400)          //?RM68140:[xx FF 68 14 00] not tested yet
        return 0x6814;

    ret = readReg40(0xEF);      //ILI9327: [xx 02 04 93 27 FF]
    if (ret == 0x9327)
        return 0x9327;

    ret = TFTLCD_readReg32(0x04);      //ST7789V: [85 85 52]
    if (ret == 0x8000)          //HX8357-D
        return 0x8357;
    if (ret == 0x8552)
        return 0x7789;

    ret = TFTLCD_readReg32(0xD3);      //for ILI9488, 9486, 9340, 9341
    msb = ret >> 8;
    if (msb == 0x93 || msb == 0x94)
        return ret;             //0x9488, 9486, 9340, 9341
    if (ret == 0x00D3 || ret == 0xD3D3)
        return ret;             //16-bit write-only bus

	return TFTLCD_readReg(0);          //0154, 7783, 9320, 9325, 9335, B505, B509
}


void TFTLCD_drawPixel(int16_t x, int16_t y, uint16_t color)
{
	int16_t width = GFX_getWidth();
	int16_t height = GFX_getHeight();

    if (x < 0 || y < 0 || x >= width || y >= height)
        return;

	WriteCmdParam4(_MC, x >> 8, x, x >> 8, x);
	WriteCmdParam4(_MP, y >> 8, y, y >> 8, y);

    WriteCmdData(_MW, color);
}


void TFTLCD_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	int16_t width = GFX_getWidth();
	int16_t height = GFX_getHeight();

    int16_t end;
    if (w < 0) {
        w = -w;
        x -= w;
    }                           //+ve w
    end = x + w;
    if (x < 0)
        x = 0;
    if (end > width)
        end = width;
    w = end - x;
    if (h < 0) {
        h = -h;
        y -= h;
    }                           //+ve h
    end = y + h;
    if (y < 0)
        y = 0;
    if (end > height)
        end = height;
    h = end - y;

    TFTLCD_setAddrWindow(x, y, x + w - 1, y + h - 1);
    CS_ACTIVE();
    WriteCmd(_MW);
    if (h > w) {
        end = h;
        h = w;
        w = end;
    }
    uint8_t hi = color >> 8, lo = color & 0xFF;
    CD_DATA();
    while (h-- > 0) {
        end = w;

        do {
            write8(hi);
            write8(lo);
        } while (--end != 0);

    }
    CS_IDLE();
}
