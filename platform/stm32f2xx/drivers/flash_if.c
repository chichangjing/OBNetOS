

/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"
#include "stdio.h"

#if defined(OS_FREERTOS)
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/


/* Private functions ---------------------------------------------------------*/
static uint32_t GetSector(uint32_t Address);


#if defined(OS_FREERTOS)
void flash_erase_delay_ms(int ms)
{
	vTaskDelay(ms);
}
#endif


/**
  * @brief  Unlocks Flash for write access
  * @param  None
  * @retval None
  */
void FLASH_If_Init(void)
{ 
  FLASH_Unlock(); 

  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
}

/**
  * @brief  This function does an erase of all user flash area
  * @param  StartSector: start of user flash area
  * @retval 0: user flash area successfully erased
  *         1: error occurred
  */
uint32_t FLASH_If_Erase(uint32_t StartAddress, uint32_t EndAddress)
{
	uint32_t StartSector, EndSector, i = 0;

	/* Get the sector where start the user flash area */
	StartSector = GetSector(StartAddress);
	EndSector = GetSector(EndAddress);
	
	for(i = StartSector; i <= EndSector; i += 8) {
		if (FLASH_EraseSector(i, VoltageRange_3) != FLASH_COMPLETE) {
			return (1);
		}
	}

	return (0);
}

/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  FlashAddress: pointer on start address for writing data buffer
  * @param  Data: pointer on data buffer
  * @param  DataLength: length of data buffer (unit is 8-bit bytes)   
  * @retval 0: Data successfully written to Flash memory
  *         1: Error occurred while writing data in Flash memory
  *         2: Written Data in flash memory is different from expected one
  */
uint32_t FLASH_If_Write(__IO uint32_t* FlashAddress, uint32_t* Data ,uint32_t DataLength)
{
	uint32_t i = 0;
    uint32_t status = 0;
	uint32_t dataInRam;
	u8 startNumber;
	uint32_t DataLength32 = DataLength;

	/*First bytes that are not 32bit align*/
	if(*FlashAddress%4){
		startNumber = 4-(*FlashAddress)%4;
        status = FLASH_If_Byte_Write(FlashAddress, (uint8_t *)Data, startNumber);
		if(status != 0)
            return status;
		DataLength32 = DataLength - startNumber;
		Data = (uint32_t *)((u32)Data + startNumber);
	}

	/*Program flash by words*/
	for (i = 0; (i < DataLength32/4) && (*FlashAddress <= (FLASH_END_ADDRESS-3)); i++) {
		dataInRam = *(Data+i);
		if (FLASH_ProgramWord(*FlashAddress, dataInRam) == FLASH_COMPLETE) {
			/* Check the written value */
			if (*(uint32_t*)*FlashAddress != dataInRam) {
				return(2);
			}
			/* Increment FLASH destination address */
			*FlashAddress += 4;
		} else {
			/* Error occurred while writing data in Flash memory */
			return (1);
		}
	}

	/*Last bytes that cannot be write by a 32 bit word*/
	status = FLASH_If_Byte_Write(FlashAddress, (uint8_t *)Data + i*4, DataLength32-i*4);
    if(status != 0)
        return status;

	return (0);
}


uint32_t FLASH_If_Byte_Write(__IO uint32_t* FlashAddress, uint8_t* Data ,uint32_t DataLength)
{
	uint32_t i = 0;
	uint32_t dataInRam;

	for (i = 0; (i < DataLength) && (*FlashAddress <= (FLASH_END_ADDRESS)); i++) {
		dataInRam = *(uint8_t*)(Data+i);
		if (FLASH_ProgramByte(*FlashAddress, dataInRam) == FLASH_COMPLETE) {
			/* Check the written value */
			if (*(uint8_t*)*FlashAddress != dataInRam) {
				/* Flash content doesn't match SRAM content */
				return(2);
			}
			/* Increment FLASH destination address */
			*FlashAddress +=1;
		} else {
			/* Error occurred while writing data in Flash memory */
			return (1);
		}
	}

	return (0);
}

/**
  * @brief  Gets the sector of a given address
  * @param  Address: Flash address
  * @retval The sector of a given address
  */
static uint32_t GetSector(uint32_t Address)
{
	uint32_t sector = 0;

	if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
		sector = FLASH_Sector_0;  
	else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
		sector = FLASH_Sector_1;  
	else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
		sector = FLASH_Sector_2;  
	else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
		sector = FLASH_Sector_3;  
	else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
		sector = FLASH_Sector_4;  
	else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
		sector = FLASH_Sector_5;  
	else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
		sector = FLASH_Sector_6;  
	else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
		sector = FLASH_Sector_7;  
	else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
		sector = FLASH_Sector_8;  
	else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
		sector = FLASH_Sector_9;  
	else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
		sector = FLASH_Sector_10;  
	else
		sector = FLASH_Sector_11;  

	return sector;
}

