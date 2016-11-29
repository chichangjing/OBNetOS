/*******************************************************************************
  * @file    Bootloader.c
  * @author  OB T&C Development Team
  * @brief   Bootloader
  ******************************************************************************/

#include "loader_config.h"

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
#include "cmd_misc.h"
#include "cmd_bdinfo.h"

#define DEFAULT_APP_ENTRY_ADDRESS	((uint32_t)0x08010000)
#define PRIMARY_PARTITION_ADDRESS	((uint32_t)0x08010000)
#define SECOND_PARTITION_ADDRESS	((uint32_t)0x08080000)

#define BootPrint(fmt, args...) do { if(gConsoleEnable) printf(fmt, ##args); } while(0)

unsigned char gBootDelay;
unsigned char gConsoleEnable;
char BootloaderVersion[MAX_LOADER_VERSION_SIZE] = LOADER_VERSION;
#if BOARD_GE22103MA
u8 gPcbVer87 = 0;
/* Check GE-2A005U PCB address */
u16 gPcbAddr = 0;
#endif

typedef void pFunction_t(void);

void StartApp(uint32_t EntryAddress)
{
	pFunction_t * AppEntry;
	
	__set_MSP(*(__IO uint32_t *)EntryAddress);
	AppEntry = ((pFunction_t *)(*((uint32_t *)(EntryAddress + 4))));
	AppEntry();
}


void GpioInit(void)
{
#if BOARD_GE20023MA
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
	
#elif BOARD_GE22103MA
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* HWV8, HWV7  = PD4, PD3 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_3;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
    
	/* U5_ADDR1, U5_ADDR2, U5_ADDR3, U5_ADDR4, U5_ADDR5, U5_ADDR6,U5_ADDR7 =
       PE9,     PE10,     PE11,     PE12,     PE13,     PE14,    PE15 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | 
        GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	gPcbVer87  = (u8)((GPIOD->IDR & GPIO_Pin_3) >> 3);
	gPcbVer87 |= (u8)((GPIOD->IDR & GPIO_Pin_4) >> 3);
    /* GE-2A005U PCB address, bit0...bit7 = ADDR1...ADDR8 */
    gPcbAddr  = (u16)(((GPIOE->IDR & GPIO_Pin_9) >> 9) & 0x01) << 0;
	gPcbAddr |= (u16)(((GPIOE->IDR & GPIO_Pin_10) >> 10) & 0x01) << 1;
    gPcbAddr |= (u16)(((GPIOE->IDR & GPIO_Pin_11) >> 11) & 0x01) << 2;
    gPcbAddr |= (u16)(((GPIOE->IDR & GPIO_Pin_12) >> 12) & 0x01) << 3;
    gPcbAddr |= (u16)(((GPIOE->IDR & GPIO_Pin_13) >> 13) & 0x01) << 4;
    gPcbAddr |= (u16)(((GPIOE->IDR & GPIO_Pin_14) >> 14) & 0x01) << 5;
    gPcbAddr |= (u16)(((GPIOE->IDR & GPIO_Pin_15) >> 15) & 0x01) << 6;
	
#endif
}

BOOL ReadSW1(void)
{
#if BOARD_GE20023MA
	if ((GPIOD->IDR & GPIO_Pin_1) != (unsigned int)Bit_RESET)
		return TRUE;
	else
		return FALSE;
#else
	return TRUE;
#endif
}

void FPGA_UpgradeGpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* FPGA UPGRADE pins configuration ************************************************/
   /*
            init -------------------------> PC8
            program ----------------------> PC7
            done -------------------------> PC6
            SPI_CS -----------------------> PA4
            SPI_SCK ----------------------> PC10
            SPI_MOSI ---------------------> PB5
            SPI_MISO ---------------------> PC11

                                                  */

    /* Configure PC6, PC7, PC8, PC10 and PC11 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_11 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    /* Configure PA4 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* Configure PB5 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void FPGA_LoadCheck(void)
{
    u8 bitstatus = 0;
    
    for(;;)
    {   /* waiting FPGA loader program complete */ 
        bitstatus = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6);
        if(bitstatus == 0)
        {
            GPIO_ToggleBits(GPIOB, GPIO_Pin_14);
            TimerDelayMs(150);
        } else {
            GPIO_ResetBits(GPIOB, GPIO_Pin_14);
            break;
        }
    }
    
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
	GpioInit();
	LED_GPIO_config();
#if BOARD_GV3S_HONUE_QM
    FPGA_UpgradeGpioInit();
    FPGA_LoadCheck();
#endif
	I2C_GPIO_Config();

	if(ReadSW1() == FALSE) {	/* SW1 is DIP on */
		gBootDelay = 3;
		gConsoleEnable = 1;
	} else {	
		if(eeprom_read(EPROM_ADDR_BOOT_DELAY, &bootdelay, 1) == I2C_FAILURE) {
			gBootDelay = 3;
			goto exit;
		} else {
			if(((bootdelay == 0) || (bootdelay > 0)) && (bootdelay <= 5))
				gBootDelay = bootdelay;
			else
				gBootDelay = 1;
		}	
		if(eeprom_read(EPROM_ADDR_CONSOLE_ENABLE, &console_enable, 1) == I2C_FAILURE) {
			gConsoleEnable = 1;
			goto exit;
		} else {
			if((console_enable == 0) || (console_enable == 1))
				gConsoleEnable = console_enable;
			else
				gConsoleEnable = 1;
		}
	}

	if(eeprom_read(EPROM_ADDR_LOADER_VERSION, (u8 *)&loader_ver, MAX_LOADER_VERSION_SIZE) != I2C_SUCCESS) {
		memset(loader_ver, 0, MAX_LOADER_VERSION_SIZE);
        goto exit;
	} else {
		if(memcmp(loader_ver, BootloaderVersion, MAX_LOADER_VERSION_SIZE) != 0) {
			memcpy(loader_ver, BootloaderVersion, MAX_LOADER_VERSION_SIZE);
			eeprom_write(EPROM_ADDR_LOADER_VERSION, (u8 *)&loader_ver, MAX_LOADER_VERSION_SIZE);
		}
	}

	if(gConsoleEnable > 0) {
#if BOARD_GE22103MA
        if(gPcbAddr == 0x007F){
            if(gPcbVer87 != 0x03)
                ConsoleInit(COM_USART3, 115200);
            else
                ConsoleInit(COM_UART5, 115200);
        }else{
            ConsoleInit(COM_USART3, 115200);
        }
#elif BOARD_GV3S_HONUE_QM
        ConsoleInit(COM_USART3, 115200);
#else
  		ConsoleInit(CONSOLE_PORT, 115200);
#endif
	}

  	BootPrint("\r\n\r\n\r\n\r");
    
	BootPrint("## %s bootloader v%s ##\r\n\r\n", BOARD_NAME, BootloaderVersion);

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
	
    /* Check if valid stack address (RAM address) then jump to user application */
    if (((*(__IO uint32_t*)DEFAULT_APP_ENTRY_ADDRESS) & 0x2FFE0000 ) != 0x20000000)
        goto exit;
	NVIC_SetVectorTable(NVIC_VectTab_FLASH , DEFAULT_APP_ENTRY_ADDRESS - NVIC_VectTab_FLASH);
    StartApp(DEFAULT_APP_ENTRY_ADDRESS);

exit:
#if BOARD_GE22103MA || BOARD_GV3S_HONUE_QM || BOARD_GE11500MD
	TimerEnable();
	for(;;) {
#if BOARD_GV3S_HONUE_QM || BOARD_GE11500MD
        TimerDelayMs(150);
		GPIO_ToggleBits(GPIOB, GPIO_Pin_15);	
		TimerDelayMs(150);
		GPIO_ToggleBits(GPIOB, GPIO_Pin_15);		
		TimerDelayMs(150);
		GPIO_ToggleBits(GPIOB, GPIO_Pin_15);		
		TimerDelayMs(150);
		GPIO_ToggleBits(GPIOB, GPIO_Pin_15);		
		
		TimerDelayMs(300);
		GPIO_ToggleBits(GPIOB, GPIO_Pin_15);	
		TimerDelayMs(150);
		GPIO_ToggleBits(GPIOB, GPIO_Pin_15);	
#else
		TimerDelayMs(150);
		GPIO_ToggleBits(GPIOE, GPIO_Pin_9);	
		TimerDelayMs(150);
		GPIO_ToggleBits(GPIOE, GPIO_Pin_9);	
		TimerDelayMs(150);
		GPIO_ToggleBits(GPIOE, GPIO_Pin_9);	
		TimerDelayMs(150);
		GPIO_ToggleBits(GPIOE, GPIO_Pin_9);	
		
		TimerDelayMs(300);
		GPIO_ToggleBits(GPIOE, GPIO_Pin_9);
		TimerDelayMs(150);
		GPIO_ToggleBits(GPIOE, GPIO_Pin_9);	
#endif
	}
#else
	for(;;);
#endif
}



