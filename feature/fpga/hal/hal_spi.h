
#ifndef _HAL_SPI_H
#define _HAL_SPI_H


#include "stm32f2xx_gpio.h"
 
#define SPISLAMODE 1
#define SPIMASMODE 2

#define SPI_FLASH                 SPI1
#define SPI_FLASH_CLK             RCC_APB2Periph_SPI1
#define SPI_FLASH_GPIO            GPIOA
#define SPI_FLASH_GPIO_CLK        RCC_AHB1Periph_GPIOA  
#define SPI_FLASH_PIN_SCK         GPIO_Pin_5
#define SPI_FLASH_PIN_MISO        GPIO_Pin_6
#define SPI_FLASH_PIN_MOSI        GPIO_Pin_7

#define SPI_FLASH_CS           	  GPIO_Pin_4
#define SPI_FLASH_CS_GPIO      	  GPIOA
#define SPI_FLASH_CS_GPIO_CLK	  RCC_AHB1Periph_GPIOA

#define SPI_FLASH_CS_LOW()       GPIO_ResetBits(SPI_FLASH_CS_GPIO, SPI_FLASH_CS)
#define SPI_FLASH_CS_HIGH()      GPIO_SetBits(SPI_FLASH_CS_GPIO, SPI_FLASH_CS)

void SPI1Init(unsigned char);





#endif // end hal_spi.h
