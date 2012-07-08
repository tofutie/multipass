#include "msp430g2553.h"

#define RXD	BIT1
#define TXD BIT2

void (*uart_rx_isr_ptr)(unsigned char c);

void uart_set_rx_isr_ptr(void (*isr_ptr)(unsigned char c))
{
	uart_rx_isr_ptr = isr_ptr;
}


void uart_init() {
		uart_set_rx_isr_ptr(0L);

		P1SEL  = RXD + TXD;
	  	P1SEL2 = RXD + TXD;
	  	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	  	UCA0BR0 = 130;                            // 1MHz 9600
	  	UCA0BR1 = 6;                              // 1MHz 9600

	  	//UCA0BR0 = 0xA0;
	  	//UCA0BR1 = 0x01;
	  	UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
	  	UCA0CTL1 &= ~UCSWRST;                     // Initialize USCI state machine
	  	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}



unsigned char uart_getc()
{
    while (!(IFG2&UCA0RXIFG));                // USCI_A0 RX buffer ready?
	return UCA0RXBUF;
}

void TX_char(char c) {
	while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
	//P1OUT ^= BIT0;
	P1OUT ^= BIT0;
	UCA0TXBUF = c;
}

void TX_str(char * string) {
	int i=0;
	while(string[i] != '\0') {
		while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
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
	//while(i<207)
	{
		while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
		UCA0TXBUF = file[i];
		//if((i%40)==0)
		//	P1OUT ^= BIT0;
		i++;
	}
	//UCA0TXBUF = 0x03;
}

#pragma vector = USCIAB0RX_VECTOR;
__interrupt void USCI0RX_ISR(void) {
	if(uart_rx_isr_ptr != 0L) {
		(uart_rx_isr_ptr)(UCA0RXBUF);
	}
}
