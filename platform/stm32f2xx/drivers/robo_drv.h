
/**
  ******************************************************************************
  * @file    robo_drv.h
  * @author  OB networks
  * @version v1.0.0
  * @date    2014-06-23
  * @brief   RoboSwitch SPI driver, ## pressure test passed ##
  ******************************************************************************
  */ 

#ifndef __ROBO_SPI_H
#define __ROBO_SPI_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "mconfig.h"

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#define ROBO_CMD_WREN					0x61  /* Write instruction */
#define ROBO_CMD_READ					0x60  /* Read instruction */

/* RoboSwitch SPI Interface pins  */  
/* GE20023MA : SCK=PC10, MISO=PC11, MOSI=PB5,  SS=PA4 */
/* GE1040PU  : SCK=PC10, MISO=PC11, MOSI=PC12, SS=PA4 */
/* GE2C400U  : SCK=PC10, MISO=PC11, MOSI=PC12, SS=PA4 */
#define ROBO_SPI						SPI3
#define ROBO_SPI_CLK					RCC_APB1Periph_SPI3
#define ROBO_SPI_CLK_INIT				RCC_APB1PeriphClockCmd

#define ROBO_SPI_SCK_PIN				GPIO_Pin_10
#define ROBO_SPI_SCK_GPIO_PORT			GPIOC
#define ROBO_SPI_SCK_GPIO_CLK			RCC_AHB1Periph_GPIOC
#define ROBO_SPI_SCK_SOURCE				GPIO_PinSource10
#define ROBO_SPI_SCK_AF					GPIO_AF_SPI3

#define ROBO_SPI_MISO_PIN				GPIO_Pin_11
#define ROBO_SPI_MISO_GPIO_PORT			GPIOC
#define ROBO_SPI_MISO_GPIO_CLK			RCC_AHB1Periph_GPIOC
#define ROBO_SPI_MISO_SOURCE			GPIO_PinSource11
#define ROBO_SPI_MISO_AF				GPIO_AF_SPI3

#if (BOARD_GE1040PU || BOARD_GE2C400U || BOARD_GE11014MA || BOARD_GE204P0U)
#define ROBO_SPI_MOSI_PIN				GPIO_Pin_12
#define ROBO_SPI_MOSI_GPIO_PORT			GPIOC
#define ROBO_SPI_MOSI_GPIO_CLK			RCC_AHB1Periph_GPIOC
#define ROBO_SPI_MOSI_SOURCE 			GPIO_PinSource12
#define ROBO_SPI_MOSI_AF				GPIO_AF_SPI3
#else
#define ROBO_SPI_MOSI_PIN				GPIO_Pin_5
#define ROBO_SPI_MOSI_GPIO_PORT			GPIOB
#define ROBO_SPI_MOSI_GPIO_CLK			RCC_AHB1Periph_GPIOB
#define ROBO_SPI_MOSI_SOURCE 			GPIO_PinSource5
#define ROBO_SPI_MOSI_AF				GPIO_AF_SPI3
#endif

#define ROBO_CS_PIN						GPIO_Pin_4
#define ROBO_CS_GPIO_PORT				GPIOA
#define ROBO_CS_GPIO_CLK 				RCC_AHB1Periph_GPIOA

#define ROBO_CS_LOW()					GPIO_ResetBits(ROBO_CS_GPIO_PORT, ROBO_CS_PIN)
#define ROBO_CS_HIGH()					GPIO_SetBits(ROBO_CS_GPIO_PORT, ROBO_CS_PIN)   

/* Exported functions ------------------------------------------------------- */

/* High layer functions  */

/* Low layer functions */
void robo_SpiInit(void);
int robo_read(u8 page, u8 addr, u8 *buf, u8 len);
int robo_write(u8 page, u8 addr, u8 *buf, u8 len);
int RoboSwitch_Init(unsigned char min_ver);

#ifdef __cplusplus
}
#endif

#endif /* __ROBO_SPI_H */


