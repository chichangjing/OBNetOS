/**
  ******************************************************************************
  * @file    SPI/SPI_FLASH/spi_flash.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    18-April-2011
  * @brief   This file provides a set of functions needed to manage the SPI M25Pxxx
  *          FLASH memory. 
  *            
  *          ===================================================================      
  *          Notes: 
  *           - There is no SPI FLASH memory available in STM322xG-EVAL board,
  *             to use this driver you have to build your own hardware.     
  *          ===================================================================
  *   
  *          It implements a high level communication layer for read and write 
  *          from/to this memory. The needed STM32 hardware resources (SPI and 
  *          GPIO) are defined in spi_flash.h file, and the initialization is 
  *          performed in sFLASH_LowLevel_Init() function.
  *            
  *          You can easily tailor this driver to any development board, by just
  *          adapting the defines for hardware resources and sFLASH_LowLevel_Init()
  *          function.
  *            
  *          +-----------------------------------------------------------+
  *          |                     Pin assignment                        |
  *          +-----------------------------+---------------+-------------+
  *          |  STM32 SPI Pins             |     sFLASH    |    Pin      |
  *          +-----------------------------+---------------+-------------+
  *          | sFLASH_CS_PIN               | ChipSelect(/S)|    1        |
  *          | sFLASH_SPI_MISO_PIN / MISO  |   DataOut(Q)  |    2        |
  *          |                             |   VCC         |    3 (3.3 V)|
  *          |                             |   GND         |    4 (0 V)  |
  *          | sFLASH_SPI_MOSI_PIN / MOSI  |   DataIn(D)   |    5        |
  *          | sFLASH_SPI_SCK_PIN / SCK    |   Clock(C)    |    6        |
  *          |                             |    VCC        |    7 (3.3 V)|
  *          |                             |    VCC        |    8 (3.3 V)|  
  *          +-----------------------------+---------------+-------------+  
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

/* Includes ------------------------------------------------------------------*/
#include "spi_flash.h"

/**
  * @brief  Erases the specified FLASH sector.
  * @param  SectorAddr: address of the sector to erase.
  * @retval None
  */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void SPI3_FLASH_LowLevel_DeInit(void);
void SPI3_FLASH_LowLevel_Init(void); 

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  DeInitializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void SPI3_FLASH_DeInit(void)
{
  SPI3_FLASH_LowLevel_DeInit();
}

/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void SPI3_FLASH_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;

  SPI3_FLASH_LowLevel_Init();
    
  /*!< Deselect the FLASH: Chip Select high */
  SPI3_FLASH_CS_HIGH();

  /*!< SPI configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;

  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI3_FLASH_SPI, &SPI_InitStructure);

  /*!< Enable the sFLASH_SPI  */
  SPI_Cmd(SPI3_FLASH_SPI, ENABLE);
}

/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver to ÊäÈëÐü¿Õ.
  * @param  None
  * @retval None
  */
void SPI3_MCU_pin_to_3state(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  /*!< Disable the sFLASH_SPI  ************************************************/
  SPI_Cmd(SPI3_FLASH_SPI, DISABLE);
  
  /*!< DeInitializes the sFLASH_SPI *******************************************/
  SPI_I2S_DeInit(SPI3_FLASH_SPI);
  
  /*!< sFLASH_SPI Periph clock disable ****************************************/
  SPI3_FLASH_SPI_CLK_INIT(SPI3_FLASH_SPI_CLK, DISABLE);///////////////////////
      
  /*!< Configure all pins used by the SPI as input floating *******************/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  GPIO_InitStructure.GPIO_Pin = SPI3_FLASH_SPI_SCK_PIN;
  GPIO_Init(SPI3_FLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = SPI3_FLASH_SPI_MISO_PIN;
  GPIO_Init(SPI3_FLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = SPI3_FLASH_SPI_MOSI_PIN;
  GPIO_Init(SPI3_FLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = SPI3_FLASH_CS_PIN;
  GPIO_Init(SPI3_FLASH_CS_GPIO_PORT, &GPIO_InitStructure);

}

/**
  * @brief  Erases the specified FLASH sector.
  * @param  SectorAddr: address of the sector to erase.
  * @retval None
  */
void SPI3_FLASH_EraseSector(uint32_t SectorAddr)
{
  /*!< Send write enable instruction */
  SPI3_FLASH_WriteEnable();

  /*!< Sector Erase */
  /*!< Select the FLASH: Chip Select low */
  SPI3_FLASH_CS_LOW();
  /*!< Send Sector Erase instruction */
  SPI3_FLASH_SendByte(sFLASH_CMD_SE);
  /*!< Send SectorAddr high nibble address byte */
  SPI3_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
  /*!< Send SectorAddr medium nibble address byte */
  SPI3_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
  /*!< Send SectorAddr low nibble address byte */
  SPI3_FLASH_SendByte(SectorAddr & 0xFF);
  /*!< Deselect the FLASH: Chip Select high */
  SPI3_FLASH_CS_HIGH();

  /*!< Wait the end of Flash writing */
  SPI3_FLASH_WaitForWriteEnd();
}

/**
  * @brief  Erases the entire FLASH.
  * @param  None
  * @retval None
  */
void SPI3_FLASH_EraseBulk(void)
{
  /*!< Send write enable instruction */
  SPI3_FLASH_WriteEnable();

  /*!< Bulk Erase */
  /*!< Select the FLASH: Chip Select low */
  SPI3_FLASH_CS_LOW();
  /*!< Send Bulk Erase instruction  */
  SPI3_FLASH_SendByte(sFLASH_CMD_BE);
  /*!< Deselect the FLASH: Chip Select high */
  SPI3_FLASH_CS_HIGH();

  /*!< Wait the end of Flash writing */
  SPI3_FLASH_WaitForWriteEnd();
}

/**
  * @brief  Writes more than one byte to the FLASH with a single WRITE cycle 
  *         (Page WRITE sequence).
  * @note   The number of byte can't exceed the FLASH page size.
  * @param  pBuffer: pointer to the buffer  containing the data to be written
  *         to the FLASH.
  * @param  WriteAddr: FLASH's internal address to write to.
  * @param  NumByteToWrite: number of bytes to write to the FLASH, must be equal
  *         or less than "sFLASH_PAGESIZE" value.
  * @retval None
  */
void SPI3_FLASH_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  /*!< Enable the write access to the FLASH */
  SPI3_FLASH_WriteEnable();

  /*!< Select the FLASH: Chip Select low */
  SPI3_FLASH_CS_LOW();
  /*!< Send "Write to Memory " instruction */
  SPI3_FLASH_SendByte(sFLASH_CMD_WRITE);
  /*!< Send WriteAddr high nibble address byte to write to */
  SPI3_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /*!< Send WriteAddr medium nibble address byte to write to */
  SPI3_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
  /*!< Send WriteAddr low nibble address byte to write to */
  SPI3_FLASH_SendByte(WriteAddr & 0xFF);

  /*!< while there is data to be written on the FLASH */
  while (NumByteToWrite--)
  {
    /*!< Send the current byte */
    SPI3_FLASH_SendByte(*pBuffer);
    /*!< Point on the next byte to be written */
    pBuffer++;
  }

  /*!< Deselect the FLASH: Chip Select high */
  SPI3_FLASH_CS_HIGH();

  /*!< Wait the end of Flash writing */
  SPI3_FLASH_WaitForWriteEnd();
}

/**
  * @brief  Writes block of data to the FLASH. In this function, the number of
  *         WRITE cycles are reduced, using Page WRITE sequence.
  * @param  pBuffer: pointer to the buffer  containing the data to be written
  *         to the FLASH.
  * @param  WriteAddr: FLASH's internal address to write to.
  * @param  NumByteToWrite: number of bytes to write to the FLASH.
  * @retval None
  */
void SPI3_FLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

  Addr = WriteAddr % sFLASH_SPI_PAGESIZE;
  count = sFLASH_SPI_PAGESIZE - Addr;
  NumOfPage =  NumByteToWrite / sFLASH_SPI_PAGESIZE;
  NumOfSingle = NumByteToWrite % sFLASH_SPI_PAGESIZE;

  if (Addr == 0) /*!< WriteAddr is sFLASH_PAGESIZE aligned  */
  {
    if (NumOfPage == 0) /*!< NumByteToWrite < sFLASH_PAGESIZE */
    {
      SPI3_FLASH_WritePage(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /*!< NumByteToWrite > sFLASH_PAGESIZE */
    {
      while (NumOfPage--)
      {
        SPI3_FLASH_WritePage(pBuffer, WriteAddr, sFLASH_SPI_PAGESIZE);
        WriteAddr +=  sFLASH_SPI_PAGESIZE;
        pBuffer += sFLASH_SPI_PAGESIZE;
      }

      SPI3_FLASH_WritePage(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /*!< WriteAddr is not sFLASH_PAGESIZE aligned  */
  {
    if (NumOfPage == 0) /*!< NumByteToWrite < sFLASH_PAGESIZE */
    {
      if (NumOfSingle > count) /*!< (NumByteToWrite + WriteAddr) > sFLASH_PAGESIZE */
      {
        temp = NumOfSingle - count;

        SPI3_FLASH_WritePage(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        SPI3_FLASH_WritePage(pBuffer, WriteAddr, temp);
      }
      else
      {
        SPI3_FLASH_WritePage(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /*!< NumByteToWrite > sFLASH_PAGESIZE */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / sFLASH_SPI_PAGESIZE;
      NumOfSingle = NumByteToWrite % sFLASH_SPI_PAGESIZE;

      SPI3_FLASH_WritePage(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        SPI3_FLASH_WritePage(pBuffer, WriteAddr, sFLASH_SPI_PAGESIZE);
        WriteAddr +=  sFLASH_SPI_PAGESIZE;
        pBuffer += sFLASH_SPI_PAGESIZE;
      }

      if (NumOfSingle != 0)
      {
        SPI3_FLASH_WritePage(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

/**
  * @brief  Reads a block of data from the FLASH.
  * @param  pBuffer: pointer to the buffer that receives the data read from the FLASH.
  * @param  ReadAddr: FLASH's internal address to read from.
  * @param  NumByteToRead: number of bytes to read from the FLASH.
  * @retval None
  */
void SPI3_FLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
  /*!< Select the FLASH: Chip Select low */
  SPI3_FLASH_CS_LOW();

  /*!< Send "Read from Memory " instruction */
  SPI3_FLASH_SendByte(sFLASH_CMD_READ);

  /*!< Send ReadAddr high nibble address byte to read from */
  SPI3_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /*!< Send ReadAddr medium nibble address byte to read from */
  SPI3_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /*!< Send ReadAddr low nibble address byte to read from */
  SPI3_FLASH_SendByte(ReadAddr & 0xFF);

  while (NumByteToRead--) /*!< while there is data to be read */
  {
    /*!< Read a byte from the FLASH */
    *pBuffer = SPI3_FLASH_SendByte(sFLASH_DUMMY_BYTE);
    /*!< Point to the next location where the byte read will be saved */
    pBuffer++;
  }

  /*!< Deselect the FLASH: Chip Select high */
  SPI3_FLASH_CS_HIGH();
}

/**
  * @brief  Reads FLASH identification.
  * @param  None
  * @retval FLASH identification
  */
uint32_t SPI3_FLASH_ReadID(void)
{
  uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

  /*!< Select the FLASH: Chip Select low */
  SPI3_FLASH_CS_LOW();

  /*!< Send "RDID " instruction */
  SPI3_FLASH_SendByte(0x9F);

  /*!< Read a byte from the FLASH */
  Temp0 = SPI3_FLASH_SendByte(sFLASH_DUMMY_BYTE);

  /*!< Read a byte from the FLASH */
  Temp1 = SPI3_FLASH_SendByte(sFLASH_DUMMY_BYTE);

  /*!< Read a byte from the FLASH */
  Temp2 = SPI3_FLASH_SendByte(sFLASH_DUMMY_BYTE);

  /*!< Deselect the FLASH: Chip Select high */
  SPI3_FLASH_CS_HIGH();

  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

  return Temp;
}

/**
  * @brief  Initiates a read data byte (READ) sequence from the Flash.
  *   This is done by driving the /CS line low to select the device, then the READ
  *   instruction is transmitted followed by 3 bytes address. This function exit
  *   and keep the /CS line low, so the Flash still being selected. With this
  *   technique the whole content of the Flash is read with a single READ instruction.
  * @param  ReadAddr: FLASH's internal address to read from.
  * @retval None
  */
void SPI3_FLASH_StartReadSequence(uint32_t ReadAddr)
{
  /*!< Select the FLASH: Chip Select low */
  SPI3_FLASH_CS_LOW();

  /*!< Send "Read from Memory " instruction */
  SPI3_FLASH_SendByte(sFLASH_CMD_READ);

  /*!< Send the 24-bit address of the address to read from -------------------*/
  /*!< Send ReadAddr high nibble address byte */
  SPI3_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /*!< Send ReadAddr medium nibble address byte */
  SPI3_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /*!< Send ReadAddr low nibble address byte */
  SPI3_FLASH_SendByte(ReadAddr & 0xFF);
}

/**
  * @brief  Reads a byte from the SPI Flash.
  * @note   This function must be used only if the Start_Read_Sequence function
  *         has been previously called.
  * @param  None
  * @retval Byte Read from the SPI Flash.
  */
uint8_t SPI3_FLASH_ReadByte(void)
{
  return (SPI3_FLASH_SendByte(sFLASH_DUMMY_BYTE));
}

/**
  * @brief  Sends a byte through the SPI interface and return the byte received
  *         from the SPI bus.
  * @param  byte: byte to send.
  * @retval The value of the received byte.
  */
uint8_t SPI3_FLASH_SendByte(uint8_t byte)
{
  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI3_FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(SPI3_FLASH_SPI, byte);

  /*!< Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(SPI3_FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /*!< Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI3_FLASH_SPI);
}

/**
  * @brief  Sends a Half Word through the SPI interface and return the Half Word
  *         received from the SPI bus.
  * @param  HalfWord: Half Word to send.
  * @retval The value of the received Half Word.
  */
uint16_t SPI3_FLASH_SendHalfWord(uint16_t HalfWord)
{
  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI3_FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send Half Word through the sFLASH peripheral */
  SPI_I2S_SendData(SPI3_FLASH_SPI, HalfWord);

  /*!< Wait to receive a Half Word */
  while (SPI_I2S_GetFlagStatus(SPI3_FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /*!< Return the Half Word read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI3_FLASH_SPI);
}

/**
  * @brief  Enables the write access to the FLASH.
  * @param  None
  * @retval None
  */
void SPI3_FLASH_WriteEnable(void)
{
  /*!< Select the FLASH: Chip Select low */
  SPI3_FLASH_CS_LOW();

  /*!< Send "Write Enable" instruction */
  SPI3_FLASH_SendByte(sFLASH_CMD_WREN);

  /*!< Deselect the FLASH: Chip Select high */
  SPI3_FLASH_CS_HIGH();
}

/**
  * @brief  Polls the status of the Write In Progress (WIP) flag in the FLASH's
  *         status register and loop until write opertaion has completed.
  * @param  None
  * @retval None
  */
void SPI3_FLASH_WaitForWriteEnd(void)
{
  uint8_t flashstatus = 0;

  /*!< Select the FLASH: Chip Select low */
  SPI3_FLASH_CS_LOW();

  /*!< Send "Read Status Register" instruction */
  SPI3_FLASH_SendByte(sFLASH_CMD_RDSR);

  /*!< Loop as long as the memory is busy with a write cycle */
  do
  {
    /*!< Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    flashstatus = SPI3_FLASH_SendByte(sFLASH_DUMMY_BYTE);

  }
  while ((flashstatus & sFLASH_WIP_FLAG) == SET); /* Write in progress */

  /*!< Deselect the FLASH: Chip Select high */
  SPI3_FLASH_CS_HIGH();
}

/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void SPI3_FLASH_LowLevel_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< Enable the SPI clock */
  SPI3_FLASH_SPI_CLK_INIT(SPI3_FLASH_SPI_CLK, ENABLE);

  /*!< Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd(SPI3_FLASH_SPI_SCK_GPIO_CLK | SPI3_FLASH_SPI_MISO_GPIO_CLK | 
                         SPI3_FLASH_SPI_MOSI_GPIO_CLK | SPI3_FLASH_CS_GPIO_CLK, ENABLE);
  
  /*!< SPI pins configuration *************************************************/

  /*!< Connect SPI pins to AF5 */  
  GPIO_PinAFConfig(SPI3_FLASH_SPI_SCK_GPIO_PORT, SPI3_FLASH_SPI_SCK_SOURCE, SPI3_FLASH_SPI_SCK_AF);
  GPIO_PinAFConfig(SPI3_FLASH_SPI_MISO_GPIO_PORT, SPI3_FLASH_SPI_MISO_SOURCE, SPI3_FLASH_SPI_MISO_AF);
  GPIO_PinAFConfig(SPI3_FLASH_SPI_MOSI_GPIO_PORT, SPI3_FLASH_SPI_MOSI_SOURCE, SPI3_FLASH_SPI_MOSI_AF);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
        
  /*!< SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = SPI3_FLASH_SPI_SCK_PIN;
  GPIO_Init(SPI3_FLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /*!< SPI MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  SPI3_FLASH_SPI_MOSI_PIN;
  GPIO_Init(SPI3_FLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /*!< SPI MISO pin configuration */
  GPIO_InitStructure.GPIO_Pin =  SPI3_FLASH_SPI_MISO_PIN;
  GPIO_Init(SPI3_FLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure sFLASH Card CS pin in output pushpull mode ********************/
  GPIO_InitStructure.GPIO_Pin = SPI3_FLASH_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(SPI3_FLASH_CS_GPIO_PORT, &GPIO_InitStructure);
}

/**
  * @brief  DeInitializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void SPI3_FLASH_LowLevel_DeInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< Disable the sFLASH_SPI  ************************************************/
  SPI_Cmd(SPI3_FLASH_SPI, DISABLE);
  
  /*!< DeInitializes the sFLASH_SPI *******************************************/
  SPI_I2S_DeInit(SPI3_FLASH_SPI);
  
  /*!< sFLASH_SPI Periph clock disable ****************************************/
  SPI3_FLASH_SPI_CLK_INIT(SPI3_FLASH_SPI_CLK, DISABLE);
      
  /*!< Configure all pins used by the SPI as input floating *******************/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  GPIO_InitStructure.GPIO_Pin = SPI3_FLASH_SPI_SCK_PIN;
  GPIO_Init(SPI3_FLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = SPI3_FLASH_SPI_MISO_PIN;
  GPIO_Init(SPI3_FLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = SPI3_FLASH_SPI_MOSI_PIN;
  GPIO_Init(SPI3_FLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = SPI3_FLASH_CS_PIN;
  GPIO_Init(SPI3_FLASH_CS_GPIO_PORT, &GPIO_InitStructure);
}

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
