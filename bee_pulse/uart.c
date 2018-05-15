/*
 * uart.c
 *
 *  Created on: Dec 30, 2016
 *      Author: rada
 */

#include "uart.h"
#include <avr/io.h>
#include <stdint.h>


void UART_init(void){
	DDRD |= (1 << PD1);
	UBRR0L = (unsigned char)BAUD_PRESCALE;
	UBRR0H = (BAUD_PRESCALE >> 8);
	UCSR0B = (1<<TXEN0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);   /* 8 data bits, 1 stop bit */
}

void UART_printStr(char *str)
{
	uint8_t i = 0;

	while (str[i] != '\0')
	{
		UART_printByte(str[i]);
		str++;
	}
}

void UART_printDec(uint16_t number){
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

	UART_printStr(snum);
}

void UART_printHex(uint16_t number){
	char snum[5] = {0};
	uint8_t temp, i;

	for(i = 0; i < 4; i++){
		temp = (number >> (12 - (i * 4))) & 0x0F;
		if(temp < 10){
			snum[i] = temp + '0';
		}else{
			snum[i] = temp - 10 + 'A';
		}
	}

	UART_printStr(snum);
}
