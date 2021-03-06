/*******************************************************************************
  * @file    Bootloader.c
  * @author  OB T&C Development Team
  * @brief   Bootloader
  ******************************************************************************/

/* Standard includes */
#include <stdio.h>
#include <string.h>

/* BSP includes */
#include "misc.h"
#include "console.h"
#include "timer.h"
#include "soft_i2c.h"
#include "flash_if.h"
#include "led_drv.h"
#include "bdinfo.h"

/* Other includes */
#include "common.h"
#include "cli.h"
#include "xmodem.h"
#include "ob_image.h"
#include "ob_version.h"
#include "cmd_misc.h"
#include "cmd_bdinfo.h"

#define DEFAULT_APP_ENTRY_ADDRESS	((uint32_t)0x08010000)
#define PRIMARY_PARTITION_ADDRESS	((uint32_t)0x08010000)
#define SECOND_PARTITION_ADDRESS	((uint32_t)0x08080000)

#define BootPrint(fmt, args...) do { if(gConsoleEnable) printf(fmt, ##args); } while(0)

unsigned char gBootDelay;
unsigned char gConsoleEnable;
char BootloaderVersion[MAX_LOADER_VERSION_SIZE] = OB_LOADER_VERSION;

typedef void pFunction_t(void);

void StartApp(uint32_t EntryAddress)
{
	pFunction_t * AppEntry;
	
	__set_MSP(*(__IO uint32_t *)EntryAddress);
	AppEntry = ((pFunction_t *)(*((uint32_t *)(EntryAddress + 4))));
	AppEntry();
}


void SwGpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* DIP Switch GPIO Initialize */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);	/* SW1, PD1 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 	/* SW2, PD3 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 	/* SW3, PD4 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

}

BOOL ReadSW1(void)
{
	if ((GPIOD->IDR & GPIO_Pin_1) != (unsigned int)Bit_RESET)
		return TRUE;
	else
		return FALSE;
}

void main(void) 
{
	int i, ret, loop; 
	unsigned char bootdelay, console_enable, abort=0;
	RCC_ClocksTypeDef rcc_clocks;
	ManuInfo_t	DevManuInfo;
	unsigned char upgrade_flag=0;
	unsigned int fmSize=0, fmCrc32=0;
	unsigned int writeAddress, readAddress;
	char loader_ver[MAX_LOADER_VERSION_SIZE];
	
	TimerInit();
	SwGpioInit();
	I2C_GPIO_Config();
	LED_GPIO_config();
	
	if(ReadSW1() == FALSE) {	/* SW1 is DIP on */
		gBootDelay = 3;
		gConsoleEnable = 1;
	} else {
		if(eeprom_read(EPROM_ADDR_BOOT_DELAY, &bootdelay, 1) == I2C_FAILURE) {
			gBootDelay = 3;
		} else {
			if(((bootdelay == 0) || (bootdelay > 0)) && (bootdelay <= 5))
				gBootDelay = bootdelay;
			else
				gBootDelay = 0;
		}	
		if(eeprom_read(EPROM_ADDR_CONSOLE_ENABLE, &console_enable, 1) == I2C_FAILURE) {
			gConsoleEnable = 1;
		} else {
			if((console_enable == 0) || (console_enable == 1))
				gConsoleEnable = console_enable;
			else
				gConsoleEnable = 1;
		}
	}

	if(eeprom_read(EPROM_ADDR_LOADER_VERSION, (u8 *)&loader_ver, MAX_LOADER_VERSION_SIZE) != I2C_SUCCESS) {
		memset(loader_ver, 0, MAX_LOADER_VERSION_SIZE);
	} else {
		if(memcmp(loader_ver, BootloaderVersion, MAX_LOADER_VERSION_SIZE) != 0) {
			memcpy(loader_ver, BootloaderVersion, MAX_LOADER_VERSION_SIZE);
			eeprom_write(EPROM_ADDR_LOADER_VERSION, (u8 *)&loader_ver, MAX_LOADER_VERSION_SIZE);
		}
	}

	if(gConsoleEnable > 0)
  		ConsoleInit(COM_UART5, 115200);

  	BootPrint("\r\n\r\n\r\n\r");
	BootPrint("## GE20023MA bootloader v%s ##\r\n\r\n", BootloaderVersion);

	RCC_GetClocksFreq(&rcc_clocks);
	BootPrint("System Clock Frequency %d MHz\r\n", rcc_clocks.SYSCLK_Frequency/1000/1000);

	if(gConsoleEnable) {
		
		printf("Reading Factory Information ... ");
		ret = ManuInfo_Read(&DevManuInfo);
		if(ret == BDINFO_SUCCESS) {
			printf("Done\r\n");
			ManuInfo_Print(&DevManuInfo);
		} else {
			printf("Failed\r\n");
		}	

		printf("\r\n");
		printf("Press Ctrl-C to stop autoboot: %2d ", gBootDelay);
		
		while ((gBootDelay > 0) && (!abort)) {
			--gBootDelay;
			for (i=0; !abort && i<100; ++i) {
				if(ConsoleCheckRxChar(0x03)) {
					abort = 1;
					gBootDelay = 0;
					break;
				}
				TimerDelayMs(10);
			}
			printf("\b\b\b%2d ", gBootDelay);
		}
		printf("\r\n");

		if (((gBootDelay == 0) || (gBootDelay > 0)) && abort) {
			loop = 1;
			RegisterMiscCommand();
			RegisterFactoryCommand();
			RegisterXmodemCommand();
			cli_set_prompt("(OB-Boot) >");
			while(loop) {
				char ch;
				ch = ConsoleGetChar();
				cli_char_input(ch);
			}
		} 
	}

	if(eeprom_read(EPROM_ADDR_UPGRADE_FLAG, &upgrade_flag, 1) == I2C_SUCCESS) {
		if(upgrade_flag == 0x80) {
			BootPrint("Detected upgrade flag bit\r\n");
			BootPrint("Verifing the image at 0x08080000 ... ");
			if(OB_Check_Upgrade_Image(0x8080000, &fmSize, &fmCrc32) == IHCHK_OK) {
				BootPrint("Passed\r\n");
				BootPrint("Erasing flash sector   ... ");
				FLASH_If_Init();
				FLASH_If_Erase(0x08010000, 0x0807FFFF);
				BootPrint("Done\r\n");
				BootPrint("Upgrading firmware     ... ");
				writeAddress = PRIMARY_PARTITION_ADDRESS;
				readAddress = SECOND_PARTITION_ADDRESS + sizeof(OB_Image_Header_t);
				if(FLASH_If_Write(&writeAddress, (uint32_t *)readAddress, fmSize) != 0) {
					BootPrint("Failed\r\n");
					NVIC_SystemReset();
				} else {
					BootPrint("Done\r\n");
					BootPrint("Verifing the image at 0x08010000 ... ");
					if(crc32(0, (unsigned char *)(PRIMARY_PARTITION_ADDRESS), fmSize) != fmCrc32) {
						BootPrint("Failed\r\n");
						NVIC_SystemReset();
					} else {	
						BootPrint("Passed\r\n");
						BootPrint("Firmware size: %d bytes, CRC-32: 0x%08X\r\n", fmSize, fmCrc32);
						
						/* Store the Fimware size and crc value to eeprom */
						fmSize = htonl(fmSize);
						fmCrc32 = htonl(fmCrc32);
						if(eeprom_write(EPROM_ADDR_FIRMWARE_SIZE, (u8 *)&fmSize, 4) == I2C_FAILURE) {
							BootPrint("Error: i2c write failed\r\n");
							NVIC_SystemReset();
						}
						TimerDelayMs(10);
						if(eeprom_write(EPROM_ADDR_FIRMWARE_CRC32, (u8 *)&fmCrc32, 4) == I2C_FAILURE){
							BootPrint("Error: i2c write failed\r\n");
							NVIC_SystemReset();
						}
						TimerDelayMs(10);
						
						/* Clear upgrade flag */
						BootPrint("Clear upgrade flag bit ... ");
						upgrade_flag = 0x00;
						if(eeprom_write(EPROM_ADDR_UPGRADE_FLAG, (u8 *)&upgrade_flag, 1) == I2C_FAILURE) {
							BootPrint("Failed, i2c write failed\r\n");
							NVIC_SystemReset();
						} else {
							BootPrint("Done\r\n");
						}
					}
				}
			} else {
				BootPrint("Failed\r\n");
				/* Clear upgrade flag */
				BootPrint("Clear upgrade flag bit ... ");
				upgrade_flag = 0x00;
				if(eeprom_write(EPROM_ADDR_UPGRADE_FLAG, (u8 *)&upgrade_flag, 1) == I2C_FAILURE) {
					BootPrint("Failed, i2c write failed\r\n");
				} else {
					BootPrint("Done\r\n");
				}				
				NVIC_SystemReset();
			}
		}
	}
	
	BootPrint("Now start application at 0x08010000 ...\r\n");

	if(gConsoleEnable)
		ConsoleDisable();
	TimerDisable();
	
	NVIC_SetVectorTable(NVIC_VectTab_FLASH , DEFAULT_APP_ENTRY_ADDRESS - NVIC_VectTab_FLASH);
	StartApp(DEFAULT_APP_ENTRY_ADDRESS);

	for(;;);
}



