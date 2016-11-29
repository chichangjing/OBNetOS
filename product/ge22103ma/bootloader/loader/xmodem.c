/* Standard includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* BSP includes */
#include "stm32f2xx.h"
#include "console.h"
#include "timer.h"
#include "flash_if.h"
#include "soft_i2c.h"

/* Other includes */
#include "cli.h"
#include "xmodem.h"
#include "ob_image.h"

#define READBYTE_TIMEOUT		1000	/* 1000 ms */
#define WAIT_RECEIVE			100		/* 100 * READBYTE_TIMEOUT */

#define XMODEM_RXBUFFER_SIZE	1028	/* 1024 + 2 + 2 */
#define FIRST_FLASH_PARTITION	0x08010000
#define SECOND_FLASH_PARTITION	0x08080000
#define DEFAULT_FLASH_UPGRADE_ADDR	FIRST_FLASH_PARTITION

unsigned char RxBuffer[XMODEM_RXBUFFER_SIZE];

extern USART_TypeDef *ConsolePort;
extern eComMode ConsoleMode;

static cli_cmd_t cmd_xmodem	= {
	0,
	"xmodem",
	"Upgrade firmware by 1K-Xmodem",
	"<cr>",
	CmdXmodem,0,0,0
};

static unsigned short crctab[256] = { 
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};


static void SendByte(char data)
{
	ConsolePutChar(data);
}

static int ReadByte(int timeoutMs)
{
	int i;
	char ch;

	extern int gTimeoutFlag;
	gTimeoutFlag = 0;
	
	for(i=0; i<timeoutMs; i++) {
		while(gTimeoutFlag == 0) {
			if((ConsolePort->SR & USART_FLAG_RXNE)){
				ch = (uint16_t)(ConsolePort->DR & (uint16_t)0x01FF);
				ConsolePort->SR = (uint16_t)~USART_FLAG_RXNE;
				return ch;
			}
		}
		gTimeoutFlag = 0;
	}
	
	return ERR_TIMEOUT;
}


static void ReadPacket(int RxSize, unsigned char *pBuffer)
{
	int i;
	
	for(i=0;i<RxSize;i++) {
		while(!(ConsolePort->SR & USART_SR_RXNE));
		pBuffer[i] = (ConsolePort->DR) & 0xff;
	}
	if(ConsoleMode == RS485)
		while(!(ConsolePort->SR & USART_SR_IDLE));	
}


static int ImagePreCheck(unsigned char *buffer, int *DataSize)
{
	OB_Image_Header_t	OBHeader;
	unsigned long checksum;

	memcpy (&OBHeader, buffer, sizeof(OB_Image_Header_t));
	
	if(ntohl(OBHeader.Magic) != IH_MAGIC)
		return -1;
	
	checksum = ntohl(OBHeader.HeaderCRC32);
	OBHeader.HeaderCRC32 = 0;

	if (crc32(0, (unsigned char *)&OBHeader, sizeof(OB_Image_Header_t)) != checksum)
		return -2;

	*DataSize = ntohl(OBHeader.DataSize) + sizeof(OB_Image_Header_t);
	
	return 0;
}

int XmodemWrite(unsigned int write_address, int *receive_size)
{
	int	firstchar;  	/* first character of a packet */
	int	sectnum = 0;    /* number of last received packet (mod 128) */
	int	sectcurr;   	/* 2nd byte of packet--should be packet number (mod 128) */
	int	sectcomp;   	/* 3rd byte of packet--should be complement of sectcurr */		
	int bufsize, wrSize;
	int start_flag;
	int i;
	unsigned int FlashWriteAddr;
	unsigned int FlashRet;
	int ImageSize=0;
	int ImageHeaderChkFlag=0;

	start_flag = 0;
	i = 0;
	do {	
		SendByte(0x43); /* Send 'C' */
		firstchar = ReadByte(1000);
		i++;
		if(i == WAIT_RECEIVE) goto ret;
		if(firstchar == 0x1A) goto ret; /* Ctrl-Z to stop */	
	} while(firstchar != SOH && firstchar != STX && firstchar != EOT);
	start_flag = 1;

	FlashWriteAddr = write_address;
	*receive_size = 0;
	do{
		if(!start_flag){
			do {	 
				firstchar = ReadByte(READBYTE_TIMEOUT);
			} while(firstchar != SOH && firstchar != STX && firstchar != EOT);
		}
		start_flag = 0;

		
		if (firstchar == SOH || firstchar == STX) {
			unsigned short cal_checksum, in_checksum;
			unsigned char c;

			bufsize = (firstchar == SOH) ? 128 : 1024;
			
			ReadPacket(bufsize+4, RxBuffer);
			
			/* Check the packet sequnce */
			sectcurr = RxBuffer[0];
			sectcomp = RxBuffer[1];
			if((sectcurr + sectcomp) != 0xFF) {
				SendByte(NAK);
				return ERR_SEQ_NUM;
			}

			if(sectcurr != ((sectnum+1) & 0xff)) {
				SendByte(NAK);
				return ERR_SEQ_NUM;
			}
				
			/* Verify the checksum */
			cal_checksum = 0;
			for(i=0;i<bufsize;i++) {
				c =  RxBuffer[i+2];
				cal_checksum = (cal_checksum<<8) ^ crctab[(cal_checksum>>8) ^ c];
			}
			in_checksum = RxBuffer[bufsize+4-2];
			in_checksum = (in_checksum << 8) + RxBuffer[bufsize+4-1];
			if(cal_checksum != in_checksum) {
				SendByte(NAK);
				return ERR_CRC;				
			}

			/* Check image header information */
			if((sectnum == 0) && (ImageHeaderChkFlag == 0)) {
				if(ImagePreCheck(&RxBuffer[2], &ImageSize) < 0) {
					SendByte(NAK);
					return ERR_CHECK_HEADER;
				} else {
					ImageHeaderChkFlag = 1;
				}
			}

			/* Process the packet */
			if(ImageSize > bufsize) {
				wrSize = bufsize;
			} else {
				wrSize = ImageSize;
			}
			
			*receive_size += wrSize;
			sectnum = sectcurr; 
			FlashRet = FLASH_If_Write(&FlashWriteAddr, (uint32_t *)&RxBuffer[2], wrSize);
			if(FlashRet == 0) {
				ImageSize-=wrSize;
				FlashWriteAddr = write_address + *receive_size;			
				SendByte(ACK);
			} else {
				SendByte(NAK);
				return ERR_FLASH_WRITE;
			}
		}

	}while (firstchar != EOT);
	
	if(ConsoleMode == RS485)
		while(!(ConsolePort->SR & USART_SR_IDLE));
		
	SendByte(ACK);
        
	return XMODEM_SUCCESS;
ret:
	return XMODEM_FAILURE;
}

void CmdXmodem(int argc, char **argv)
{
	unsigned int WriteAddrStart, WriteAddrEnd;
	int rxsize=0;
	int ret=0;
	unsigned char UpgradeFlag=0xff;
	
	if(argc != 1) {
		printf("Usage: xmodem\r\n");
		return;
	}

	WriteAddrStart = 0x08080000;
	WriteAddrEnd =  0x080FFFFF;	

	printf("Erase flash ... ");
	FLASH_If_Init();
	FLASH_If_Erase(WriteAddrStart, WriteAddrEnd);
	printf("done\r\n");
	
	printf("Start xmodem transfer ...\r\n");
	if((ret=XmodemWrite(WriteAddrStart, &rxsize)) != XMODEM_SUCCESS) {
		printf("\r\nTransfer error, ret=%d\r\n",ret);
	} else {
		printf("\r\nTransfer completed, received and writed %d bytes\r\n", rxsize);
		UpgradeFlag = 0x80;
		if(eeprom_write(EPROM_ADDR_UPGRADE_FLAG, &UpgradeFlag, 1) == I2C_SUCCESS)
			printf("System will upgrade at next boot\r\n");
		else
            printf("Error: write eeprom failed.\r\n");
	}
	
}

void RegisterXmodemCommand(void)
{
	cli_register_command(&cmd_xmodem);
}

