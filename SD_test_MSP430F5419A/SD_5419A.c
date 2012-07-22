/*
 * SD_5419A.c
 *
 *  Created on: Jul 22, 2012
 *      Author: andrew
 */
//function prototypes not used outside of SD.c
#include "SD_5419A.h"
#include "UART_5419A.h"



#define CS BIT6         // Chip Select line
#define MOSI BIT1       // Master-out Slave-in
#define MISO BIT2
#define SCK BIT3        // Serial clock

unsigned char spiSendByte(const unsigned char data);

unsigned char spiSendByte(const unsigned char data)
{
  while (!(UCB0IFG&UCRXIFG));           // Wait for RX to finish
  UCB0TXBUF=data;
  while (!(UCB0IFG&UCRXIFG));           // Wait for RX to finish
  return (UCB0RXBUF);                     // Store data
}

void SPI_init(void) {
	/* recommended procedure: set UCSWRST, configure USCI, configure ports, activate */

		  /* UCSWRST */
		  UCB0CTL1 = UCSWRST;

		  /* USCI B0 */
		  UCB0CTL0 |= UCCKPL + UCMSB + UCMST + UCSYNC; // 3-pin, 8-bit SPI master
		  UCB0CTL1 |= UCSSEL_2; // SMCLK
		  UCB0BR0 |= 20; // 8,000,000 / 20 = 400kHz (required init speed)
		  UCB0BR1 = 0;

		  /* Ports */
		  P3SEL |= SCK + MOSI + MISO;
		  P3DIR |= MOSI | SCK;
		  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
		  UCB0IE |= UCRXIE;                         // Enable USCI_B0 RX interrupt

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
/* SD_erase
 * This function erases SD from the startBlock until startBlock+totalBlocks
 */
unsigned char SD_erase (unsigned long startBlock, unsigned long totalBlocks)
{
  unsigned char response;

  response = SD_sendCommand(ERASE_BLOCK_START_ADDR, startBlock, 0x95); //send starting block address
  if(response != 0x00) //check for SD status: 0x00 - OK (No flags set)
    return response;

  response = SD_sendCommand(ERASE_BLOCK_END_ADDR,(startBlock + totalBlocks - 1), 0x95); //send end block address
  if(response != 0x00)
    return response;

  response = SD_sendCommand(ERASE_SELECTED_BLOCKS, 0, 0x95); //erase all selected blocks
  if(response != 0x00)
    return response;

  return 0; //normal return
}

/* SD_readSingleBlock
 * reads a single block of size 512 bytes
 */
unsigned char SD_readSingleBlock(unsigned long startBlock)
{
unsigned char response;
unsigned int i, retry=0;

 response = SD_sendCommand(READ_SINGLE_BLOCK, startBlock, 0x95); //read a Block command

 if(response != 0x00) return response; //check for SD status: 0x00 - OK (No flags set)

CS_SELECT();

retry = 0;
while(SD_receiveByte() != 0xfe) //wait for start block token 0xfe (0x11111110)
  if(retry++ > 0xfffe){CS_DESELECT(); return 1;} //return if time-out

for(i=0; i<512; i++) //read 512 bytes
  buffer[i] = SD_receiveByte();

SD_receiveByte(); //receive incoming CRC (16-bit), CRC is ignored here
SD_receiveByte();

SD_receiveByte(); //extra 8 clock pulses
CS_DESELECT();

return 0;
}

/* SD_writeSingleBlock
 * writes a single block of 512 bytes
 */
unsigned char SD_writeSingleBlock(unsigned long startBlock)
{
unsigned char response;
unsigned int i, retry=0;

 response = SD_sendCommand(WRITE_SINGLE_BLOCK, startBlock, 0x95); //write a Block command

 if(response != 0x00) return response; //check for SD status: 0x00 - OK (No flags set)

CS_SELECT();

spiSendByte(0xfe);     //Send start block token 0xfe (0x11111110)

for(i=0; i<512; i++)    //send 512 bytes data
	spiSendByte(buffer[i]);

spiSendByte(0xff);     //transmit dummy CRC (16-bit), CRC is ignored here
spiSendByte(0xff);

response = SD_receiveByte();

if( (response & 0x1f) != 0x05) //response= 0xXXX0AAA1 ; AAA='010' - data accepted
{                              //AAA='101'-data rejected due to CRC error
  CS_DESELECT();              //AAA='110'-data rejected due to write error
  return response;
}

while(!SD_receiveByte()) //wait for SD card to complete writing and get idle
if(retry++ > 0xfffe){CS_DESELECT(); return 1;}

CS_DESELECT();
spiSendByte(0xff);   //just spend 8 clock cycle delay before reasserting the CS line
CS_SELECT();         //re-asserting the CS line to verify if card is still busy

while(!SD_receiveByte()) //wait for SD card to complete writing and get idle
   if(retry++ > 0xfffe){CS_DESELECT(); return 1;}
CS_DESELECT();

return 0;
}

