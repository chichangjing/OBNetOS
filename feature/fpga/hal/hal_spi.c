

#include "hal_spi.h"





void SPI1Init(unsigned char spimode)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(SPI_FLASH_GPIO_CLK | SPI_FLASH_CS_GPIO_CLK, ENABLE);
//	RCC_APB2PeriphClockCmd(SPI_FLASH_CLK , ENABLE);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
//	SPI_Init(SPI_FLASH, &SPI_InitStructure);
		
	if(spimode == SPISLAMODE){
#if 0
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		
		GPIO_InitStructure.GPIO_Pin = SPI_FLASH_PIN_MISO;
		GPIO_Init(SPI_FLASH_GPIO, &GPIO_InitStructure);
#endif

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
//		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

		GPIO_InitStructure.GPIO_Pin = SPI_FLASH_PIN_MOSI | SPI_FLASH_PIN_SCK | SPI_FLASH_CS | SPI_FLASH_PIN_MISO;
		GPIO_Init(SPI_FLASH_GPIO, &GPIO_InitStructure);

		SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
	}
	else if(spimode == SPIMASMODE){
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Pin = SPI_FLASH_PIN_MOSI | SPI_FLASH_PIN_SCK;
		GPIO_Init(SPI_FLASH_GPIO, &GPIO_InitStructure);
			
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = SPI_FLASH_PIN_MISO;
		GPIO_Init(SPI_FLASH_GPIO, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = SPI_FLASH_CS;
		GPIO_Init(SPI_FLASH_CS_GPIO, &GPIO_InitStructure);
//		SPI_FLASH_CS_LOW();

		SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	}
//	SPI_Cmd(SPI_FLASH, ENABLE);
}

//end hal_spi.c
