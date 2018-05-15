/*
 * uart.h
 *
 *  Created on: Dec 30, 2016
 *      Author: rada
 */

#ifndef UART_H_
#define UART_H_

#include <stdbool.h>
#include <stdint.h>

#define USART_BAUDRATE 	(115200)
#define BAUD_PRESCALE 	(((( F_CPU / 16) + ( USART_BAUDRATE / 2) ) / ( USART_BAUDRATE ) ) - 1)
#define waitTxReady()	while (( UCSR0A & (1 << UDRE0 ) ) == 0)

#define UART_printByte(data)	do{	waitTxReady();	UDR0 = data;}while(0)

extern void UART_init(void);
extern void UART_printStr(char* msg);
extern void UART_printDec(uint16_t number);
extern void UART_printHex(uint16_t number);

#endif /* UART_H_ */
