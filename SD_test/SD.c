/*
 * SD.c
 *
 *  Created on: Jul 2, 2012
 *      Author: Andrew Reed
 */

//function prototypes not used outside of SD.c
#include "SD.h"
#include "uart.h"

volatile unsigned char SDHC_flag, cardType;

#define CS BIT4         // Chip Select line
#define MOSI BIT7       // Master-out Slave-in
#define MISO BIT6
#define SCK BIT5        // Serial clock

unsigned char spiSendByte(const unsigned char data);

unsigned char spiSendByte(const unsigned char data)
{
  while (!(IFG2 & UCB0TXIFG));              // Wait for RX to finish
  UCB0TXBUF=data;
  //USCICNT = 8;                             // send 8 bits of data
  while (!(IFG2 & UCB0TXIFG));              // Wait for RX to finish
  return (UCB0RXBUF);                     // Store data
}

void SPI_init(void) {
	/* recommended procedure: set UCSWRST, configure USCI, configure ports, activate */

		  /* UCSWRST */
		  UCB0CTL1 = UCSWRST;

		  /* USCI B0 */
		  UCB0CTL0 |= UCCKPL + UCMSB + UCMST + UCSYNC; // 3-pin, 8-bit SPI master
		  UCB0CTL1 |= UCSSEL_2; // SMCLK
		  UCB0BR0 |= 40; // 16,000,000 / 40 = 400kHz (required init speed)
		  UCB0BR1 = 0;
		  //UCB0MCTL = 0;

		  /* Ports */
		  P1SEL |= SCK + MOSI + MISO;
		  P1SEL2 |= SCK + MOSI + MISO;
		  P1DIR |= MOSI | SCK;
		  IE2 |= UCB0RXIE;
		  /* activate */
		  UCB0CTL1 &= ~UCSWRST;

}
unsigned char SD_init(void)
{
	unsigned char i, response, SD_version;
	unsigned int retry=0 ;
	CS_DESELECT();
	for(i=0;i<11;i++)
	    spiSendByte(0xff);//must wait 80 clock cycles with all bits high before sending first cmd
	CS_SELECT();
	do
	{

	   response = SD_sendCommand(GO_IDLE_STATE, 0, 0x95); //send 'reset & go idle' command
	   retry++;
	   if(retry>0x20)
	   	  return 1;   //time out, card not detected

	} while(response != 0x01);
	CS_DESELECT();
	spiSendByte(0xff);
	spiSendByte(0xff);
	retry = 0;

	SD_version = 2; //default set to SD compliance with ver2.x;
					//this may change after checking the next command
	do
	{
	response = SD_sendCommand(SEND_IF_COND,0x000001AA,0x87); //Check power supply status, mendatory for SDHC card
	retry++;
	if(retry>0xfe)
	   {
		  SD_version = 1;
		  cardType = 1;
		  break;
	   } //time out

	}while(response != 0x01);

	retry = 0;
	do
	{
	response = SD_sendCommand(APP_CMD,0, 0x95); //CMD55, must be sent before sending any ACMD command
	response = SD_sendCommand(SD_SEND_OP_COND,0x40000000, 0x95); //ACMD41

	retry++;
	if(retry>0xfe)
	   {
		  return 2;  //time out, card initialization failed
	   }

	}while(response != 0x00);


	retry = 0;
	SDHC_flag = 0;

	if (SD_version == 2)
	{
	   do
	   {
		 response = SD_sendCommand(READ_OCR,0, 0x95);
		 retry++;
		 if(retry>0xfe)
	     {
		   cardType = 0;
		   break;
	     } //time out

	   }while(response != 0x00);

	   if(SDHC_flag == 1) cardType = 2;
	   else cardType = 3;
	}

	return 0; //successful return
}

unsigned char SD_receiveByte(void)
{
	return spiSendByte(0xff);
}

unsigned char SD_sendCommand(const char cmd, unsigned long data, const char crc)
{
	unsigned char response, retry=0, status;
	char frame[6];
	char temp;
	int i;
	CS_SELECT();
	frame[0]=(cmd|0x40);
	for(i=3;i>=0;i--){
	  temp=(char)(data>>(8*i));
	  frame[4-i]=(temp);
	}
	frame[5]=(crc);
	for(i=0;i<6;i++)
	   spiSendByte(frame[i]);
	while((response = SD_receiveByte()) == 0xff)
		if(retry++ > 0xfe) break; //timeout
	//checking if card is SDHC
	if(response == 0x00 && cmd == 58)
	{
		status = SD_receiveByte() & 0x40;
		if(status == 0x40) SDHC_flag = 1;
		else SDHC_flag = 0;
		//ignore last three bytes
		SD_receiveByte();
		SD_receiveByte();
		SD_receiveByte();
	}
	SD_receiveByte(); //wait 8 cycles
	CS_DESELECT();
	return response;
}



