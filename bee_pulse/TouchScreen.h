#ifndef _TOUCHSCREEN_H_
#define _TOUCHSCREEN_H_
#include <stdint.h>

typedef struct TSPoint_t{
	int16_t x;
	int16_t y;
	int16_t z;	/* Pressure readout */
}TSPoint;

extern void TS_getPoint(TSPoint *result);

#endif
