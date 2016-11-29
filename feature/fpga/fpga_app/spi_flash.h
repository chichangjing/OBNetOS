/**
  ******************************************************************************
  * @file    spi_flash.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    18-April-2011
  * @brief   This file contains all the functions prototypes for the spi_flash
  *          firmware driver.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************  
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* M25P SPI Flash supported commands */  
#define sFLASH_CMD_WRITE          0x02  /* Write to Memory instruction */
#define sFLASH_CMD_WRSR           0x01  /* Write Status Register instruction */
#define sFLASH_CMD_WREN           0x06  /* Write enable instruction */
#define sFLASH_CMD_READ           0x03  /* Read from Memory instruction */
#define sFLASH_CMD_RDSR           0x05  /* Read Status Register instruction  */
#define sFLASH_CMD_RDID           0x9F  /* Read identification */
#define sFLASH_CMD_SE             0xD8  /* Sector Erase instruction */
#define sFLASH_CMD_BE             0xC7  /* Bulk Erase instruction */

#define sFLASH_WIP_FLAG           0x01  /* Write In Progress (WIP) flag */

#define sFLASH_DUMMY_BYTE         0xA5
#define sFLASH_SPI_PAGESIZE       0x100

#define sFLASH_M25P128_ID         0x202018
#define sFLASH_M25P64_ID          0x202017
	 

	 
/* M25P FLASH SPI Interface pins  */ 

//-------------------------------SPI3 Interface pins------------------------------------// 
#define SPI3_FLASH_SPI                           SPI3
#define SPI3_FLASH_SPI_CLK                       RCC_APB1Periph_SPI3
#define SPI3_FLASH_SPI_CLK_INIT                  RCC_APB1PeriphClockCmd

#define SPI3_FLASH_SPI_SCK_PIN                   GPIO_Pin_10
#define SPI3_FLASH_SPI_SCK_GPIO_PORT             GPIOC
#define SPI3_FLASH_SPI_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOC
#define SPI3_FLASH_SPI_SCK_SOURCE                GPIO_PinSource10
#define SPI3_FLASH_SPI_SCK_AF                    GPIO_AF_SPI3

#define SPI3_FLASH_SPI_MISO_PIN                  GPIO_Pin_11
#define SPI3_FLASH_SPI_MISO_GPIO_PORT            GPIOC
#define SPI3_FLASH_SPI_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOC
#define SPI3_FLASH_SPI_MISO_SOURCE               GPIO_PinSource11
#define SPI3_FLASH_SPI_MISO_AF                   GPIO_AF_SPI3

#define SPI3_FLASH_SPI_MOSI_PIN                  GPIO_Pin_5
#define SPI3_FLASH_SPI_MOSI_GPIO_PORT            GPIOB
#define SPI3_FLASH_SPI_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOB  /////////////////
#define SPI3_FLASH_SPI_MOSI_SOURCE               GPIO_PinSource5
#define SPI3_FLASH_SPI_MOSI_AF                   GPIO_AF_SPI3

#define SPI3_FLASH_CS_PIN                        GPIO_Pin_4
#define SPI3_FLASH_CS_GPIO_PORT                  GPIOA
#define SPI3_FLASH_CS_GPIO_CLK                   RCC_AHB1Periph_GPIOA
	 
/* Select sFLASH: Chip Select pin low */
#define SPI3_FLASH_CS_LOW()       GPIO_ResetBits(SPI3_FLASH_CS_GPIO_PORT, SPI3_FLASH_CS_PIN)
/* Deselect sFLASH: Chip Select pin high */
#define SPI3_FLASH_CS_HIGH()      GPIO_SetBits(SPI3_FLASH_CS_GPIO_PORT, SPI3_FLASH_CS_PIN)
//-------------------------------SPI3 Interface pins------------------------------------//

/* Exported macro ------------------------------------------------------------*/
/* Select sFLASH: Chip Select pin low */
//#define sFLASH_CS_LOW()       GPIO_ResetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN)
/* Deselect sFLASH: Chip Select pin high */
//#define sFLASH_CS_HIGH()      GPIO_SetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN)   

/* Exported functions ------------------------------------------------------- */

//-------------SPI3 functions------------------------------------------------------------------//
/* High layer functions  */
void SPI3_FLASH_DeInit(void);
void SPI3_FLASH_Init(void);

void SPI3_MCU_pin_to_3state(void);  //

void SPI3_FLASH_EraseSector(uint32_t SectorAddr);
void SPI3_FLASH_EraseBulk(void);
void SPI3_FLASH_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void SPI3_FLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void SPI3_FLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
uint32_t SPI3_FLASH_ReadID(void);
void SPI3_FLASH_StartReadSequence(uint32_t ReadAddr);

/* Low layer functions */
uint8_t SPI3_FLASH_ReadByte(void);
uint8_t SPI3_FLASH_SendByte(uint8_t byte);
uint16_t SPI3_FLASH_SendHalfWord(uint16_t HalfWord);
void SPI3_FLASH_WriteEnable(void);
void SPI3_FLASH_WaitForWriteEnd(void);
//-------------SPI3 functions------------------------------------------------------------------//
#ifdef __cplusplus
}
#endif

#endif /* __SPI_FLASH_H */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
