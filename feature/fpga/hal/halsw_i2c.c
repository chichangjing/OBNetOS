/*************************************************************
 * Filename     : 
 * Description  : 
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/
/* include -------------------------------------------------------------------*/
#include "halsw_i2c.h"
#include "os_mutex.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern OS_MUTEX_T i2c_mutex; 
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static char speedmode0 = DEFSPEED , speedmode1 = DEFSPEED;

/**
 * @brief delay
 * @param 
 * @return none
 */
static void SW_Delay(int speed)
{
	while (speed--);
}

void SWI2cGpioInit(int devid)
{
	GPIO_InitTypeDef gpioinit;
	gpioinit.GPIO_Mode = GPIO_Mode_OUT;
	gpioinit.GPIO_OType = GPIO_OType_OD;
	gpioinit.GPIO_PuPd = GPIO_PuPd_UP;
	gpioinit.GPIO_Speed = GPIO_Speed_25MHz;
	/* software_i2c gpio */
	switch (devid)
	{
#if  ( (defined HAL_SWI2C0) &&  HAL_SWI2C0)
		case I2CDEV_0:
			RCC_AHB1PeriphClockCmd(SCL0PERIPH | SDA0PERIPH, ENABLE);
			/* i2c0_CLK(PB6),i2c0_DAT(PB7) */
			/* swi2c scl */
			gpioinit.GPIO_Pin = SCL0PIN;
			GPIO_Init(SCL0PORT, &gpioinit);
			/* swi2c sda */
			gpioinit.GPIO_Pin = SDA0PIN;
			GPIO_Init(SDA0PORT, &gpioinit);
			break;
#endif
#if  ( (defined HAL_SWI2C1) &&  HAL_SWI2C1)
		case I2CDEV_1:
			RCC_AHB1PeriphClockCmd(SCL1PERIPH | SDA1PERIPH, ENABLE);
			/* i2c1_CLK(PB10),i2c1_DAT(PB11) */
			gpioinit.GPIO_Pin = SCL1PIN;
			GPIO_Init(SCL1PORT, &gpioinit);
			/* swi2c sda */
			gpioinit.GPIO_Pin = SDA1PIN;
			GPIO_Init(SDA1PORT, &gpioinit);
			break;
#endif
	}
}

void SWI2cSpeedSet(int devid, speedmode_e speed)
{
	switch (devid)
	{
		case I2CDEV_0:
			speedmode0	 = speed;
			break;
		case I2CDEV_1:
			speedmode1 = speed;
			break;
	}
}

int SWI2cSpeedGet(int devid)
{
	switch (devid)
	{
		case I2CDEV_0:
			return speedmode0;
		case I2CDEV_1:
			return speedmode1;
	}
	return 0;
}

void SW_I2CInit(int devid, unsigned char speed)
{
	switch (devid)
	{
		case I2CDEV_0:
			SWI2cGpioInit(I2CDEV_0);
			/* 设置SDA,SCL状态 */
			SW_SCL0SET;
			SW_SDA0SET;
			/* 设置I2C速度 */
			if (speed)
			{
				speedmode0 = speed;
			}
			break;
		case I2CDEV_1:
			SWI2cGpioInit(I2CDEV_1);
			/* 设置SDA,SCL状态 */
			SW_SCL1SET;
			SW_SDA1SET;
			/* 设置I2C速度 */
			if (speed)
			{
				speedmode1 = speed;
			}
			break;
		default:
			while (1);
	}
}


void SW_I2CBusstart(int devid)
{
	switch (devid)
	{
		case I2CDEV_0:
			SW_SCL0SET;
			SW_Delay(speedmode0);
			SW_SDA0SET;
			SW_Delay(speedmode0);
			SW_SDA0CLR;
			SW_Delay(speedmode0);
			SW_SCL0CLR;
			SW_Delay(speedmode0);
			break;
		case I2CDEV_1:
			SW_SCL1SET;
			SW_Delay(speedmode1);
			SW_SDA1SET;
			SW_Delay(speedmode1);
			SW_SDA1CLR;
			SW_Delay(speedmode1);
			SW_SCL1CLR;
			SW_Delay(speedmode1);
			break;
		default:
			while (1);
	}
}

void SW_I2CBusStop(int devid)
{
	switch (devid)
	{
		case I2CDEV_0:
			SW_SDA0CLR;
			SW_Delay(speedmode0);
			SW_SCL0SET;
			SW_Delay(speedmode0);
			SW_SDA0SET;
			SW_Delay(speedmode0);
			break;
		case I2CDEV_1:
			SW_SDA1CLR;
			SW_Delay(speedmode1);
			SW_SCL1SET;
			SW_Delay(speedmode1);
			SW_SDA1SET;
			SW_Delay(speedmode1);
			break;
		default:
			while (1);
	}
}


/* 获取总线状态 */
unsigned char SW_GetI2CState(int devid)
{
	unsigned char status = 0;
	switch (devid)
	{
		case I2CDEV_0:
			status = SW_SCL0READ ;
			status |= (SW_SDA0READ << 1);
			break;
		case I2CDEV_1:
			status = SW_SCL1READ ;
			status |= (SW_SDA1READ << 1);
			break;
		default:
			while (1);
	}
	if (status != 0x03)
	{
		return I2C_NO_IDLE;
	}
	return I2C_IDLE_OK;
}

/* 检测总线状态 */
unsigned char SW_CheakI2CState(int devid)
{
	unsigned char i, j;
	switch (devid)
	{
		case I2CDEV_0:
			SW_SCL0SET;
			SW_Delay(speedmode0);
			for (i = 0; i < 10; i++)
			{
				if (SW_SCL0READ)
				{
					SW_SDA0SET;
					SW_Delay(speedmode0);
					for (j = 0; j < 10; j++)
					{
						if (SW_SDA0READ)
						{
							return I2C_IDLE_OK;
						}
						SW_Delay(speedmode0);
					}
					return I2C_SDA_DOWN;
				}
				SW_Delay(speedmode0);
			}
			break;
		case I2CDEV_1:
			SW_SCL1SET;
			SW_Delay(speedmode1);
			for (i = 0; i < 10; i++)
			{
				if (SW_SCL1READ)
				{
					SW_SDA1SET;
					SW_Delay(speedmode1);
					for (j = 0; j < 10; j++)
					{
						if (SW_SDA1READ)
						{
							return I2C_IDLE_OK;
						}
						SW_Delay(speedmode1);
					}
					return I2C_SDA_DOWN;
				}
				SW_Delay(speedmode1);
			}
		default:
			while (1);
	}
	return I2C_NO_IDLE;
}


unsigned char SW_I2CBusIdle(int devid)
{
	unsigned char cnt = 9;
	switch (devid)
	{
		case I2CDEV_0:
			while (cnt--)
			{
				SW_SCL0SET;
				SW_Delay(speedmode0);
				SW_Delay(speedmode0);
				SW_SCL0CLR;
				SW_Delay(speedmode0);
				SW_Delay(speedmode0);
			}
			SW_SCL0SET;
			SW_Delay(speedmode0);
			break;
		case I2CDEV_1:
			while (cnt--)
			{
				SW_SCL1SET;
				SW_Delay(speedmode1);
				SW_Delay(speedmode1);
				SW_SCL1CLR;
				SW_Delay(speedmode1);
				SW_Delay(speedmode1);
			}
			SW_SCL1SET;
			SW_Delay(speedmode1);
			SW_Delay(speedmode1);
			break;
		default:
			while (1);
	}
	return SW_GetI2CState(devid);
}


#define DELAYCNT	15
unsigned int SW_I2CWaitAck(int devid)
{
	unsigned int i = 0;
	switch (devid)
	{
		case I2CDEV_0:
			while (SW_SDA0READ && (i < DELAYCNT))
			{
				SW_SCL0CLR;
				SW_Delay(speedmode0);
				SW_SCL0SET;
				SW_Delay(speedmode0);
				i++;
			}
			if (i == DELAYCNT)
			{
				return I2C_NO_ACK;
			}
			return I2C_ACK_OK;
		case I2CDEV_1:
			while (SW_SDA1READ && (i <= 10))
			{
				SW_Delay(speedmode1);
				i++;
			}
			if (i == DELAYCNT)
			{
				return I2C_NO_ACK;
			}
			return I2C_ACK_OK;
	}
	return I2C_NO_ACK;
}


unsigned char SW_I2CWriteByte(int devid, unsigned char data)
{
	unsigned char i, err = 0;
	switch (devid)
	{
		case I2CDEV_0:
			for (i = 0; i < 8; i++)
			{
				if (data & (0x80 >> i))
				{
					SW_SDA0SET;
				}
				else
				{
					SW_SDA0CLR;
				}
				SW_Delay(1);
				SW_SCL0SET;
				SW_Delay(speedmode0);
				SW_SCL0CLR;
				SW_Delay(speedmode0);
			}
			SW_SCL0SET;
			SW_SDA0SET;
//			SW_Delay(speedmode0);
			SW_Delay(speedmode0);
			if (SW_I2CWaitAck(I2CDEV_0))
			{
				err = I2C_NO_ACK;
			}
			SW_SCL0CLR;
			SW_Delay(speedmode0);
			break;
		case I2CDEV_1:
			for (i = 0; i < 8; i++)
			{
				if (data & (0x80 >> i))
				{
					SW_SDA1SET;
				}
				else
				{
					SW_SDA1CLR;
				}
				SW_Delay(1);
				SW_SCL1SET;
				SW_Delay(speedmode1);
				SW_SCL1CLR;
				SW_Delay(speedmode1);
			}
			SW_SDA1SET;
			SW_Delay(speedmode1);
			SW_SCL1SET;
			SW_Delay(speedmode1);
			if (SW_I2CWaitAck(I2CDEV_1))
			{
				err = I2C_NO_ACK;
			}
			SW_SCL1CLR;
			SW_Delay(speedmode1);
			break;
		default:
			while (1);
	}
	return err;  //'1'noack, '0'ack
}


unsigned char SW_I2CReadByte(int devid, unsigned char nack)
{
	unsigned char ret = 0 , i;
	switch (devid)
	{
		case I2CDEV_0:
//			SW_SDA0SET;
//			SW_Delay(speedmode0);
			for (i = 0; i < 8; i++)
			{
				SW_SCL0SET;
				ret <<= 1;
				SW_Delay(speedmode0);
				ret |= SW_SDA0READ;
				SW_SCL0CLR;
				SW_Delay(speedmode0);
			}
			if (nack)
			{
				SW_SDA0SET;
			}
			else
			{
				SW_SDA0CLR;
			}
			SW_Delay(speedmode0);
			SW_SCL0SET;
			SW_Delay(speedmode0);
			SW_SCL0CLR;
			SW_Delay(speedmode0);
			SW_SDA0SET;
			SW_Delay(speedmode0);
			break;
		case I2CDEV_1:
//			SW_SDA1SET;
//			SW_Delay(speedmode1);
			for (i = 0; i < 8; i++)
			{
				SW_SCL1SET;
				ret <<= 1;
				SW_Delay(speedmode1);
				ret |= SW_SDA1READ;
				SW_SCL1CLR;
				SW_Delay(speedmode1);
			}
			if (nack)
			{
				SW_SDA1SET;
			}
			else
			{
				SW_SDA1CLR;
			}
			SW_Delay(speedmode1);
			SW_SCL1SET;
			SW_Delay(speedmode1);
			SW_SCL1CLR;
			SW_Delay(speedmode1);
			SW_SDA1SET;
			SW_Delay(speedmode1);
			break;
		default:
			while (1);
	}
	return ret;
}


unsigned int SWI2cCurrentWrite(int devid, unsigned char devadd, unsigned char *buf)
{
	unsigned int err;
    
    os_mutex_lock(&i2c_mutex, OS_MUTEX_WAIT_FOREVER);
	err = SW_CheakI2CState(devid);
	if (err == I2C_IDLE_OK)
	{
		SW_I2CBusstart(devid);
		err = SW_I2CWriteByte(devid, devadd | I2C_W);
		if (err == I2C_ACK_OK)
		{
			err = SW_I2CWriteByte(devid, *buf);
		}
		SW_I2CBusStop(devid);
	}
    os_mutex_unlock(&i2c_mutex);
    
	return err;
}


unsigned int SWI2cCurrentRead(int devid, unsigned char devadd, unsigned char *buf)
{
	unsigned int err;
    
    os_mutex_lock(&i2c_mutex, OS_MUTEX_WAIT_FOREVER);
	err = SW_CheakI2CState(devid);
	if (err == I2C_IDLE_OK)
	{
		SW_I2CBusstart(devid);
		err = SW_I2CWriteByte(devid, devadd | I2C_R);
		if (err == I2C_ACK_OK)
		{
			*buf = SW_I2CReadByte(devid, I2C_NO_ACK);
		}
		SW_I2CBusStop(devid);
	}
    os_mutex_unlock(&i2c_mutex);
    
	return err;
}



unsigned int SWI2cRandomWrite(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf)
{
	unsigned int err;
    
    os_mutex_lock(&i2c_mutex, OS_MUTEX_WAIT_FOREVER);
	err = SW_CheakI2CState(devid);
	if (err == I2C_IDLE_OK)
	{
		SW_I2CBusstart(devid);
		err = SW_I2CWriteByte(devid, devadd | I2C_W);
		if (err == I2C_ACK_OK)
		{
			err = SW_I2CWriteByte(devid, (unsigned char)storeadd);
			if (err == I2C_ACK_OK)
			{
				err = SW_I2CWriteByte(devid, *buf);
			}
		}
		SW_I2CBusStop(devid);
	}
    os_mutex_unlock(&i2c_mutex);
    
	return err;
}

unsigned int SWI2cSequentialWrite(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf, unsigned int size)
{
	unsigned int err;
	unsigned int i;
    
    os_mutex_lock(&i2c_mutex, OS_MUTEX_WAIT_FOREVER);
	err = SW_CheakI2CState(devid);
	if (err == I2C_IDLE_OK)
	{
		SW_I2CBusstart(devid);
		err = SW_I2CWriteByte(devid, devadd | I2C_W);
		if (err == I2C_ACK_OK)
		{
			err = SW_I2CWriteByte(devid, (unsigned char)storeadd);
			if (err == I2C_ACK_OK)
			{
				for (i = 0; i < size; i++)
				{
					err = SW_I2CWriteByte(devid, *buf);
					buf++;
				}
			}
		}
		SW_I2CBusStop(devid);
	}
    os_mutex_unlock(&i2c_mutex);
    
	return err;
}

unsigned int SWI2cRandomRead(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf)
{
	unsigned int err;
    
    os_mutex_lock(&i2c_mutex, OS_MUTEX_WAIT_FOREVER);
	err = SW_CheakI2CState(devid);
	/*	if((err = SW_I2CBusIdle(devid)) != I2C_IDLE_OK){
				while(1);
		}*/
	if (err == I2C_IDLE_OK)
	{
		SW_I2CBusstart(devid);
		err = SW_I2CWriteByte(devid, devadd | I2C_W);
		if (err == I2C_ACK_OK)
		{
			err = SW_I2CWriteByte(devid, (unsigned char)storeadd);
			if (err == I2C_ACK_OK)
			{
				SW_I2CBusstart(devid);
				if (!SW_I2CWriteByte(devid, devadd | I2C_R))
				{
					*buf = SW_I2CReadByte(devid, I2C_NO_ACK);
				}
			}
		}
		SW_I2CBusStop(devid);
	}
    os_mutex_unlock(&i2c_mutex);
    
	return err;
}

unsigned int SWI2cSequentialRead(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf, unsigned int size)
{
	unsigned int err , i;
    
    os_mutex_lock(&i2c_mutex, OS_MUTEX_WAIT_FOREVER);
	err = SW_CheakI2CState(devid);
	size -= 1;
	if (err == I2C_IDLE_OK)
	{
		SW_I2CBusstart(devid);
		err = SW_I2CWriteByte(devid, devadd | I2C_W);
		if (err == I2C_ACK_OK)
		{
			err = SW_I2CWriteByte(devid, (unsigned char)storeadd);
			if (err == I2C_ACK_OK)
			{
				SW_I2CBusstart(devid);
				if (!SW_I2CWriteByte(devid, devadd | I2C_R))
				{
					for (i = 0; i < size; i++)
					{
						*buf = SW_I2CReadByte(devid, I2C_ACK_OK);
						buf++;
					}
					*buf = SW_I2CReadByte(devid, I2C_NO_ACK);
				}
			}
			SW_I2CBusStop(devid);
		}
	}
	if (err)
	{
		SW_I2CBusStop(devid);
	}
    os_mutex_unlock(&i2c_mutex);
    
	return err;
}



/*
switch(devid){
	case I2CDEV_0:
		break;
	case I2CDEV_1:
		break;
	default:
		while(1);

}


*/





