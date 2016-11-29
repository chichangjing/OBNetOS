
/**
  ******************************************************************************
  * @file    robo_drv.c
  * @author  OB networks
  * @version v1.0.0
  * @date    2014-06-23
  * @brief   RoboSwitch SPI driver, ## pressure test passed ##
  ******************************************************************************
  */ 
  
/* Standard includes. */
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "os_mutex.h"

/* BSP include */
#include "stm32f2xx.h"
#include "robo_drv.h"
#include "led_drv.h"

#define ROBO_SPI_TIMEOUT 50

OS_MUTEX_T robo_mutex; 

/****************************************************************************************
  * @brief  Initializes the peripherals used by the SPI Robo driver.
  * @param  None
  * @retval None
  ***************************************************************************************/
static void robo_SpiLowInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable the SPI clock */
  ROBO_SPI_CLK_INIT(ROBO_SPI_CLK, ENABLE);

  /* Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd(ROBO_SPI_SCK_GPIO_CLK | ROBO_SPI_MISO_GPIO_CLK | 
                         ROBO_SPI_MOSI_GPIO_CLK | ROBO_CS_GPIO_CLK, ENABLE);
  
  /* SPI pins configuration */

  /* Connect SPI pins to AF5 */  
  GPIO_PinAFConfig(ROBO_SPI_SCK_GPIO_PORT, ROBO_SPI_SCK_SOURCE, ROBO_SPI_SCK_AF);
  GPIO_PinAFConfig(ROBO_SPI_MISO_GPIO_PORT, ROBO_SPI_MISO_SOURCE, ROBO_SPI_MISO_AF);
  GPIO_PinAFConfig(ROBO_SPI_MOSI_GPIO_PORT, ROBO_SPI_MOSI_SOURCE, ROBO_SPI_MOSI_AF);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
        
  /* SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = ROBO_SPI_SCK_PIN;
  GPIO_Init(ROBO_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /* SPI MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  ROBO_SPI_MOSI_PIN;
  GPIO_Init(ROBO_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /* SPI MISO pin configuration */
  GPIO_InitStructure.GPIO_Pin =  ROBO_SPI_MISO_PIN;
  GPIO_Init(ROBO_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /* Configure ROBO Card CS pin in output pushpull mode */
  GPIO_InitStructure.GPIO_Pin = ROBO_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(ROBO_CS_GPIO_PORT, &GPIO_InitStructure);
}

/****************************************************************************************
  * @brief  Initializes the peripherals used by the SPI Robo driver.
  * @param  None
  * @retval None
  ***************************************************************************************/
void robo_SpiInit(void)
{
  SPI_InitTypeDef  SPI_InitStructure;

  robo_SpiLowInit();
    
  /* Deselect the RoboSwitch: Chip Select high */
  ROBO_CS_HIGH();

  /* SPI configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;

  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(ROBO_SPI, &SPI_InitStructure);

  /* Enable the SPI  */
  SPI_Cmd(ROBO_SPI, ENABLE);
}

/****************************************************************************************
  * @brief  Send byte to the chip 
  * @param  byte
  * @retval 
  *	   the byte read from spi
   ***************************************************************************************/
static u8 robo_SendByte(u8 byte)
{
  /* Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(ROBO_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /* Send byte through the SPIx peripheral */
  SPI_I2S_SendData(ROBO_SPI, byte);

  /* Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(ROBO_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the byte read from the SPI bus */
  return (u8)SPI_I2S_ReceiveData(ROBO_SPI);
}

/****************************************************************************************
  * @brief  poll for SPIF low 
  * @param  byte
  * @retval 
  *	   the byte read from spi
   ***************************************************************************************/
static int robo_poll_for_SPIF(u8 page)
{
	u8 i, timeout;
	u8 data;

	/* Timeout after 50 tries without SPIF low    */
	for(i = 0, timeout = ROBO_SPI_TIMEOUT; timeout;) {
		ROBO_CS_LOW();
		robo_SendByte(0x60);   
		robo_SendByte(0xfe);		
		data = robo_SendByte(0x00); 
		ROBO_CS_HIGH();	
		if (!(data & 0x80))
			break;
		else
			timeout--;
	}

	if(timeout == 0) {
        /* Select chip and page */
        ROBO_CS_LOW();		
        robo_SendByte(0x61);
        robo_SendByte(0xff);
        robo_SendByte(page);
        ROBO_CS_HIGH();
        return -1;
    }	

	return 0;
}

/****************************************************************************************
  * @brief  RoboSwitch read registers 
  * @param  page, addr, buf, len
  * @retval 
  *	   -1: timeout
  *		0: success
  ***************************************************************************************/
int robo_read(u8 page, u8 addr, u8 *buf, u8 len)
{
	u8 i,data,timeout;

	os_mutex_lock(&robo_mutex, OS_MUTEX_WAIT_FOREVER);

#if 0
	/* Poll the SPIF bit */
	ROBO_CS_LOW();
	robo_SendByte(0x60);
	robo_SendByte(0xfe);
	robo_SendByte(0x00);
	ROBO_CS_HIGH();	
#else
	if(robo_poll_for_SPIF(page)) {
		printf("robo_read: poll SPIF timeout!\r\n");
        os_mutex_unlock(&robo_mutex);
		return -1;
	}
#endif

	/* Select chip and page */
	ROBO_CS_LOW();		
	robo_SendByte(0x61);
	robo_SendByte(0xff);
	robo_SendByte(page);
	ROBO_CS_HIGH();

	/* Fast read */
	ROBO_CS_LOW();
	robo_SendByte(0x10);
	for (timeout = ROBO_SPI_TIMEOUT; timeout;) {
		data = robo_SendByte(addr);
		if (data & 0x03 != 0)
			break;
		else {
			timeout--;
		}
	}
	if(timeout == 0){
		os_mutex_unlock(&robo_mutex);
		//printf("robo_read: poll RACK timeout!\r\n");
		return -2;
	}
	for(i=0; i<len; i++) {
		buf[i] = robo_SendByte(0x00);		
	}
	ROBO_CS_HIGH();

	os_mutex_unlock(&robo_mutex);

	return 0;	
}

/****************************************************************************************
  * @brief  RoboSwitch write registers 
  * @param  page, addr, buf, len
  * @retval none
  ***************************************************************************************/
int robo_write(u8 page, u8 addr, u8 *buf, u8 len)
{
	u8 i;

	os_mutex_lock(&robo_mutex, OS_MUTEX_WAIT_FOREVER);

#if 0
	/* Poll the SPIF bit */
	ROBO_CS_LOW();
	robo_SendByte(0x60);
	robo_SendByte(0xfe);
	robo_SendByte(0x00);
	ROBO_CS_HIGH();	
#else
	if(robo_poll_for_SPIF(page)) {
		//printf("robo_read: poll SPIF timeout!\r\n");
        os_mutex_unlock(&robo_mutex);
		return -1;
	}
#endif

	/* Select chip and page */
	ROBO_CS_LOW();		
	robo_SendByte(0x61);
	robo_SendByte(0xff);
	robo_SendByte(page);
	ROBO_CS_HIGH();
	

	/* Write data */
	ROBO_CS_LOW();
	robo_SendByte(0x61);
	robo_SendByte(addr);
	for(i=0; i<len; i++) {
		robo_SendByte(buf[i]);
	}
	ROBO_CS_HIGH();
	
	os_mutex_unlock(&robo_mutex);
	
	return 0;	
}

/****************************************************************************************
  * @brief  RoboSwitch initialize 
  * @param  none
  * @retval none
  *	   -1: failure
  *		0: success  
  ***************************************************************************************/
int RoboSwitch_Init(unsigned char min_ver)
{
	int i;
	u8 data;
	u16 u16Data;
	u32 devid;

	if(os_mutex_init(&robo_mutex) != OS_MUTEX_SUCCESS) {
		printf("Error: init mutex failed\r\n");
		return -1;
	}
	robo_SpiInit();
	
	robo_read(0x02, 0x30, (u8 *)&devid, 4);
	if(devid != 0x00053101) {
		printf("RoboSwitch probe failed\r\n");
		return -1;
	}

	/* Receive unicast/multicast/broadcast enable */
	data = 0x1C;	
	robo_write(0x00, 0x08, &data, 1);

	/* Set IMP port 100M full-duplex, link up, TX/RX PAUSE capable */
	data = 0xB7;//0x87;
	robo_write(0x00, 0x0E, &data, 1);

	/* Set the Swtich Mode to managed mode and enable Frame forwarding */
	data = 0x03;	
	robo_write(0x00, 0x0B, &data, 1);

	/* Enable IMP0 only, receive BPDU enable, reset MIB */
	data = 0x83;
	robo_write(0x02, 0x00, &data, 1);

	/* Broadcom Tag enable for IMP0 only */
	data = 0x01;
	robo_write(0x02, 0x03, &data, 1);	

	/* Disables the aging process */
	data = 0x00;	
	//robo_write(0x02, 0x06, &data, 4);	

	/* Initialize the ports STP states */
	switch((min_ver & 0x18) >> 3) {
		case 0x00:
		data = 0x40;
		robo_write(0x00, 0x00, &data, 1);
		robo_write(0x00, 0x01, &data, 1);
		data = 0xA0;
		robo_write(0x00, 0x02, &data, 1);
		robo_write(0x00, 0x03, &data, 1);
		robo_write(0x00, 0x04, &data, 1);
		break;

		case 0x01:
		data = 0xA0;
		robo_write(0x00, 0x00, &data, 1);
		robo_write(0x00, 0x01, &data, 1);
		robo_write(0x00, 0x02, &data, 1);
		robo_write(0x00, 0x03, &data, 1);
		robo_write(0x00, 0x04, &data, 1);
		break;

		case 0x02:
		data = 0xA0;
		robo_write(0x00, 0x00, &data, 1);
		robo_write(0x00, 0x01, &data, 1);
		robo_write(0x00, 0x02, &data, 1);
		break;

		default:
		data = 0xA0;
		robo_write(0x00, 0x00, &data, 1);
		robo_write(0x00, 0x01, &data, 1);
		break;
	}

	/****************************************************************
		Configure the port 0/1 internal PHY MII register
	 ****************************************************************/
	/* Port 0 */
	u16Data = 0x2100;
	robo_write(0x10, 0x00, (u8 *)&u16Data, 2);
	u16Data = 0x0220;
	robo_write(0x10, 0x20, (u8 *)&u16Data, 2);
	u16Data = 0x0020;
	robo_write(0x10, 0x2E, (u8 *)&u16Data, 2);	
	u16Data = 0x008B;
	robo_write(0x10, 0x3E, (u8 *)&u16Data, 2);
	u16Data = 0x0200;
	robo_write(0x10, 0x32, (u8 *)&u16Data, 2);
	u16Data = 0x0084;
	robo_write(0x10, 0x3A, (u8 *)&u16Data, 2);
	u16Data = 0x000B;
	robo_write(0x10, 0x3E, (u8 *)&u16Data, 2);	

	if((min_ver & 0x18) == 0) {
		/* Port 1 */
		u16Data = 0x2100;
		robo_write(0x11, 0x00, (u8 *)&u16Data, 2);
		u16Data = 0x0220;
		robo_write(0x11, 0x20, (u8 *)&u16Data, 2);
		u16Data = 0x0020;
		robo_write(0x11, 0x2E, (u8 *)&u16Data, 2);	
		u16Data = 0x008B;
		robo_write(0x11, 0x3E, (u8 *)&u16Data, 2);
		u16Data = 0x0200;
		robo_write(0x11, 0x32, (u8 *)&u16Data, 2);
		u16Data = 0x0084;
		robo_write(0x11, 0x3A, (u8 *)&u16Data, 2);
		u16Data = 0x000B;
		robo_write(0x11, 0x3E, (u8 *)&u16Data, 2);
	}
	
	printf("RoboSwitch is initialized\r\n");
	
	return 0;
}

//#define ROBO_TEST
#ifdef ROBO_TEST
/****************************************************************************************
  * @brief  RoboSwitch driver test 
  * @param  none
  * @retval none
  ***************************************************************************************/
int robo_drv_test(void)
{
#if SWITCH_CHIP_BCM53101
	u32 i,j,loop;
	u32 data, temp_data;
	
	for(loop=0; loop<10; loop++) {
		for(i=512; i<1512; i++) {
			data = i;
			robo_write(0x36, 0x08, (u8 *)&data, 4);
			data = 0;
			robo_read(0x36, 0x08, (u8 *)&data, 4);
			if(data != i) {
				printf("robo test error, loop=%d, 0x%08x(read) != 0x%08x(write)\r\n", loop, data, i);
				return -1;
			}
		}
	}

	data = 0x200;
	robo_write(0x36, 0x08, (u8 *)&data, 4);
	
	printf("RoboSwitch test sucessfully\r\n");
	
	return 0;
#elif SWITCH_CHIP_BCM53286
	u32 i,j,loop;
	u8	temp_data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	u8	data1[8] = {0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58};
	u8	data2[8] = {0xa8, 0xa7, 0xa6, 0xa5, 0xa4, 0xa3, 0xa2, 0xa1};
	
	for(loop=0; loop<1000; loop++) {
		memset(temp_data, 0, 8);
		if(robo_write(0x08, 0x20, data1, 8) != 0) { printf("robo_write error, loop=%d\r\n", loop); return -1;}
		if(robo_read(0x08, 0x20, temp_data, 8) != 0) { printf("robo_write error, loop=%d\r\n", loop); return -1;}
		if(memcmp(data1, temp_data, 8) != 0) {
			printf("Error: write data: 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x\r\n",
			data1[0], data1[1], data1[2], data1[3], data1[4], data1[5], data1[6], data1[7]);			
			printf("       read  data: 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x  loop=%d\r\n",
			temp_data[0], temp_data[1], temp_data[2], temp_data[3], temp_data[4], temp_data[5], temp_data[6], temp_data[7], loop);
			return -1;
		}
		memset(temp_data, 0, 8);
		if(robo_write(0x08, 0x20, data2, 8) != 0) { printf("robo_write error, loop=%d\r\n", loop); return -1;}
		if(robo_read(0x08, 0x20, temp_data, 8) != 0) { printf("robo_write error, loop=%d\r\n", loop); return -1;}
		if(memcmp(data2, temp_data, 8) != 0) {
			printf("Error: write data: 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x\r\n",
			data2[0], data2[1], data2[2], data2[3], data2[4], data2[5], data2[6], data2[7]);			
			printf("       read  data: 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x  loop=%d\r\n",
			temp_data[0], temp_data[1], temp_data[2], temp_data[3], temp_data[4], temp_data[5], temp_data[6], temp_data[7], loop);
			return -1;
		}		
	}

	printf("RoboSwitch test sucessfully\r\n");
	
	return 0;	
#endif
}

#endif

