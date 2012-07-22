/*
 *  SD_MSP540F5419A.h
 *  SD related function for sending commands, initialization, etc.
 *  Created on: Jul 22, 2012
 *  Author: Andrew Reed
 */

#ifndef SD_MSP540F5419A_H_
#define SD_MSP540F5419A_H_

// macro defines
#define HIGH(a) ((a>>8)&0xFF)               // high byte from word
#define LOW(a)  (a&0xFF)                    // low byte from word

//sd card hardware configuration

//chip select
#define SD_CSn_PxOUT    P3OUT//3.6
#define SD_CSn_PxDIR    P3DIR
#define SD_CSn_PIN      BIT6

#define CS_SELECT()    SD_CSn_PxOUT &= ~SD_CSn_PIN               // Card Select
#define CS_DESELECT()   SD_CSn_PxOUT |= SD_CSn_PIN                // Card Deselect

#define DUMMY 0xff

//SD commands
#define GO_IDLE_STATE            0
#define SEND_OP_COND             1
#define SEND_IF_COND			  8
#define SEND_CSD                 9
#define STOP_TRANSMISSION        12
#define SEND_STATUS              13
#define SET_BLOCK_LEN            16
#define READ_SINGLE_BLOCK        17
#define READ_MULTIPLE_BLOCKS     18
#define WRITE_SINGLE_BLOCK       24
#define WRITE_MULTIPLE_BLOCKS    25
#define ERASE_BLOCK_START_ADDR   32
#define ERASE_BLOCK_END_ADDR     33
#define ERASE_SELECTED_BLOCKS    38
#define SD_SEND_OP_COND			  41
//ACMD
#define APP_CMD					  55
#define READ_OCR				  58
#define CRC_ON_OFF               59


volatile unsigned char SDHC_flag, cardType, buffer[512];

unsigned char SD_init(void);
unsigned char SD_sendCommand(const char cmd, unsigned long data, const char crc);
unsigned char SD_receiveByte(void);
unsigned char SD_erase (unsigned long startBlock, unsigned long totalBlocks);
unsigned char SD_readSingleBlock(unsigned long startBlock);
unsigned char SD_writeSingleBlock(unsigned long startBlock);
void SPI_init(void);


#endif /* SD_MSP540F5419A_H_ */
