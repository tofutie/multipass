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
#include "mmc.h"
#include "string.h"


int i = 0;
char first_name[21]="Andrew";
char last_name[21]="Reed";
char age[3] = "27";
char occupation[25] = "Electrical Engineer";
char last_school[25] = "DeVry College";
char graduation_date[11] = "10/31/2012";
char address[35] = "1224 St. Nicholas Ave Apt #3G";
char city[15] = "New York";
char state[3] = "NY";
char zipcode[6] = "10032";

char status = 0;
unsigned long size;


void uart_rx_isr(unsigned char c) {

	//TX_char(c);

	if(c != 3)
	{
		if(c != '\0')
		{
			switch(status)
			{
			case 0:
				first_name[i] = c;
				break;
			case 1:
				last_name[i] = c;
				break;
			case 2:
				age[i] = c;
				break;
			case 3:
				occupation[i] = c;
				break;
			case 4:
				last_school[i] = c;
				break;
			case 5:
				graduation_date[i] = c;
				break;
			case 6:
				address[i] = c;
				break;
			case 7:
				city[i] = c;
				break;
			case 8:
				state[i] = c;
				break;
			case 9:
				zipcode[i] = c;
				break;
			}
			i++;
		}
		else {
			switch(status)
			{
			case 0:
				first_name[i] = '\0';
				break;
			case 1:
				last_name[i] = '\0';
				break;
			case 2:
				age[i] = '\0';
				break;
			case 3:
				occupation[i] = '\0';
				break;
			case 4:
				last_school[i] = '\0';
				break;
			case 5:
				graduation_date[i] = '\0';
				break;
			case 6:
				address[i] = '\0';
				break;
			case 7:
				city[i] = '\0';
				break;
			case 8:
				state[i] = '\0';
				break;
			case 9:
				zipcode[i] = '\0';
				break;
			}
			i=0;
			status++;
			if(status > 9)
				status = 0;
		}
	} else {
		//temp[i]=3;
		//strcpy(str,temp);

		//TX_str("ACK");
		//TX_char(0x03);
		__delay_cycles(1000000);
		i = 0;
		status = 0;
	}



}

unsigned char status1 = 1;
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

	TX_str("Initialized the UART--------");
	MMC_initSPI();
	TX_str("Initialized the SPI........");
	//Initialisation of the MMC/SD-card
	  while (status1 != 0)                       // if return in not NULL an error did occur and the
	                                            // MMC/SD-card will be initialized again
	  {
	    status1 = initMMC();
	    TX_str("Didn't make it baaaaah");
	    timeout++;
	    if (timeout == 50)                      // Try 50 times till error
	    {
	       TX_str("No MMC/SD-card found");
	       TX_char(status1+0x30);
	      break;
	    }
	  }
	  if(timeout != 50)
	  {
		  TX_str("MMC/SD-card initialized!");
	  }


	  //while ((mmc_ping() != MMC_SUCCESS));      // Wait till card is inserted


	__delay_cycles(100);


	//if(mmc_ping()==0x00)
	//{
	 //   TX_str("Successfully Pinged the Card!");
	//} else {
	//    TX_str("There was an error");
	//}
	size = MMC_ReadCardSize();
	TX_char((char)size+0x30);



	//TX_str(str);
	uart_set_rx_isr_ptr(uart_rx_isr);
	//Enter low power mode with interrupts enabled
	__bis_SR_register(LPM4_bits + GIE ) ;
}

#pragma vector = PORT1_VECTOR
__interrupt void P1_ISR(void) {
    switch(P1IFG&BIT3) {
        case BIT3:
        	P1IFG &= ~BIT3;    // clear the interrupt flag
        	P1IE |= ~BIT3;    // disable interrupts for P1.3
            TX_str(first_name);
            TX_char(0x03);
            __delay_cycles(100);
            TX_str(last_name);
            TX_char(0x03);
            __delay_cycles(100);
            TX_str(age);
            TX_char(0x03);
            __delay_cycles(100);
            TX_str(occupation);
            TX_char(0x03);
            __delay_cycles(100);
            TX_str(last_school);
            TX_char(0x03);
            __delay_cycles(100);
            TX_str(graduation_date);
            TX_char(0x03);
            __delay_cycles(100);
            TX_str(address);
            TX_char(0x03);
            __delay_cycles(100);
            TX_str(city);
            TX_char(0x03);
            __delay_cycles(100);
            TX_str(state);
            TX_char(0x03);
            __delay_cycles(100);
            TX_str(zipcode);
            TX_char(0x03);
            TX_char(10);
            //TX_char(13);

            __delay_cycles(1000000);


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
