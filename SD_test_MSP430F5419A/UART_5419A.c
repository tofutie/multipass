/*
 * UART_5419A.c
 *
 *  Created on: Jul 22, 2012
 *  Author: Andrew Reed
 */

#include "msp430f5419a.h"

#define RXD 	BIT1
#define TXD    BIT2

void (*uart_rx_isr_ptr)(unsigned char c);

void uart_set_rx_isr_ptr(void (*isr_ptr)(unsigned char c))
{
	uart_rx_isr_ptr = isr_ptr;
}


void uart_init() {
		uart_set_rx_isr_ptr(0L);

		P3SEL  = RXD + TXD;
		UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
		UCA0CTL1 |= UCSSEL_2;                     // SMCLK
		UCA0BR0 = 69;                              // 8MHz 115200
		UCA0BR1 = 0;                              // 8MHz 115200
		UCA0MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
		UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
		UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}



unsigned char uart_getc()
{
	while (!(UCA0IFG&UCRXIFG));             // USCI_A0 RX buffer ready?
	return UCA0RXBUF;
}

void TX_char(char c) {
	while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
	P1OUT ^= BIT0;
	UCA0TXBUF = c;
}

void TX_str(char * string) {
	int i=0;
	while(string[i] != '\0') {
		while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
		if(string[i] == '\n')
		{
			TX_char(10);
			TX_char(13);
		}
		else
		{
		UCA0TXBUF = string[i];
		}
		i++;
	}
	//UCA0TXBUF = 0x03;
}

void TX_file(char * file) {
	int i=0;
	while(file[i] != 3)
	{
		while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
		UCA0TXBUF = file[i];
		i++;
	}
}

#pragma vector = USCI_A0_VECTOR;
__interrupt void USCI_A0_ISR(void) {
	if(uart_rx_isr_ptr != 0L) {
		(uart_rx_isr_ptr)(UCA0RXBUF);
	}
}


