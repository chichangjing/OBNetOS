/*************************************************************
 * Filename     : 
 * Description  : 
 * Copyright    : OB Telecom Electronics Co.
 * Email        : 0609ccj@163.com
 *************************************************************/
/* include -------------------------------------------------------------------*/

#include "halsw_i2c.h"
#include "i2c_ee.h"
#include "delay.h"
#include "os_mutex.h"
#include "stdio.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

extern OS_MUTEX_T i2c_mutex; 
const  unsigned int pagesize = (AT24C32 + 1);

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void At24WpInit(void)
{
	GPIO_InitTypeDef gpioinit;
	RCC_APB2PeriphClockCmd(EEWPPERIPH, ENABLE);
	/* set io mode is output */
	gpioinit.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpioinit.GPIO_OType = GPIO_OType_PP;
	gpioinit.GPIO_Mode = GPIO_Mode_AF;
	gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
	gpioinit.GPIO_Pin = EEWPPIN;
	GPIO_Init(EEWPPORT, &gpioinit);
	AT24_WPDIS;
    
    if(NULL == i2c_mutex)
    {
        if(os_mutex_init(&i2c_mutex) != OS_MUTEX_SUCCESS) {
		printf("Error: init mutex failed\r\n");
		return;
        } 
    }
}


unsigned char At24_Byte_Write(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf)
{
	unsigned char err;
    
    os_mutex_lock(&i2c_mutex, OS_MUTEX_WAIT_FOREVER);
	AT24_WPDIS;
	Delay_5Us(2000);
	err = SW_CheakI2CState(devid);

	if(err == I2C_IDLE_OK) {
		SW_I2CBusstart(devid);
		err = SW_I2CWriteByte(devid, devadd | I2C_W);
		if (err == I2C_ACK_OK) {
			if (!(err = SW_I2CWriteByte(devid, (unsigned char)(storeadd>>8)))) {
				err = SW_I2CWriteByte(devid, (unsigned char)storeadd);
				if (err == I2C_ACK_OK) {
					err = SW_I2CWriteByte(devid, *buf);
				}
			}
		}
		SW_I2CBusStop(devid);
	}
	AT24_WPENA;
    os_mutex_unlock(&i2c_mutex);
    
	return err;
}
unsigned char At24_Page_Write(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf, int size)
{
	unsigned char err;
	unsigned int  tsize, i;
	unsigned int  startpage;
	unsigned char spramainsize;
    
    os_mutex_lock(&i2c_mutex, OS_MUTEX_WAIT_FOREVER);
	startpage = storeadd / pagesize;
	spramainsize = pagesize - (storeadd - startpage * pagesize);
	AT24_WPDIS;
	Delay_5Us(2000);
	if (spramainsize) {
		if (spramainsize > size) {
			tsize = size;
		}
		else{
			tsize = spramainsize;
		}
	}
	else {
		tsize = (size / pagesize) ? pagesize : (size % pagesize);
	}
	if (err = SW_CheakI2CState(devid)) {
		if ((err = SW_I2CBusIdle(devid)) != I2C_IDLE_OK) {
			while (1);
		}
	}
	if (err == I2C_IDLE_OK) {
		do {
			SW_I2CBusstart(devid);
			err = SW_I2CWriteByte(devid, devadd | I2C_W);
			if (err != I2C_ACK_OK) {
				return err;
			}
			err = SW_I2CWriteByte(devid, (unsigned char)(storeadd >> 8));
			if (err != I2C_ACK_OK) {
				return err;
			}
			err = SW_I2CWriteByte(devid, (unsigned char)storeadd);
			if (err == I2C_ACK_OK) {
				for (i = 0; i < tsize; i++)	{
					err = SW_I2CWriteByte(devid, *buf);
					buf++;
				}
			}
			SW_I2CBusStop(devid);
			storeadd += tsize;
//			Delay_5Us(1000);
			Delay_5Us(2000);
			size -= tsize;
			tsize = (size / pagesize) ? pagesize : (size % pagesize);
		}
		while (size);
	}
	AT24_WPENA;
    os_mutex_unlock(&i2c_mutex);
    
	return err;
}


unsigned char At24_Addr_Write(int devid, unsigned char devadd, unsigned int storeadd)
{
	unsigned char err;
    
	SW_I2CBusstart(devid);
	if ((err = SW_I2CWriteByte(devid, devadd | I2C_W)) ==  I2C_ACK_OK) {
		if ((err = SW_I2CWriteByte(devid, (unsigned char)(storeadd >> 8))) ==  I2C_ACK_OK) {
			if ((err = SW_I2CWriteByte(devid, (unsigned char)storeadd)) == I2C_ACK_OK) {
				;
			}
		}
	}
	if (err) {
		SW_I2CBusStop(devid);
	}
    
	return err;
}

unsigned char At24_Current_Read(int devid, unsigned char devadd, unsigned char *buf)
{
	unsigned char err;
    
    os_mutex_lock(&i2c_mutex, OS_MUTEX_WAIT_FOREVER);
	AT24_WPDIS;
	Delay_5Us(2000);
	err = SW_CheakI2CState(devid);
	if (err == I2C_IDLE_OK) {
		SW_I2CBusstart(devid);
		err = SW_I2CWriteByte(devid, devadd | I2C_R);
		if (err == I2C_ACK_OK) {
			*buf = SW_I2CReadByte(devid, I2C_NO_ACK);
		}
		SW_I2CBusStop(devid);
	}
	AT24_WPENA;
    os_mutex_unlock(&i2c_mutex);
    
	return err;
}

unsigned char At24_Random_Read(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf)
{
	unsigned char err;
	AT24_WPDIS;
	Delay_5Us(2000);
	err = SW_CheakI2CState(devid);
	if (err == I2C_IDLE_OK) {
		if (I2C_ACK_OK == (err = At24_Addr_Write(devid, devadd, storeadd))) {
			SW_I2CBusstart(devid);
			if ((err = SW_I2CWriteByte(devid, devadd | I2C_R)) == I2C_ACK_OK) {
				*buf = SW_I2CReadByte(devid, I2C_NO_ACK);
			}
		}
	}
	AT24_WPENA;
    os_mutex_unlock(&i2c_mutex);
    
	return err;
}

unsigned char At24_Sequential_Read(int devid, unsigned char devadd, unsigned int storeadd, unsigned char *buf, int size)
{
	unsigned char err ;
    unsigned char i;
    
    os_mutex_lock(&i2c_mutex, OS_MUTEX_WAIT_FOREVER);
	AT24_WPDIS;
	Delay_5Us(2000);
	err = SW_CheakI2CState(devid);
	size -= 1;
	if (err == I2C_IDLE_OK) {
		if (I2C_ACK_OK == (err = At24_Addr_Write(devid, devadd, storeadd))) {
			SW_I2CBusstart(devid);
			if ((err = SW_I2CWriteByte(devid, devadd | I2C_R)) ==  I2C_ACK_OK) {
				for (i = 0; i < size; i++) {
					*buf = SW_I2CReadByte(devid, I2C_ACK_OK);
					buf++;
				}
				*buf = SW_I2CReadByte(devid, I2C_NO_ACK);
			}
			SW_I2CBusStop(devid);
		}
	}
	if (err) {
		SW_I2CBusStop(devid);
	}
	AT24_WPENA;
    os_mutex_unlock(&i2c_mutex);
    
	return err;
}