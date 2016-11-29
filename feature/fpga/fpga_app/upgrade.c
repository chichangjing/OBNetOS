
#include "fpga_debug.h"
#include "upgrade.h"
#include "common.h"
#include "hal_uart.h"
#include <string.h>
#include "halsw_i2c.h"
#include "spi_flash.h"


unsigned int fblocknum = 0;
unsigned int fdstaddr = PARTITOPNCADDR;
unsigned int uermemmask = 0;
unsigned int flashprotection = 0;

static device_t deviceinfo;


pFunction JumpToApplication;
unsigned int jumpaddress;


void JumpToApp(void)
{
	if(((*(__IO unsigned int *)ApplicationAddress) & 0x2FFE0000) == 0x20000000){
		jumpaddress = *(__IO unsigned int *)(ApplicationAddress + 4);
		/* Jump to user application */
		JumpToApplication = (pFunction) jumpaddress;
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO unsigned int *) ApplicationAddress);
		JumpToApplication();
	}
}
	
void JumpToBoot(void)
{
	if(((*(__IO unsigned int *)BootAddress) & 0x2FFE0000) == 0x20000000){
		jumpaddress = *(__IO unsigned int *)(BootAddress + 4);
		/* Jump to user application */
		JumpToApplication = (pFunction) jumpaddress;
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO unsigned int *) BootAddress);
		JumpToApplication();
	}
}
#if 0
void FlashDisWriteProtectPage(void)
{
	unsigned int useroptionbyte = 0, WRPR = 0;
	unsigned short int var1 = OB_IWDG_SW, var2 = OB_STOP_NoRST, var3 = OB_STDBY_NoRST;
	FLASH_Status status = FLASH_BUSY;
	
	WRPR = FLASH_GetWriteProtectionOptionByte();
	/* Test if user memory is write protected */
	if((WRPR & uermemmask) != uermemmask)
	{
		useroptionbyte = FLASH_GetUserOptionByte();
		uermemmask |= WRPR;
		status = FLASH_EraseOptionBytes();
		if(uermemmask != 0xFFFFFFFF)
		{
			status = FLASH_EnableWriteProtection((unsigned int)~uermemmask);
		}
		/* Test if user Option Bytes are programmed */
		if((useroptionbyte & 0x07) != 0x07)
		{
			/* Restore user Option Bytes */
			if((useroptionbyte & 0x01) == 0x0)
			{
				var1 = OB_IWDG_HW;
			}
			if((useroptionbyte & 0x02) == 0x0)
			{
				var2 = OB_STOP_RST;
			}
			if((useroptionbyte & 0x04) == 0x0)
			{
				var3 = OB_STDBY_RST;
			}
			FLASH_UserOptionByteConfig(var1, var2, var3);
		}
		if(status == FLASH_COMPLETE)
		{
//			DEBUG("Write Protection disabled...\r\n");
//			DEBUG("...and a System Reset will be generated to re-load the new option bytes\r\n");
			/* Generate System Reset to load the new option byte values */
			NVIC_SystemReset();
		}
		else
		{
//			DEBUG("Error: Flash write unprotection failed...\r\n");
		}
	}
	else
	{
//		DEBUG("Flash memory not write protected\r\n");
	}
}
#endif
#if 0
void FLASH_DisableWriteProtectionPages(void)
{
	unsigned int useroptionbyte = 0, WRPR = 0;
	unsigned short int var1 = OB_IWDG_SW, var2 = OB_STOP_NoRST, var3 = OB_STDBY_NoRST;
	FLASH_Status status = FLASH_BUSY;
	
	WRPR = FLASH_GetWriteProtectionOptionByte();
	/* Test if user memory is write protected */
	if((WRPR & uermemmask) != uermemmask)
	{
		useroptionbyte = FLASH_GetUserOptionByte();
		uermemmask |= WRPR;
		status = FLASH_EraseOptionBytes();
		if(uermemmask != 0xFFFFFFFF)
		{
			status = FLASH_EnableWriteProtection((unsigned int)~uermemmask);
		}
		/* Test if user Option Bytes are programmed */
		if((useroptionbyte & 0x07) != 0x07)
		{
			/* Restore user Option Bytes */
			if((useroptionbyte & 0x01) == 0x0)
			{
				var1 = OB_IWDG_HW;
			}
			if((useroptionbyte & 0x02) == 0x0)
			{
				var2 = OB_STOP_RST;
			}
			if((useroptionbyte & 0x04) == 0x0)
			{
				var3 = OB_STDBY_RST;
			}
			FLASH_UserOptionByteConfig(var1, var2, var3);
		}
		if(status == FLASH_COMPLETE)
		{
			DEBUG("Write Protection disabled...\r\n");
			DEBUG("...and a System Reset will be generated to re-load the new option bytes\r\n");
			/* Generate System Reset to load the new option byte values */
			NVIC_SystemReset();
		}
		else
		{
			DEBUG("Error: Flash write unprotection failed...\r\n");
		}
	}
	else
	{
		DEBUG("Flash memory not write protected\r\n");
	}
}
#endif

FLASH_Status flashstatus = FLASH_COMPLETE;

#if 0
unsigned int FlashCpy(unsigned int dist,unsigned int src, unsigned int size)
{
	upgradeif updateinfo;
	unsigned int erasepage = 0, pagecnt = 0;
	unsigned int i;
	unsigned char err[2] = {0, 0};
	FLASH_Status fstatus = FLASH_COMPLETE;

	if(size % PAGE_SIZE){
		pagecnt = size  / PAGE_SIZE + 1;
	}
	else{
		pagecnt = size / PAGE_SIZE;
	}
	/* erase flash */
	for(erasepage = 0; ((erasepage < pagecnt) && (fstatus == FLASH_COMPLETE)); erasepage++){
		fstatus = FLASH_ErasePage(dist + (PAGE_SIZE * erasepage));
	}
	if(fstatus != FLASH_COMPLETE){
		err[0] = (unsigned char)FLASHERASEERR;
		err[1] = (unsigned char)FLASHERASEERR >> 8;
	}
//	size = pagecnt * PAGE_SIZE;
	size = (size & 0x03) ? size + (4 - (size & (0x03))): size ;	
	for(i=0; i<size; i+=4){
		fstatus = FLASH_ProgramWord(dist, *(unsigned int *)src);
		if((fstatus != FLASH_COMPLETE) || (*(unsigned int*)dist != *(unsigned int*)src)){
			err[0] = (unsigned char)FLASHWRITEERR;
			err[1] = (unsigned char)FLASHWRITEERR >> 8;
			break;
		}
		dist += 4;
		src += 4;
	}
	/* none err */
	Uart1Pollputc(err, 2);
	if(i != size){
		return 0;
	}
	else{
		At24_Sequential_Read(I2CDEV_0, E2DEVADD0, UPGRADEINFOADD, (unsigned char *)&updateinfo, sizeof(upgradeif));
		updateinfo.status= NOMAL;
		At24_Page_Write(I2CDEV_0, E2DEVADD0, UPGRADEINFOADD, (unsigned char *)&updateinfo, sizeof(upgradeif));
		return i;
	}

}
#endif

unsigned char SumAdd(unsigned char* buf, unsigned int size)
{
	unsigned int i;
	unsigned char ret = 0;
	for (i = 0; i < size; i++) {
		ret += *buf;
		buf++;
	}
	return ret;
}



static upgradeif lastupinfo;
static char imgtempame[32], version;

#define SUBDATA 7

void UpgradeCmdDump(upgrademsg *UpgradeMsgp)
{ 
	unsigned char opcode;
	unsigned short seqid;
	
	opcode = UpgradeMsgp->opcode;
	seqid = SWP16(*(unsigned short *)&(UpgradeMsgp->version[0]));
	switch(opcode)
		{
		case FUPSTART	:	
			st_fpga_debug(ST_FPGA_DEBUG,"%s device id:0x%x version id:0x%x \r\n",
			"Startting fpga upgrade", UpgradeMsgp->version[0],UpgradeMsgp->version[1]);
			break;
		case FUPCONTENT	:	
			st_fpga_debug(ST_FPGA_DEBUG,"%s \r\nsequence id:0x%x \r\n",
			"Receiving fpga Upgrade context", seqid);
			break;
		case FUPEND		:	
			st_fpga_debug(ST_FPGA_DEBUG,"%s crc check sum :0x%x \r\n",
			"ending fpga Upgrade", UpgradeMsgp->version[1]);
			break;
		default 		:	
			st_fpga_debug(ST_FPGA_DEBUG,"%s \r\n","unknow code");
			break;
		}

}


void UpgradeImageProsess(unsigned char* buf, unsigned int* size)
{
	upgrademsgp upmsgp = (upgrademsgp)buf;
	unsigned int pagecnt, erasepage;
	static int filesize;
	static unsigned char sum = 0;
	static short int  contentcnt = 0;
    uint8_t  Rx_Buffer1[128];

//	upmsgp->content = (char *)&upmsgp->content;
    
    /* display ugrade info */
	UpgradeCmdDump(upmsgp);
	switch (upmsgp->opcode) 
	{
		case FUPSTART:
			if (*(unsigned int *)upmsgp->size > PRIMARYSIZE) {
				// return file size to large //
				*(unsigned int *)upmsgp->size = FILETOLARERR;
				*size = sizeof(upgrademsg) - sizeof(upmsgp->content);
				break;
			}

			
			if ((*size - SUBDATA) > (sizeof(imgtempame) - 1) ) {
				// return file name to long //
				*(unsigned int *)upmsgp->size = FNAMTOLAREERR;
				*size = sizeof(upgrademsg) - sizeof(upmsgp->content);
				break;
			}
			
			At24_Sequential_Read(I2CDEV_0, E2DEVADD0, UPGRADEINFOADD3, (unsigned char *)&lastupinfo, sizeof(lastupinfo));

			/* internal Flash operation  */
		//	FLASH_Unlock();
			SPI3_FLASH_Init();
			/* disable Flash page */
		//	FlashDisWriteProtectPage();
			
			/* init upgrade info */
			lastupinfo.status = (unsigned short int)UPDATING;
			lastupinfo.errtype = (unsigned short int)NONEERR;
			lastupinfo.filesize = *(unsigned int *)upmsgp->size;
			
			/* recode image name */
			memcpy(imgtempame, &upmsgp->content, (*size - SUBDATA));
			imgtempame[*size - SUBDATA] = '\0';
			
			version = upmsgp->version[1];
			if (*(unsigned int *)upmsgp->size % SPI_FLASH_SIZE) {
				pagecnt = *(unsigned int *)upmsgp->size / SPI_FLASH_SIZE + 1;
			}
			else {
				pagecnt = *(unsigned int *)upmsgp->size / SPI_FLASH_SIZE;
			}

			/* erase flash */
			for (erasepage = 0; (erasepage < pagecnt); erasepage++) {
				SPI3_FLASH_EraseSector(PRIMARY_ADDRESS + (SPI_FLASH_SIZE * erasepage));
			}
		//	if (flashstatus != FLASH_COMPLETE) {
		//		/* return flash erase error */
		//		*(unsigned int *)upmsgp->size = FLASHERASEERR;
		//		*size = sizeof(upgrademsg) - sizeof(upmsgp->content);
		//		break;
		//	}
			/* return NONE ERR */
			 
			fdstaddr = PRIMARY_ADDRESS;
			filesize = 0;
			contentcnt = 0;
			sum = 0;
			/* send data size = 128 */
			*(unsigned short int *)upmsgp->version = SWP16(0x80);
		//	*(unsigned short int *)upmsgp->version = *(unsigned short int *)upmsgp->version;
		//	*(unsigned short int *)upmsgp->version = lastupinfo.image.imgver;
			*(unsigned int *)upmsgp->size = NONEERR;
			memcpy(&upmsgp->content, lastupinfo.image.imgname, strlen((char *)lastupinfo.image.imgname));
			*size = sizeof(upgrademsg) - sizeof(upmsgp->content) + strlen((char *)lastupinfo.image.imgname);
			//SPI_Cmd(SPI_FLASH, DISABLE);
			//spi_MCU_pin_to_3state();
			break;
		case FUPCONTENT: {//FUPCONTENT  FUPIFCFG
			uint8_t *pp;
			uint8_t  Rx_Buffer111;
			unsigned char ssump;		
			
			filesize += *(unsigned int *)upmsgp->size;
			//pp = (unsigned int *)&upmsgp->content;
			pp = (uint8_t *)&upmsgp->content;
			ssump = SumAdd(&upmsgp->content, *(unsigned int *)upmsgp->size);
			sum += ssump;
			
			if (filesize > lastupinfo.filesize) {
				*(unsigned int *)upmsgp->size = FILETOLARERR;
				*size = sizeof(upgrademsg) - sizeof(upmsgp->content);
				SPI_Cmd(SPI3_FLASH_SPI,DISABLE);
				SPI3_MCU_pin_to_3state();
				break;
			}	
			if(*(unsigned int *)upmsgp->size & 0x7f){
				for (uint8_t m = 0; m < (128 - (*(unsigned int *)upmsgp->size & 0x7f)); m++) {
					*(&upmsgp->content + *(unsigned int *)upmsgp->size + m) = 0xff;
				}
				*(unsigned int *)upmsgp->size += 128 - (*(unsigned int *)upmsgp->size & 0x7f);
			}
			
			for (int i = 0; i < *(unsigned int *)upmsgp->size; ) {
				SPI3_FLASH_WriteBuffer(pp,fdstaddr,128);
				SPI3_FLASH_ReadBuffer(Rx_Buffer1,fdstaddr,128);
				for(uint8_t j = 0;j < 128; j++){
					if(Rx_Buffer1[j] != *pp){
					*(unsigned int *)upmsgp->size = FLASHWRITEERR;
					*size = sizeof(upgrademsg) - sizeof(upmsgp->content);
					SPI_Cmd(SPI3_FLASH_SPI,DISABLE);
					SPI3_MCU_pin_to_3state();
					break;
					}
					pp++;
				}
				
				fdstaddr += 128;
				i += 128;
			}
            
			contentcnt++;
			*(unsigned short int *)upmsgp->version = contentcnt;
			*(unsigned int *)upmsgp->size = NONEERR;
			*size = sizeof(upgrademsg) - sizeof(upmsgp->content);
			break;
		}
		case FUPEND:
			//GPIO_InitTypeDef GPIO_InitStructure;
		//	FLASH_Lock();
			SPI_Cmd(SPI3_FLASH_SPI, DISABLE);
			SPI3_MCU_pin_to_3state();
			if (sum != upmsgp->version[1]) {
				upmsgp->version[1] = sum;
				*(unsigned int *)upmsgp->size = FILECRCERR;
				*size = sizeof(upgrademsg) - sizeof(upmsgp->content);
				break;
			}
			else {
				lastupinfo.cnt++;
				lastupinfo.errtype = NONEERR;
				lastupinfo.status = UPDATEND;
				memcpy(lastupinfo.image.imgname, imgtempame, strlen(imgtempame) + 1);
				lastupinfo.image.imgsize = lastupinfo.filesize;
				lastupinfo.image.imgver = version;
				At24_Page_Write(I2CDEV_0, E2DEVADD0, UPGRADEINFOADD3, (unsigned char *)&lastupinfo, sizeof(lastupinfo));
				*(unsigned int *)upmsgp->size = NONEERR;
				SPI_Cmd(SPI3_FLASH_SPI, DISABLE);
				SPI3_MCU_pin_to_3state();
			}
			*size = sizeof(upgrademsg) - sizeof(upmsgp->content);
			break;
	}
}




