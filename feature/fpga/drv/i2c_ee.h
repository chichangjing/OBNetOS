

#ifndef _I2C_EE_H
#define _I2C_EE_H


#include "stm32f2xx_gpio.h"
#include "stm32f2xx_rcc.h"




#define HAL_EEPROM


#define EEWPPORT		GPIOB
#define EEWPPIN			GPIO_Pin_8
#define EEWPPERIPH		RCC_AHB1Periph_GPIOB

#define EEWP1PORT		GPIOD
#define EEWP1PIN		GPIO_Pin_15
#define EEWP1PERIPH		RCC_AHB1Periph_GPIOD

#if 1
#define AT24_WPENA			(PINSET(EEWPPORT,EEWPPIN))
#define AT24_WPDIS			(PINCLR(EEWPPORT,EEWPPIN))
#define AT24_WP1ENA			(PINSET(EEWP1PORT,EEWP1PIN))
#define AT24_WP1DIS			(PINCLR(EEWP1PORT,EEWP1PIN))

#else
#define AT24_WPENA
#define AT24_WPDIS
#endif

#define E2DEVADD0 0xa2
#define E2DEVADD1 0xa0








typedef enum STORE_SIZE
{
    AT24C02	= 0x07,
    AT24C32	= 0x1f,
    AT24C64	= 0x3f,
    AT24C2F	= 0x7f
} storesize_e;



#define	CURRENTSIZE	AT24C64






void At24WpInit(void);
unsigned char At24_Byte_Write(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf);
unsigned char At24_Page_Write(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf, int size);
unsigned char At24_Current_Read(int devid, unsigned char devadd, unsigned char *buf);
unsigned char At24_Random_Read(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf);
unsigned char At24_Sequential_Read(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf, int size);





#endif  //end i2c_ee.h
