/* Andrew Reed and Sage Pujji
 * DeVry 2012
 * Senior Project
 * Serial Test #1
 * Project: Multipass
 * Date: 5/4/2012
 *
 * Testing the Hardware UART TX/RX of the MSP430G2553 w/ Launchpad
 *
 * Should send the contents of "str" when the button is pressed and the return to LPM
 *
 */

#include "msp430g2553.h"
#include "uart.h"
#include "SD.h"
#include "string.h"


int i = 0;


int status = 0;

unsigned long size;
unsigned char error;

extern unsigned char cardType;

unsigned int timeout = 0;

void main(void) {
	WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
	BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;


	uart_init();

	P1DIR  = BIT0;  //enable LED P1.0 as output
	P1DIR |= BIT5;
	P1SEL |= BIT5;   // output SMCLK for measurement on P1.5
	P1OUT &= ~BIT0;
	P1REN |= BIT3;	//enable internal pull-up resistor
	P1OUT |= BIT3;
	P1IES |= BIT3;   // high -> low is selected with IESx = 1.
	P1IFG &= ~BIT3;  // To prevent an immediate interrupt, clear the flag for
			                 // P1.3 before enabling the interrupt.
		P1IE |= BIT3;    // Enable interrupts for P1.3

	TX_str("Initialized the UART--------\n");
	SPI_init();
	TX_str("Initialized the SPI........\n");

	cardType = 0;

	for (i=0; i<10; i++)
	{
	  error = SD_init();
	  if(!error) break;
	}

	if(error)
	{
	  if(error == 1) TX_str("SD card not detected\n");
	  if(error == 2) TX_str("Card Initialization failed..\n");

	  while(1);  //wait here forever if error in SD init
	}
	TX_str("SD/MMC Card Initialized Successfully!\n");
	switch (cardType)
	{
	  case 1:TX_str("Standard Capacity Card (Ver 1.x) Detected!\n");
	  		 break;
	  case 2:TX_str("High Capacity Card Detected! (SDHC)\n");
	  		 break;
	  case 3:TX_str("Standard Capacity Card (Ver 2.x) Detected!\n");
	  		 break;
	  default:TX_str("Unknown SD Card Detected!\n");
	  		 break;
	}


	  //while ((mmc_ping() != MMC_SUCCESS));      // Wait till card is inserted


	__delay_cycles(100);

	//uart_set_rx_isr_ptr(uart_rx_isr);
	//Enter low power mode with interrupts enabled
	__bis_SR_register(LPM4_bits + GIE ) ;
}

#pragma vector = PORT1_VECTOR
__interrupt void P1_ISR(void) {
    switch(P1IFG&BIT3) {
        case BIT3:
        	P1IFG &= ~BIT3;    // clear the interrupt flag
        	P1IE |= ~BIT3;    // disable interrupts for P1.3


        	if(!error)
        	{
        		TX_str("SD Card has been initialized!\n");
        		switch (cardType)
        		{
        			case 1:TX_str("Standard Capacity Card (Ver 1.x) Detected!\n");
        			  	   break;
        			case 2:TX_str("High Capacity Card Detected! (SDHC)\n");
        			  	   break;
        			case 3:TX_str("Standard Capacity Card (Ver 2.x) Detected!\n");
        			  	   break;
        			default:TX_str("Unknown SD Card Detected!\n");
        			  	    break;
        		}
        	}
        	if(error)
        	{
        	  if(error == 1) TX_str("SD card not detected..\n");
        	  if(error == 2) TX_str("Card Initialization failed..\n");
        	}

        	/*if(mmc_ping()==0x00)
        	{
        		TX_str("Successfully Pinged the Card!");
        	} else {
        		TX_str("There was an error");
        	}
        	size = MMC_ReadCardSize();
        	TX_char((char)size+0x30);*/

            P1IE |= BIT3;    //re-enable interrupts for P1.3

            return;
        default:
            P1IFG = 0;    //clears interrupt flag is other interrupt occurs
            return;
    }
} // P1_ISR
