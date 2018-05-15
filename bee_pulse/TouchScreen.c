#include <avr/pgmspace.h>
#include <stdbool.h>
#include "TouchScreen.h"
#include "reg_helper.h"

#define MINPRESSURE 2
#define MAXPRESSURE 1000


static int analogRead(uint8_t pin) {
	uint8_t low, high;

	// set the analog reference (high two bits of ADMUX) and select the
	// channel (low 4 bits).  this also sets ADLAR (left-adjust result)
	// to 0 (the default).
	ADMUX = (1 << REFS0) | (pin & 0x07);

	// without a delay, we seem to read from the wrong channel
	//delay(1);

	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	// start the conversion
	ADCSRA |= (1<<ADSC);

	// ADSC is cleared when the conversion finishes
	while (ADCSRA & (1<<ADSC));          //wait for end of conversion

	// we have to read ADCL first; doing so locks both ADCL
	// and ADCH until ADCH is read.  reading ADCL second would
	// cause the results of each conversion to be discarded,
	// as ADCL and ADCH would be locked when it completed.
	low  = ADCL;
	high = ADCH;

	// combine the two bytes
	return (high << 8) | low;

}

static long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void TS_getPoint(TSPoint* result){
	int x, y, z;
	int samples[2];
	bool valid;

	valid = true;

	YP_SET_INPUT();
	YM_SET_INPUT();
	YP_SET_LOW();
	YM_SET_LOW();
	XP_SET_OUTPUT();
	XM_SET_OUTPUT();
	XP_SET_HIGH();
	XM_SET_LOW();

	samples[0] = analogRead(YP_PIN);
	samples[1] = analogRead(YP_PIN);

	if (samples[0] != samples[1]) {
		valid = false;
	}

	x = 1023-samples[1];

	XP_SET_INPUT();
	XM_SET_INPUT();
	XP_SET_LOW();
	YP_SET_OUTPUT();
	YP_SET_HIGH();
	YM_SET_OUTPUT();
  
	samples[0] = analogRead(XM_PIN);
	samples[1] = analogRead(XM_PIN);

	if (samples[0] != samples[1]) {
		valid = false;
	}

	y = 1023-samples[1];

	// Set X+ to ground
	XP_SET_OUTPUT();
	XP_SET_LOW();

	// Set Y- to VCC
	YM_SET_HIGH();
	// Hi-Z X- and Y+
	YP_SET_LOW();
	YP_SET_INPUT();

	if ((!valid)) {
		z = 0;
	}else{
		int z1 = analogRead(XM_PIN);
		int z2 = analogRead(YP_PIN);
		// now read the x
		z = 1023-(z2-z1);
	}

	if((z < MINPRESSURE) || (z > MAXPRESSURE)){
		z = 0;
	}

	result->x = map(x, TS_MINX, TS_MAXX, TFTWIDTH, 0);
	result->y = TFTHEIGHT - map(y, TS_MINY, TS_MAXY, TFTHEIGHT, 0);
	result->z = z;
	XM_SET_OUTPUT();
	YP_SET_OUTPUT();
}
