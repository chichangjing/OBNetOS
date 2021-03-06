
/* Standard includes. */
#include <stdio.h>

/* BSP include */
#include "soft_i2c.h"

/********************************************************/              

#define SCL_H			GPIOB->BSRRL = GPIO_Pin_6
#define SCL_L			GPIOB->BSRRH = GPIO_Pin_6 
#define SDA_H			GPIOB->BSRRL = GPIO_Pin_7
#define SDA_L			GPIOB->BSRRH = GPIO_Pin_7
#define SCL_read		GPIOB->IDR & GPIO_Pin_6
#define SDA_read		GPIOB->IDR & GPIO_Pin_7
#define I2C_PageSize	32

/********************************************************/   

extern void TimerDelayMs(unsigned int DelayTime);

void I2C_GPIO_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* PB6 I2C SCL */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);  

	/* PB7 I2C SDA */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);  
	
#if BOARD_GE22103MA
        /* PC8 I2C WP */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); 
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOC, &GPIO_InitStructure);  
    
        GPIOC->BSRRH = GPIO_Pin_8;    
#elif (BOARD_GE22103MA_V1_20) || (BOARD_GV3S_HONUE_QM_V1_00)
        /* PC8 I2C WP */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); 
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOB, &GPIO_InitStructure);  
    
        GPIOB->BSRRH = GPIO_Pin_8;
        /* PD15 I2C WP1 */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOD, &GPIO_InitStructure);  
    
        GPIOD->BSRRH = GPIO_Pin_15;

#elif BOARD_GE20023MA
	/* PB8 I2C WP */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);  

	GPIOB->BSRRH = GPIO_Pin_8;
#elif BOARD_GE11144MD
	/* PB8 I2C WP */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);  

	GPIOB->BSRRH = GPIO_Pin_8;
#endif	
}

void I2C_WriteEnable(void)
{
#if BOARD_GE22103MA
    GPIOC->BSRRH = GPIO_Pin_8;    
#elif (BOARD_GE22103MA_V1_20) || (BOARD_GV3S_HONUE_QM_V1_00)
    GPIOB->BSRRH = GPIO_Pin_8;
#elif BOARD_GE20023MA
	GPIOB->BSRRH = GPIO_Pin_8;
#elif BOARD_GE11144MD
	GPIOB->BSRRH = GPIO_Pin_8;
#endif
}

void I2C_WriteDisable(void)
{
#if BOARD_GE22103MA
	GPIOC->BSRRL = GPIO_Pin_8;    
#elif (BOARD_GE22103MA_V1_20) || (BOARD_GV3S_HONUE_QM_V1_00)
    GPIOB->BSRRL = GPIO_Pin_8;
#elif BOARD_GE20023MA
	GPIOB->BSRRL = GPIO_Pin_8;
#elif BOARD_GE11144MD
	GPIOB->BSRRL = GPIO_Pin_8;
#endif
}

void I2C_delay(void)
{	
   u16 i=100;
   while(i--) ;
}

int I2C_Start(void)
{
	SDA_H;
	SCL_H;
	I2C_delay();
	if(!SDA_read)return 0;
	SDA_L;
	I2C_delay();
	if(SDA_read) return 0;
	SDA_L;
	I2C_delay();
	return 1;
}

void I2C_Stop(void)
{
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SDA_H;
	I2C_delay();
}

void I2C_Ack(void)
{	
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}

void I2C_NoAck(void)
{	
	SCL_L;
	I2C_delay();
	SDA_H;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}

int I2C_WaitAck(void) 	 /* 1: ACK, 0: NO ACK */
{
	SCL_L;
	I2C_delay();
	SDA_H;			
	I2C_delay();
	SCL_H;
	I2C_delay();
	if(SDA_read)
	{
		SCL_L;
		return 0;
	}
	SCL_L;
	return 1;
}

void I2C_SendByte(u8 SendByte)	/* MSB->LSB to send */
{
	u8 i=8;
	while(i--)
	{
		SCL_L;
		I2C_delay();
	if(SendByte&0x80)
		SDA_H;  
	else 
		SDA_L;   
		SendByte<<=1;
		I2C_delay();
		SCL_H;
		I2C_delay();
	}
	SCL_L;
}

u8 I2C_ReceiveByte(void)		/* MSB->LSB to receive */
{ 
	u8 i=8;
	u8 ReceiveByte=0;

	SDA_H;				
	while(i--)
	{
		ReceiveByte<<=1;      
		SCL_L;
		I2C_delay();
		SCL_H;
		I2C_delay();	
		if(SDA_read)
		{
			ReceiveByte|=0x01;
		}
	}
	SCL_L;
	return ReceiveByte;
}

int I2C_ReadByte(u8 *ReadBype, u16 ReadAddress, u8 DeviceAddress)
{
	if(!I2C_Start()){return 0;}
	I2C_SendByte(DeviceAddress & 0xFFFE);					/* Send the device address */
	if(!I2C_WaitAck()){I2C_Stop(); return 0;}
	
	I2C_SendByte((u8)((ReadAddress & 0x1F00) >> 8));		/* Send the read address */
	if(!I2C_WaitAck()){I2C_Stop(); return 0;}
	I2C_SendByte((u8)(ReadAddress & 0x00FF));
	if(!I2C_WaitAck()){I2C_Stop(); return 0;}
											
	if(!I2C_Start()){return 0; }							/* Start read */
	I2C_SendByte(DeviceAddress | 0x0001);
	if(!I2C_WaitAck()){I2C_Stop(); return 0;}

	*ReadBype = I2C_ReceiveByte();
	I2C_NoAck();
	I2C_Stop();
	
	return 1;
}

int I2C_Read(u8 *pBuffer, u8 length, u16 ReadAddress, u8 DeviceAddress)
{                
	if(!I2C_Start()){return 0; }
	I2C_SendByte(DeviceAddress & 0xFFFE);					/* Send the device address */
	if(!I2C_WaitAck()){I2C_Stop(); return 0; }
	
	I2C_SendByte((u8)((ReadAddress & 0x1F00) >> 8));		/* Send the read address */
	if(!I2C_WaitAck()){I2C_Stop(); return 0;}
	I2C_SendByte((u8)(ReadAddress & 0x00FF));
	if(!I2C_WaitAck()){I2C_Stop(); return 0;}
											
	if(!I2C_Start()){return 0; }							/* Start read */
	I2C_SendByte(DeviceAddress | 0x0001);
	I2C_WaitAck();
	while(length)
	{
		*pBuffer = I2C_ReceiveByte();
		if(length == 1) 
			I2C_NoAck();
		else
			I2C_Ack(); 
		pBuffer++;
		length--;
	}
	I2C_Stop();
	
	return 1;
}


int I2C_WriteByte(u8 SendByte, u16 WriteAddress, u8 DeviceAddress)
{
	if(!I2C_Start()){return 0;}
	I2C_SendByte(DeviceAddress & 0xFFFE);					/* Send the device address */
	if(!I2C_WaitAck()){I2C_Stop(); return 0;}
	
	I2C_SendByte((u8)((WriteAddress & 0x1F00) >> 8));		/* Send the write address */
	if(!I2C_WaitAck()){I2C_Stop(); return 0;}
	I2C_SendByte((u8)(WriteAddress & 0x00FF));
	if(!I2C_WaitAck()){I2C_Stop(); return 0;}
	
	I2C_SendByte(SendByte);  
	if(!I2C_WaitAck()){I2C_Stop(); return 0;}
	I2C_Stop(); 

	return 1;
}

/* Note: can't stride page to write */
int I2C_Write(u8 *pBuffer, u8 length, u16 WriteAddress, u8 DeviceAddress)
{
	if(!I2C_Start()){return 0;}
	I2C_SendByte(DeviceAddress & 0xFFFE);					/* Send the device address */
	if(!I2C_WaitAck()){I2C_Stop(); return 0;}
	
	I2C_SendByte((u8)((WriteAddress & 0x1F00) >> 8));		/* Send the write address */
	if(!I2C_WaitAck()){I2C_Stop(); return 0;}
	I2C_SendByte((u8)(WriteAddress & 0x00FF));
	if(!I2C_WaitAck()){I2C_Stop(); return 0;}
	  
	while(length--)
	{
		I2C_SendByte(*pBuffer);
		if(!I2C_WaitAck()){I2C_Stop(); return 0;}
		pBuffer++;
	}
	I2C_Stop();
	TimerDelayMs(10);	
	return 1;
}


int I2C_PageWrite(u8* pBuffer, u8 length, u16 WriteAddress, u8 DeviceAddress)
{
	u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0;
	Addr = WriteAddress % I2C_PageSize;
	count = I2C_PageSize - Addr;
	NumOfPage = length / I2C_PageSize;
	NumOfSingle = length % I2C_PageSize;

	if(Addr == 0) {
		if(NumOfPage == 0) {
			if(I2C_Write(pBuffer,NumOfSingle,WriteAddress,DeviceAddress) != I2C_SUCCESS) return I2C_FAILURE;
		} else {
			while(NumOfPage) {
				if(I2C_Write(pBuffer,I2C_PageSize,WriteAddress,DeviceAddress) != I2C_SUCCESS) return I2C_FAILURE;
				WriteAddress +=  I2C_PageSize;
				pBuffer      +=  I2C_PageSize;
				NumOfPage--;
			}
			if(NumOfSingle!=0) {
				if(I2C_Write(pBuffer,NumOfSingle,WriteAddress,DeviceAddress) != I2C_SUCCESS) return I2C_FAILURE;
			}
		}
	} else {
		if(NumOfPage== 0) {
			if(I2C_Write(pBuffer,NumOfSingle,WriteAddress,DeviceAddress) != I2C_SUCCESS) return I2C_FAILURE;
		} else {
			length       -= count;
			NumOfPage     = length / I2C_PageSize;
			NumOfSingle   = length % I2C_PageSize;

			if(count != 0) {  
				if(I2C_Write(pBuffer,count,WriteAddress,DeviceAddress) != I2C_SUCCESS) return I2C_FAILURE;
				WriteAddress += count;
				pBuffer      += count;
			} 

			while(NumOfPage--) {
				if(I2C_Write(pBuffer,I2C_PageSize,WriteAddress,DeviceAddress) != I2C_SUCCESS) return I2C_FAILURE;
				WriteAddress +=  I2C_PageSize;
				pBuffer      +=  I2C_PageSize; 
			}
			if(NumOfSingle != 0) {
				if(I2C_Write(pBuffer,NumOfSingle,WriteAddress,DeviceAddress) != I2C_SUCCESS) return I2C_FAILURE;
			}
		}
	} 

	return I2C_SUCCESS;
}

int eeprom_read(u16 address, u8 *pBuffer, u8 length) 
{
	if(I2C_Read(pBuffer, length, address, EEPROM_SLAVE_ADDR) != I2C_SUCCESS) {
		return I2C_FAILURE;
	}

	return I2C_SUCCESS;
}

int eeprom_write(u16 address, u8 *pBuffer, u8 length) 
{
	if(I2C_Write(pBuffer, length, address, EEPROM_SLAVE_ADDR) != I2C_SUCCESS) {
		return I2C_FAILURE;
	}

	return I2C_SUCCESS;
}

int eeprom_page_write(u16 address, u8 *pBuffer, u8 length) 
{
	if(I2C_PageWrite(pBuffer, length, address, EEPROM_SLAVE_ADDR) != I2C_SUCCESS) {
		return I2C_FAILURE;
	}

	return I2C_SUCCESS;
}


#ifdef I2C_TEST
static u8 test_data_write[32];
static u8 test_data_read[32];
static u8 test_data_write1[256];
static u8 test_data_read1[256];

int i2c_drv_test(void)
{
	u16 i,j;
	u16 write_address;

	for(i=0; i<128; i++) {
		for(j=0;j<32;j++) {
			test_data_write[j] = i+j;
		}
		
		eeprom_write(0x00, test_data_write, 32);
		eeprom_read(0x00, test_data_read, 32);

		for(j=0;j<32;j++) {
			if(test_data_write[j] != test_data_read[j]) {
				printf("i2c test(%d) error, 0x%08x(read) != 0x%08x(write)\r\n", i, test_data_read[j], test_data_write[j]);
				return -1;
			}
		}
	}
	
	for(j=0;j<32;j++) {
		test_data_write[j] = 0xFF;
	}
	eeprom_write(0x00, test_data_write, 32);
	printf("I2C drv test1 sucessfully\r\n");

	write_address = 0x1000;
	
	for(j=0;j<255;j++) {
		test_data_write1[j] = j;
	}
		
	for(i=0; i<64; i++) {
		eeprom_page_write(write_address+i, test_data_write1, 0xff);	
		eeprom_read(write_address+i, test_data_read1, 0xff);
		for(j=0;j<0xff;j++) {
			if(test_data_write1[j] != test_data_read1[j]) {
				printf("i2c test(%d) error, 0x%08x(read) != 0x%08x(write)\r\n", i, test_data_read1[j], test_data_write1[j]);
				return -1;
			}
		}
	}

	for(j=0;j<255;j++) {
		test_data_write1[j] = 0xFF;
	}

	eeprom_page_write(write_address, test_data_write1, 0xff);	
	eeprom_page_write(write_address + 0x100, test_data_write1, 64);	
	
	printf("I2C drv test2 sucessfully\r\n");
	
	return 0;
}

#endif


