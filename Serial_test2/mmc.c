// ***********************************************************
// File: mmc.c
// Description: Library to access a MultiMediaCard
//              functions: init, read, write ...
//  C. Speck / S. Schauer
//  Texas Instruments, Inc
//  June 2005
//
// Version 1.1
//   corrected comments about connection the MMC to the MSP430
//   increased timeout in mmcGetXXResponse
//
// ***********************************************************
// MMC Lib
// ***********************************************************
#include "msp430g2553.h"
#include "mmc.h"
#include "uart.h"

#define CS BIT4         // Chip Select line
#define MOSI BIT7       // Master-out Slave-in
#define MISO BIT6
#define SCK BIT5        // Serial clock

#define SPI_400kHz 2 /* 16MHz/400000 */

#ifndef LOBYTE
#define LOBYTE(w) ((w)&0xFF)
#define HIBYTE(w) ((w)>>8)
#endif



#define SPI_MODE_0 UCCKPL | UCMSB| UCMST | UCSYNC

// Function Prototypes
char mmcGetResponse(void);
char mmcGetXXResponse(const char resp);
char mmcCheckBusy(void);
unsigned char spiSendByte(const unsigned char data);
char mmc_GoIdle();

// Varialbes
//char mmc_buffer[512] = { 0 };               // Buffer for mmc i/o for data and registers

//---------------------------------------------------------------------
unsigned char spiSendByte(const unsigned char data)
{
  while (!(IFG2 & UCB0TXIFG));              // Wait for RX to finish
  UCB0TXBUF=data;
  //USICNT = 8;                             // send 8 bits of data
  while (!(IFG2 & UCB0TXIFG));              // Wait for RX to finish
  return (UCB0RXBUF);                     // Store data
}

void MMC_initSPI(void){


	/*//chip select
	SD_CSn_PxDIR|=SD_CSn_PIN;
	SD_CSn_PxOUT|=SD_CSn_PIN;
	P1SEL  |= SCK | MISO | MOSI;	// configure P1.5, P1.6, P1.7 for USCI
	P1SEL2 |= SCK | MISO | MOSI;
	P1DIR |= MOSI | SCK;
	__delay_cycles(15000);

	//UCB0CTL1 = UCSSEL_2 | UCSWRST;	// source USCI clock from SMCLK, put USCI in reset mode
		//UCB0CTL0 |= SPI_MODE_0;			// SPI MODE 0 - CPOL=0 CPHA=0
											// note: UCCKPH is inverted from CPHA
		 USCI B0
		UCB0CTL0 |= UCCKPL + UCMSB + UCMST + UCSYNC; // 3-pin, 8-bit SPI master
		UCB0CTL1 |= UCSSEL_2; // SMCLK
		UCB0BR0 |= 8; // /2
		UCB0BR1 = 0;
		//UCB0MCTL = 0;



	//P2OUT |= BIT0;					// CS on P2.0. start out disabled
	//P2DIR |= BIT0;					// CS configured as output

		UCB0CTL1 &= ~UCSWRST;			// release for operation

*/


	  /* recommended procedure: set UCSWRST, configure USCI, configure ports, activate */

	  /* UCSWRST */
	  UCB0CTL1 = UCSWRST;

	  /* USCI B0 */
	  UCB0CTL0 |= UCCKPL + UCMSB + UCMST + UCSYNC; // 3-pin, 8-bit SPI master
	  UCB0CTL1 |= UCSSEL_2; // SMCLK
	  UCB0BR0 |= 64; // /2
	  UCB0BR1 = 0;
	  //UCB0MCTL = 0;

	  /* Ports */
	  P1SEL |= SCK + MOSI + MISO;
	  P1SEL2 |= SCK + MOSI + MISO;
	  P1DIR |= MOSI | SCK;
	  IE2 |= UCB0RXIE;
	  /* activate */
	  UCB0CTL1 &= ~UCSWRST;

  //P1SEL = BIT5 + BIT6 + BIT7;
  //P1SEL2 = BIT5 + BIT6 + BIT7;


      // Pause so everything has time to start up properly.
      //__delay_cycles( 15000 );

 // UCB0CTL0 |= UCCKPL + UCMSB + UCMST + UCSYNC;  // 3-pin, 8-bit SPI master
  //UCB0CTL1 |=  UCSSEL_2 + UCCKPL;                  // SMCLK
  //UCB0BR0 |= 8;                          // /2
  //UCB0BR1 = 0;
                            // Enable USCI_B0 RX interrupt
  //USICTL0 |= USIPE7 + USIPE6 + USIPE5 + USIMST + USIOE; // Port, SPI master
  //USICTL1 |= USICKPH; //  USICKPH;                         // Counter interrupt, flag remains set
  //USICKCTL = USIDIV_0 + USISSEL_2 + USICKPL;          // /4 SMCLK + USICKPL +
  //USICTL0 &= ~USISWRST;                     // USI released for operation
  //UCB0CTL1 &= ~UCSWRST;                  // **Initialize USCI state machine**
}


// Initialize MMC card
//set DI and CS high and apply more than 74 pulses to SCLK
char initMMC (void){
  int i;
  CS_HIGH(); //set CS high
  for(i=0;i<11;i++)
    spiSendByte(0xff);//set DI high with ff (10 times)
  return (mmc_GoIdle());
}

char mmc_GoIdle(){
  char response=0x01;
  char temp;
  CS_LOW();

  //Send Command 0 to put MMC in SPI mode
  mmcSendCmd(MMC_GO_IDLE_STATE,0,0x95);
  //Now wait for READY RESPONSE
  temp = mmcGetResponse();
  if(temp!=0x01)
  {
    return MMC_INIT_ERROR;
  }
  while(response==0x01)
  {
    CS_HIGH();

    spiSendByte(0xff);
    CS_LOW();

    mmcSendCmd(MMC_SEND_OP_COND,0x00,0xff);

    response=mmcGetResponse();

  }

  CS_HIGH();
  spiSendByte(0xff);
  return (MMC_SUCCESS);
}

// mmc Get Responce
char mmcGetResponse(void)
{
  //Response comes 1-8bytes after command
  //the first bit will be a 0
  //followed by an error code
  //data will be 0xff until response
  int i=0;

  char response;

  while(i<=64)
  {
    response=spiSendByte(0xff);
    if(response==0x00)break;
    if(response==0x01)break;
    i++;
  }
  return response;
}

char mmcGetXXResponse(const char resp)
{
  //Response comes 1-8bytes after command
  //the first bit will be a 0
  //followed by an error code
  //data will be 0xff until response
  int i=0;

  char response;

  while(i<=1000)
  {
    response=spiSendByte(0xff);
    if(response==resp)break;
    i++;
  }
  return response;
}
char mmcCheckBusy(void)
{
  //Response comes 1-8bytes after command
  //the first bit will be a 0
  //followed by an error code
  //data will be 0xff until response
  int i=0;

  char response;
  char rvalue;
  while(i<=64)
  {
    response=spiSendByte(0xff);
    response &= 0x1f;
    switch(response)
    {
      case 0x05: rvalue=MMC_SUCCESS;break;
      case 0x0b: return(MMC_CRC_ERROR);
      case 0x0d: return(MMC_WRITE_ERROR);
      default:
        rvalue = MMC_OTHER_ERROR;
        break;
    }
    if(rvalue==MMC_SUCCESS)break;
    i++;
  }
  i=0;
  do
  {
    response=spiSendByte(0xff);
    i++;
  }while(response==0);
  return response;
}

// The card will respond with a standard response token followed by a data
// block suffixed with a 16 bit CRC.

char mmcReadBlock(const unsigned long address, const unsigned long count, unsigned char *pBuffer)
{
  unsigned long i = 0;
  char rvalue = MMC_RESPONSE_ERROR;

  // Set the block length to read
  if (mmcSetBlockLength (count) == MMC_SUCCESS)   // block length could be set
  {
    // SS = LOW (on)
    CS_LOW ();
    // send read command MMC_READ_SINGLE_BLOCK=CMD17
    mmcSendCmd (MMC_READ_SINGLE_BLOCK,address, 0xFF);
    // Send 8 Clock pulses of delay, check if the MMC acknowledged the read block command
    // it will do this by sending an affirmative response
    // in the R1 format (0x00 is no errors)
    if (mmcGetResponse() == 0x00)
    {
      // now look for the data token to signify the start of
      // the data
      if (mmcGetXXResponse(MMC_START_DATA_BLOCK_TOKEN) == MMC_START_DATA_BLOCK_TOKEN)
      {
        // clock the actual data transfer and receive the bytes; spi_read automatically finds the Data Block
        for (i = 0; i < count; i++)
          pBuffer[i] = spiSendByte(0xff);   // is executed with card inserted

        // get CRC bytes (not really needed by us, but required by MMC)
        spiSendByte(0xff);
        spiSendByte(0xff);
        rvalue = MMC_SUCCESS;
      }
      else
      {
        // the data token was never received
        rvalue = MMC_DATA_TOKEN_ERROR;      // 3
      }
    }
    else
    {
      // the MMC never acknowledge the read command
      rvalue = MMC_RESPONSE_ERROR;          // 2
    }
  }
  else
  {
    rvalue = MMC_BLOCK_SET_ERROR;           // 1
  }
  CS_HIGH ();
  spiSendByte(0xff);
  return rvalue;
}// mmc_read_block

/*****************************************************
*
*       Function to mount an SD card sector for a read.
*       Second function reads X bytes into the designated array
*
******************************************************/
char mmcReadBytes(const unsigned char count, unsigned char *pBuffer)
{
  unsigned long i=0;

   // clock the actual data transfer and receive the bytes; spi_read automatically finds the Data Block
   for (i = 0; i < count; i++)
       pBuffer[i] = spiSendByte(0xff);   // is executed with card inserted
   return count;
}

char mmcReadByte(void)
{
    return  spiSendByte(0xff);   // is executed with card inserted
}

void mmcUnmountBlock(void)
{
    // get CRC bytes (not really needed by us, but required by MMC)
    spiSendByte(0xff);
    spiSendByte(0xff);
    CS_HIGH ();
    spiSendByte(0xff);
}

char mmcMountBlock(unsigned long address)
{
  char rvalue = MMC_RESPONSE_ERROR;

  // Set the block length to read
  if (mmcSetBlockLength (512) == MMC_SUCCESS)   // block length could be set
  {
    // SS = LOW (on)
    CS_LOW ();
    // send read command MMC_READ_SINGLE_BLOCK=CMD17
    mmcSendCmd (MMC_READ_SINGLE_BLOCK,address, 0xFF);
    // Send 8 Clock pulses of delay, check if the MMC acknowledged the read block command
    // it will do this by sending an affirmative response
    // in the R1 format (0x00 is no errors)
    if (mmcGetResponse() == 0x00)
    {
      // now look for the data token to signify the start of
      // the data
      if (mmcGetXXResponse(MMC_START_DATA_BLOCK_TOKEN) == MMC_START_DATA_BLOCK_TOKEN)
      {
         //success, data ready to read
         rvalue = MMC_SUCCESS;
      }
      else
      {
        // the data token was never received
        rvalue = MMC_DATA_TOKEN_ERROR;      // 3
        CS_HIGH ();
        spiSendByte(0xff);
      }
    }
    else
    {
      // the MMC never acknowledge the read command
      rvalue = MMC_RESPONSE_ERROR;          // 2
      CS_HIGH ();
      spiSendByte(0xff);
    }
  }
  else
  {
    rvalue = MMC_BLOCK_SET_ERROR;           // 1
    CS_HIGH ();
    spiSendByte(0xff);
  }
    return rvalue;
}// mmc_read_block

//---------------------------------------------------------------------
//char mmcWriteBlock (const unsigned long address)
char mmcWriteBlock (const unsigned long address, const unsigned long count, unsigned char *pBuffer)
{
  unsigned long i = 0;
  char rvalue = MMC_RESPONSE_ERROR;         // MMC_SUCCESS;
  //  char c = 0x00;

  // Set the block length to read
  if (mmcSetBlockLength (count) == MMC_SUCCESS)   // block length could be set
  {
    // SS = LOW (on)
    CS_LOW ();
    // send write command
    mmcSendCmd (MMC_WRITE_BLOCK,address, 0xFF);

    // check if the MMC acknowledged the write block command
    // it will do this by sending an affirmative response
    // in the R1 format (0x00 is no errors)
    if (mmcGetXXResponse(MMC_R1_RESPONSE) == MMC_R1_RESPONSE)
    {
      spiSendByte(0xff);
      // send the data token to signify the start of the data
      spiSendByte(0xfe);
      // clock the actual data transfer and transmitt the bytes
      for (i = 0; i < count; i++)
        spiSendByte(pBuffer[i]);

      // put CRC bytes (not really needed by us, but required by MMC)
      spiSendByte(0xff);
      spiSendByte(0xff);
      // read the data response xxx0<status>1 : status 010: Data accected, status 101: Data
      //   rejected due to a crc error, status 110: Data rejected due to a Write error.
      mmcCheckBusy();
      rvalue = MMC_SUCCESS;
    }
    else
    {
      // the MMC never acknowledge the write command
      rvalue = MMC_RESPONSE_ERROR;   // 2
    }
  }
  else
  {
    rvalue = MMC_BLOCK_SET_ERROR;   // 1
  }
  // give the MMC the required clocks to finish up what ever it needs to do
  //  for (i = 0; i < 9; ++i)
  //    spiSendByte(0xff);

  CS_HIGH ();
  // Send 8 Clock pulses of delay.
  spiSendByte(0xff);
  return rvalue;
} // mmc_write_block


//---------------------------------------------------------------------
void mmcSendCmd (const char cmd, unsigned long data, const char crc)
{
  char frame[6];
  char temp;
  int i;
  frame[0]=(cmd|0x40);
  for(i=3;i>=0;i--){
    temp=(char)(data>>(8*i));
    frame[4-i]=(temp);
  }
  frame[5]=(crc);
  for(i=0;i<6;i++)
    spiSendByte(frame[i]);
}


//--------------- set blocklength 2^n ------------------------------------------------------
char mmcSetBlockLength (const unsigned long blocklength)
{
  //  char rValue = MMC_TIMEOUT_ERROR;
  //  char i = 0;
  // SS = LOW (on)
  CS_LOW ();
  // Set the block length to read
  //MMC_SET_BLOCKLEN =CMD16
  mmcSendCmd(MMC_SET_BLOCKLEN, blocklength, 0xFF);

  // get response from MMC - make sure that its 0x00 (R1 ok response format)
  if(mmcGetResponse()!=0x00)
  { initMMC();
    mmcSendCmd(MMC_SET_BLOCKLEN, blocklength, 0xFF);
    mmcGetResponse();
  }

  CS_HIGH ();

  // Send 8 Clock pulses of delay.
  spiSendByte(0xff);

  return MMC_SUCCESS;
} // Set block_length


// Reading the contents of the CSD and CID registers in SPI mode is a simple
// read-block transaction.
char mmcReadRegister (const char cmd_register, const unsigned char length, unsigned char *pBuffer)
{
  char uc = 0;
  char rvalue = MMC_TIMEOUT_ERROR;

  if (mmcSetBlockLength (length) == MMC_SUCCESS)
  {
    CS_LOW ();
    // CRC not used: 0xff as last byte
    mmcSendCmd(cmd_register, 0x000000, 0xff);

    // wait for response
    // in the R1 format (0x00 is no errors)
    if (mmcGetResponse() == 0x00)
    {
      if (mmcGetXXResponse(0xfe)== 0xfe)
        for (uc = 0; uc < length; uc++)
          pBuffer[uc] = spiSendByte(0xff);  //mmc_buffer[uc] = spiSendByte(0xff);
      // get CRC bytes (not really needed by us, but required by MMC)
      spiSendByte(0xff);
      spiSendByte(0xff);
      rvalue = MMC_SUCCESS;
    }
    else
      rvalue = MMC_RESPONSE_ERROR;
    // CS = HIGH (off)
    CS_HIGH ();

    // Send 8 Clock pulses of delay.
    spiSendByte(0xff);
  }
  CS_HIGH ();
  return rvalue;
} // mmc_read_register


#include "math.h"
unsigned long MMC_ReadCardSize(void)
{
  // Read contents of Card Specific Data (CSD)

  unsigned long MMC_CardSize;
  unsigned short i,      // index
                 j,      // index
                 b,      // temporary variable
                 response,   // MMC response to command
                 mmc_C_SIZE;

  unsigned char mmc_READ_BL_LEN,  // Read block length
                mmc_C_SIZE_MULT;

  CS_LOW ();

  spiSendByte(MMC_READ_CSD);   // CMD 9
  for(i=4; i>0; i--)      // Send four dummy bytes
    spiSendByte(0);
  spiSendByte(0xFF);   // Send CRC byte

  response = mmcGetResponse();

  // data transmission always starts with 0xFE
  b = spiSendByte(0xFF);

  if( !response )
  {
    while (b != 0xFE) b = spiSendByte(0xFF);
    // bits 127:87
    for(j=5; j>0; j--)          // Host must keep the clock running for at
      b = spiSendByte(0xff);


    // 4 bits of READ_BL_LEN
    // bits 84:80
    b =spiSendByte(0xff);  // lower 4 bits of CCC and
    mmc_READ_BL_LEN = b & 0x0F;

    b = spiSendByte(0xff);

    // bits 73:62  C_Size
    // xxCC CCCC CCCC CC
    mmc_C_SIZE = (b & 0x03) << 10;
    b = spiSendByte(0xff);
    mmc_C_SIZE += b << 2;
    b = spiSendByte(0xff);
    mmc_C_SIZE += b >> 6;

    // bits 55:53
    b = spiSendByte(0xff);

    // bits 49:47
    mmc_C_SIZE_MULT = (b & 0x03) << 1;
    b = spiSendByte(0xff);
    mmc_C_SIZE_MULT += b >> 7;

    // bits 41:37
    b = spiSendByte(0xff);

    b = spiSendByte(0xff);

    b = spiSendByte(0xff);

    b = spiSendByte(0xff);

    b = spiSendByte(0xff);

  }

  for(j=4; j>0; j--)          // Host must keep the clock running for at
    b = spiSendByte(0xff);  // least Ncr (max = 4 bytes) cycles after
                               // the card response is received
  b = spiSendByte(0xff);
  CS_LOW ();

  MMC_CardSize = (mmc_C_SIZE + 1);
  // power function with base 2 is better with a loop
  // i = (pow(2,mmc_C_SIZE_MULT+2)+0.5);
  for(i = 2,j=mmc_C_SIZE_MULT+2; j>1; j--)
    i <<= 1;
  MMC_CardSize *= i;
  // power function with base 2 is better with a loop
  //i = (pow(2,mmc_READ_BL_LEN)+0.5);
  for(i = 2,j=mmc_READ_BL_LEN; j>1; j--)
    i <<= 1;
  MMC_CardSize *= i;

  return (MMC_CardSize);

}


char mmc_ping(void)
{
 // if ((SD_CD_PxIN & SD_CD_PIN))
    return (MMC_SUCCESS);
 // else
 //   return (MMC_INIT_ERROR);
}

