/*
 * UART_5419A.h
 *
 *  Created on: Jul 22, 2012
 *      Author: Andrew Reed
 */

#ifndef UART_5419A_H_
#define UART_5419A_H_

#include "msp430f5419a.h"

void uart_init(void); //initialize the UARTA0 hardware

void uart_set_rx_isr_ptr(void (*isr_ptr)(unsigned char c));

void TX_char(char c); //send char over UART

void TX_str(char * str); //send string over UART

unsigned char uart_getc();

void TX_file(char * file);


#endif /* UART_5419A_H_ */
