
#ifndef __UART_H
#define __UART_H

#include "msp430g2553.h"

void uart_init(void); //initialize the UART0 hardware

void uart_set_rx_isr_ptr(void (*isr_ptr)(unsigned char c));

void TX_char(char c); //send char over UART

void TX_str(char * str); //send string over UART

unsigned char uart_getc();

void TX_file(char * file);


#endif
