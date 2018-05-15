#ifndef _REG_HELPER_H_
#define _REG_HELPER_H_

#include <avr/io.h>
#include "config.h"

#define PIN_LOW(p, b)        	(p) &= ~(1<<(b))
#define PIN_HIGH(p, b)       	(p) |= (1<<(b))
#define PIN_OUTPUT(p, b)     	*(&p-1) |= (1<<(b))
#define PIN_INPUT(p, b)     	*(&p-1) &= ~(1<<(b))
#define PIN_READ(p, b)     		((*(&p-2) >> (b)) & 1)

#define YP_SET_INPUT()	PIN_INPUT(YP_PORT, YP_PIN)
#define YP_SET_OUTPUT()	PIN_OUTPUT(YP_PORT, YP_PIN)
#define YP_SET_LOW()	PIN_LOW(YP_PORT, YP_PIN)
#define YP_SET_HIGH()	PIN_HIGH(YP_PORT, YP_PIN)

#define XM_SET_INPUT()	PIN_INPUT(XM_PORT, XM_PIN)
#define XM_SET_OUTPUT()	PIN_OUTPUT(XM_PORT, XM_PIN)
#define XM_SET_LOW()	PIN_LOW(XM_PORT, XM_PIN)
#define XM_SET_HIGH()	PIN_HIGH(XM_PORT, XM_PIN)

#define YM_SET_INPUT()	PIN_INPUT(YM_PORT, YM_PIN)
#define YM_SET_OUTPUT()	PIN_OUTPUT(YM_PORT, YM_PIN)
#define YM_SET_LOW()	PIN_LOW(YM_PORT, YM_PIN)
#define YM_SET_HIGH()	PIN_HIGH(YM_PORT, YM_PIN)

#define XP_SET_INPUT()	PIN_INPUT(XP_PORT, XP_PIN)
#define XP_SET_OUTPUT()	PIN_OUTPUT(XP_PORT, XP_PIN)
#define XP_SET_LOW()	PIN_LOW(XP_PORT, XP_PIN)
#define XP_SET_HIGH()	PIN_HIGH(XP_PORT, XP_PIN)


#if (defined __AVR_ATmega328P__) || (defined __AVR_ATmega328__)
#define DMASK         0x03
#define NMASK         ~DMASK
#define write_8(x)    { PORTB = (PORTB & NMASK) | ((x) & DMASK); PORTD = (PORTD & DMASK) | ((x) & NMASK); }
#define read_8()      ( (PINB & DMASK) | (PIND & NMASK) )
#define setWriteDir() { DDRB = (DDRB & NMASK) | DMASK; DDRD = (DDRD & DMASK) | NMASK;  }
#define setReadDir()  { DDRB = (DDRB & NMASK) & NMASK; DDRD = (DDRD & DMASK) & DMASK;  }
#else
#error ***This configuration is for Arduino Uno only. Please configure macros for your platform.*****************
#endif

#define write8(x)     { write_8(x); WR_STROBE(); }
#define write16(x)    { uint8_t h = (x)>>8, l = x; write8(h); write8(l); }
#define READ_8(dst)   { RD_STROBE(); dst = read_8(); RD_IDLE(); }
#define READ_16(dst)  { uint8_t hi; READ_8(hi); READ_8(dst); dst |= (hi << 8); }

#define RD_ACTIVE()  	PIN_LOW(RD_PORT, RD_PIN)
#define RD_IDLE()    	PIN_HIGH(RD_PORT, RD_PIN)
#define RD_OUTPUT()  	PIN_OUTPUT(RD_PORT, RD_PIN)
#define WR_ACTIVE()  	PIN_LOW(WR_PORT, WR_PIN)
#define WR_IDLE()    	PIN_HIGH(WR_PORT, WR_PIN)
#define WR_OUTPUT()  	PIN_OUTPUT(WR_PORT, WR_PIN)
#define CD_COMMAND() 	PIN_LOW(CD_PORT, CD_PIN)
#define CD_DATA()    	PIN_HIGH(CD_PORT, CD_PIN)
#define CD_OUTPUT()  	PIN_OUTPUT(CD_PORT, CD_PIN)
#define CS_ACTIVE()  	PIN_LOW(CS_PORT, CS_PIN)
#define CS_IDLE()    	PIN_HIGH(CS_PORT, CS_PIN)
#define CS_OUTPUT()  	PIN_OUTPUT(CS_PORT, CS_PIN)
#define RESET_ACTIVE()  PIN_LOW(RESET_PORT, RESET_PIN)
#define RESET_IDLE()    PIN_HIGH(RESET_PORT, RESET_PIN)
#define RESET_OUTPUT()  PIN_OUTPUT(RESET_PORT, RESET_PIN)

 // General macros.   IOCLR registers are 1 cycle when optimised.
#define WR_STROBE() { WR_ACTIVE(); WR_IDLE(); }       //PWLW=TWRL=50ns
#define RD_STROBE() { RD_IDLE(); RD_ACTIVE(); RD_ACTIVE(); RD_ACTIVE();}      //PWLR=TRDL=150ns, tDDR=100ns

#define CTL_INIT()   { RD_OUTPUT(); WR_OUTPUT(); CD_OUTPUT(); CS_OUTPUT(); RESET_OUTPUT(); }
#define WriteCmd(x)  { CD_COMMAND(); write16(x); }
#define WriteData(x) { CD_DATA(); write16(x); }

#endif //_REG_HELPER_H_
