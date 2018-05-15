/*
 * main.c
 *
 *  Created on: Dec 1, 2017
 *      Author: ujagaga
 */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "reg_helper.h"
#include "GFX.h"
#include "TouchScreen.h"
#include "main.h"

#include "time_ctrl.h"
#include "uart.h"

#define ON		true
#define OFF		false

uint16_t main_ID;
uint16_t setTempX;
uint16_t setTempY;
uint16_t statusX;
uint16_t statusY;

int16_t avgTemp = 0;
gfx_btn btnUp = {0};
gfx_btn btnDown = {0};
gfx_btn btnStatus = {0};
uint32_t dsErrTime = 0;
uint32_t upBtnTime = 0;
uint32_t downBtnTime = 0;
uint32_t reqTempChangeTime = 0;
bool lastState;

uint8_t requestedPulse;
uint8_t requestedPause;
uint8_t EEMEM savedRequestedPulse = 10;	/* in us */
uint8_t EEMEM savedRequestedPause = 3;	/* in ms */

static void drawRequestedTemp(void){
	GFX_setCursor(setTempX, setTempY);
	GFX_setTextSize(3);
	GFX_setTextColor(BLUE, SCR_CLR);
	GFX_printDec(requestedPulse/10);
	GFX_printStr(",");
	GFX_printDec(requestedPulse%10);
}

static void drawStatus(bool state){
	if(state == ON){
		btnStatus.fillcolor = RED;
		btnStatus.textcolor = YELLOW;
		btnStatus.label = "on  -///-";
	}else{
		btnStatus.fillcolor = BLACK;
		btnStatus.textcolor = RED;
		btnStatus.label = "off -///-";
	}
	GFX_btnDraw(&btnStatus, false);
}

static void refreshScr(void){
	/* Write labels */
	GFX_setTextColor(BLUE, SCR_CLR);
	GFX_setTextSize(2);
	GFX_setCursor(2, 60);
	GFX_printStr("Podeseno: ");
	GFX_setTextSize(3);
	setTempX = GFX_getCursorX();
	GFX_setCursor(setTempX, 60 - TXTH);
	setTempY = GFX_getCursorY();

	GFX_printDec(requestedPulse/10);
	GFX_printStr(",");
	GFX_printDec(requestedPulse%10);
	/* print degree celsius */
	GFX_printChar(247);
	GFX_printChar(67);
	GFX_printStr("\n");

	GFX_setTextSize(2);
	GFX_printStr("Srednja temperatura\n\n");
	GFX_setTextColor(YELLOW, SCR_CLR);

	if(avgTemp != 0){
		if(avgTemp == 0){
			GFX_setTextSize(4);
			GFX_printStr("GRESKA!\n");
			GFX_setTextSize(3);
			GFX_printStr("Proverite sondu.");
		}
		else if(avgTemp < 0){
			GFX_setTextSize(3);
			GFX_printStr("Temperatura\nispod nule.");
		}else{
			GFX_setCursor(GFX_getCursorX() + TXTW, GFX_getCursorY());
			GFX_setTextSize(10);
			GFX_printDec(avgTemp/10);
			GFX_setTextSize(7);
			GFX_setCursor(GFX_getCursorX(), GFX_getCursorY() + TXTH * 3);
			GFX_printStr(",");
			GFX_setCursor(GFX_getCursorX(), GFX_getCursorY() - TXTH * 3);
			GFX_setTextSize(10);
			GFX_printDec(avgTemp%10);
		}
	}
}

static void drawButtons(void){
	btnUp.fillcolor = BLUE;
	btnUp.label = "+";
	btnUp.outlinecolor = WHITE;
	btnUp.textcolor = BLACK;
	btnUp.textsize = 4;
	btnUp.x = GFX_getWidth() / 2;
	btnUp.y = 20;
	btnUp.width = GFX_getWidth();;
	btnUp.height = 40;
	GFX_btnDraw(&btnUp, false);

	btnDown.fillcolor = BLUE;
	btnDown.label = "-";
	btnDown.outlinecolor = WHITE;
	btnDown.textcolor = BLACK;
	btnDown.textsize = 4;
	btnDown.x = GFX_getWidth() / 2;
	btnDown.y = GFX_getHeight() - 20;
	btnDown.width = GFX_getWidth();;
	btnDown.height = 40;
	GFX_btnDraw(&btnDown, false);

	/* Replacement for LED */
	btnStatus.outlinecolor = RED;
	btnStatus.textsize = 3;
	btnStatus.x = GFX_getWidth() / 2;
	btnStatus.y = GFX_getHeight() - 70;
	btnStatus.width = GFX_getWidth() - 20;
	btnStatus.height = 40;
	drawStatus(ON);
}

static void setHeater(bool state){
	if(state == ON){
		PIN_HIGH(HEAT_PORT, HEAT_PIN);
	}else{
		PIN_LOW(HEAT_PORT, HEAT_PIN);
	}

	if(lastState != state){
		drawStatus(state);
	}

	lastState = state;
}

static void processButtons(TSPoint* touchPoint){

	if((TCTRL_micros() - upBtnTime) > 300000){
		GFX_btnUpdate(&btnDown, touchPoint);
	}/* else btnUp recently pressed so do not pass touch to btnDown */

	if((TCTRL_micros() - downBtnTime) > 300000){
		GFX_btnUpdate(&btnUp, touchPoint);
	}/* else btnDown recently pressed so do not pass touch to btnUp */

	if(GFX_btnJustPressed(&btnUp)){
		requestedPulse++;
		drawRequestedTemp();
		upBtnTime = TCTRL_micros();
		reqTempChangeTime = TCTRL_micros();
	}

	if(GFX_btnIsPressed(&btnUp)){
		if((TCTRL_micros() - upBtnTime) > 250000){
			upBtnTime = TCTRL_micros();
			requestedPulse += 10;
			drawRequestedTemp();
			reqTempChangeTime = TCTRL_micros();
		}
	}

	if(GFX_btnJustPressed(&btnDown)){
		requestedPulse--;
		drawRequestedTemp();
		downBtnTime = TCTRL_micros();
		reqTempChangeTime = TCTRL_micros();
	}

	if(GFX_btnIsPressed(&btnDown)){
		if((TCTRL_micros() - downBtnTime) > 250000){
			downBtnTime = TCTRL_micros();
			requestedPulse -= 10;
			drawRequestedTemp();
			reqTempChangeTime = TCTRL_micros();
		}
	}

	if(requestedPulse > 400){
		requestedPulse = 400;
		drawRequestedTemp();
	}else if(requestedPulse < 250){
		requestedPulse = 250;
		drawRequestedTemp();
	}

	if(reqTempChangeTime > 0){
		if((TCTRL_micros() - reqTempChangeTime) > 1000000){
			eeprom_update_byte(&savedRequestedPulse, requestedPulse);
			reqTempChangeTime = 0;
		}
	}
}

static void init( void ){
	UART_init();
	PIN_OUTPUT(OUT_PORT, OUT_PIN);

	requestedPulse = eeprom_read_word(&savedRequestedPulse);
	if(requestedPulse < 10){
		requestedPulse = 10;
	}else if(requestedPulse > 200){
		requestedPulse = 200;
	}

	requestedPause = eeprom_read_word(&savedRequestedPause);
	if(requestedPause < 3){
		requestedPause = 3;
	}else if(requestedPause > 100){
		requestedPause = 100;
	}

	main_ID = GFX_init();
	UART_printStr("\n\rLCD ID: 0x");
	UART_printHex(main_ID);

	TCTRL_start();

	GFX_setRotation(0);
	GFX_fillScreen(BLACK);
	GFX_setTextColor(GREEN, GREEN);

	drawButtons();
}

int main( void )
{
	TSPoint point;

	init();

	for(;;){

		TS_getPoint(&point);

//		/* Hack to compensate for broken touch which only detects Y coordinates. */
//		point.x = 120;
//		if(point.y < 0){
//			point.z = 0;
//		}else{
//			UART_printStr("\n\r y:");
//			UART_printDec((uint16_t)point.y);
//
//			point.z = 10;
//			if(point.y > 10){
//				if(point.y > 200){
//					point.y = 300;
//				}else if(point.y < 50){
//					point.y = 20;
//				}
//			}
//		}

		processButtons(&point);

	}

	return 0;
}
