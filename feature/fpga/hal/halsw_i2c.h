
#ifndef HALSW_I2C_H
#define HALSW_I2C_H

#include "stm32f2xx.h"
#include "stm32f2xx_gpio.h"

typedef enum DEV_NUM
{
    I2CDEV_0 = 0,
    I2CDEV_1
} i2cdevid_e;

typedef enum ACK_STATE
{
    I2C_ACK_OK = 0,
    I2C_NO_ACK,
    I2C_SDA_DOWN,
    I2C_IDLE_OK = I2C_ACK_OK,
    I2C_NO_IDLE = 0xFF
} ack_state;

typedef enum OP_FLAG
{
    I2C_W = 0,
    I2C_R
} op_flag;

typedef enum
{
    SPEED_400K	= 5,
    SPEED_200K	= 10,
    SPEED_10K	= 100,
    SPEED_4K	= 250
} speedmode_e;

#define NONEDEV		0

#define HAL_SWI2C0	1
#define HAL_SWI2C1	NONEDEV

#define DEFSPEED	SPEED_400K

/* port op */
#define  PINSET(port,pin)		(port->BSRRL = pin)
#define  PINCLR(port,pin)		(port->BSRRH = pin)
#define  PINREAD(port,pin)	((port->IDR & pin)? 1 : 0)

/* software_i2c define */
/* swI2c port */
#define	SCL0PIN			GPIO_Pin_6
#define	SCL0PORT		GPIOB
#define	SCL0PERIPH		RCC_AHB1Periph_GPIOB

#define	SDA0PIN			GPIO_Pin_7
#define	SDA0PORT		GPIOB
#define	SDA0PERIPH		RCC_AHB1Periph_GPIOB

#define	SCL1PIN			GPIO_Pin_2
#define	SCL1PORT		GPIOE
#define	SCL1PERIPH		RCC_AHB1Periph_GPIOE

#define	SDA1PIN			GPIO_Pin_3
#define	SDA1PORT		GPIOE
#define	SDA1PERIPH		RCC_AHB1Periph_GPIOE

#define SW_SCL0SET			(PINSET(SCL0PORT,SCL0PIN))
#define SW_SCL0CLR			(PINCLR(SCL0PORT,SCL0PIN))
#define SW_SDA0SET			(PINSET(SDA0PORT,SDA0PIN))
#define SW_SDA0CLR			(PINCLR(SDA0PORT,SDA0PIN))

#define SW_SCL1SET			(PINSET(SCL1PORT,SCL1PIN))
#define SW_SCL1CLR			(PINCLR(SCL1PORT,SCL1PIN))
#define SW_SDA1SET			(PINSET(SDA1PORT,SDA1PIN))
#define SW_SDA1CLR			(PINCLR(SDA1PORT,SDA1PIN))

#define SW_SCL0READ		(PINREAD(SCL0PORT,SCL0PIN))
#define SW_SDA0READ		(PINREAD(SDA0PORT,SDA0PIN))
#define SW_SCL1READ		(PINREAD(SCL1PORT,SCL1PIN))
#define SW_SDA1READ		(PINREAD(SDA1PORT,SDA1PIN))
/* end define */

void SWI2cSpeedSet(int devid, speedmode_e speed);
int SWI2cSpeedGet(int devid);
void SW_I2CInit(int devid, unsigned char speed);
void SW_I2CBusstart(int devid);
void SW_I2CBusStop(int devid);
unsigned char SW_GetI2CState(int devid);
unsigned char SW_CheakI2CState(int devid);
unsigned char SW_I2CBusIdle(int devid);
unsigned char SW_I2CWriteByte(int devid, unsigned char data);
unsigned char SW_I2CReadByte(int devid, unsigned char ack);
unsigned int SWI2cCurrentWrite(int devid, unsigned char devadd, unsigned char *buf);
unsigned int SWI2cCurrentRead(int devid, unsigned char devadd, unsigned char *buf);
unsigned int SWI2cRandomWrite(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf);
unsigned int SWI2cSequentialWrite(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf, unsigned int size);
unsigned int SWI2cRandomRead(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf);
unsigned int SWI2cSequentialRead(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf, unsigned int size);

#endif  /* HALSW_I2C_H */


