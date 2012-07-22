/* Andrew Reed and Sage Pujji
 * DeVry 2012
 * Senior Project
 * Serial Test #1
 * Project: Multipass
 * Date: 5/4/2012
 *
 * Testing the UART and SPI, SD Card interfaces with MSP430F5419A
 *
 * Should initialize the SDHC card and send initialization text to UART.
 *
 */

#include "msp430f5419a.h"
#include "UART_5419A.h"
#include "SD_5419A.h"
#include "string.h"


int i = 0;


int status = 0;

unsigned long size;
unsigned char error;

//extern unsigned char cardType;
//extern unsigned char buffer[512];

unsigned int timeout = 0;

void main(void) {
	WDTCTL = WDTPW+WDTHOLD;                   // Stop WDT
	/****** Initialize Clock to 8MHz ********/
	P11DIR |= 0x07;                           // ACLK, MCLK, SMCLK set out to pins
	P11SEL |= 0x07;                           // P11.0,1,2 for debugging purposes.

    UCSCTL3 |= SELREF_2;                      // Set DCO FLL reference = REFO
	UCSCTL4 |= SELA_2;                        // Set ACLK = REFO

	__bis_SR_register(SCG0);                  // Disable the FLL control loop
	UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
	UCSCTL1 = DCORSEL_5;                      // Select DCO range 16MHz operation
	UCSCTL2 = FLLD_1 + 249;                   // Set DCO Multiplier for 8MHz
	                                            // (N + 1) * FLLRef = Fdco
	                                            // (249 + 1) * 32768 = 8MHz
	                                            // Set FLL Div = fDCOCLK/2
	__bic_SR_register(SCG0);                  // Enable the FLL control loop

	// Worst-case settling time for the DCO when the DCO range bits have been
	// changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
	// UG for optimization.
	// 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
	__delay_cycles(250000);

	// Loop until XT1,XT2 & DCO fault flag is cleared
	do
	{
	  UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
	                                            // Clear XT2,XT1,DCO fault flags
	  SFRIFG1 &= ~OFIFG;                      // Clear fault flags
	}while (SFRIFG1&OFIFG);                   // Test oscillator fault flag


	uart_init();
	P1DIR  = BIT0;  //enable LED P1.0 as output
	//P1DIR |= BIT5;
	//P1SEL |= BIT5;   // output SMCLK for measurement on P1.5
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

            P1IE |= BIT3;    //re-enable interrupts for P1.3

            return;
        default:
            P1IFG = 0;    //clears interrupt flag is other interrupt occurs
            return;
    }
} // P1_ISR
