

/* Standard includes */
#include "stdio.h"
#include "ctype.h"
#include "string.h"

#if 0
#if defined(OS_FREERTOS)
/* Kernel includes. */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#endif
#else
/* OS includes */
//#include "os_mutex.h"
#endif

/* BSP includes */
#include "stm32f2xx.h"
#include "soft_i2c.h"
#include "i2c_if.h"
//#include "bdinfo.h"

const unsigned int baudrate_list[] = {110,300,600,1200,2400,4800,9600,14400,19200,28800,38400,57600,115200,230400,460800,921600};
const unsigned short wordlength_list[] = {USART_WordLength_8b, USART_WordLength_9b};
const unsigned short stopbits_list[] = {USART_StopBits_1,USART_StopBits_0_5,USART_StopBits_2,USART_StopBits_1_5};
const unsigned short parity_list[] = {USART_Parity_No,USART_Parity_Even,USART_Parity_Odd};
const unsigned short flowctrl_list[] = {USART_HardwareFlowControl_None,USART_HardwareFlowControl_RTS,USART_HardwareFlowControl_CTS,USART_HardwareFlowControl_RTS_CTS};

#if 0
#if defined(OS_FREERTOS)
extern xSemaphoreHandle mutexI2C;
#endif
#else
//extern OS_MUTEX_T i2c_mutex;
#endif


#if 0
int eeprom_rtos_read(u16 address, u8 *pBuffer, u8 length) 
{
#if defined(OS_FREERTOS)
	if(xSemaphoreTakeRecursive( mutexI2C, portMAX_DELAY) == pdTRUE ) {
#endif		
		if(I2C_Read(pBuffer, length, address, EEPROM_SLAVE_ADDR)) {
			#if defined(OS_FREERTOS)
			xSemaphoreGiveRecursive(mutexI2C);
			#endif
			return I2C_SUCCESS;
		}
		#if defined(OS_FREERTOS)
		xSemaphoreGiveRecursive(mutexI2C);
		#endif
#if defined(OS_FREERTOS)		
	}
#endif	

	return I2C_FAILURE;
}
#endif



#if 0
int eeprom_rtos_write(u16 address, u8 *pBuffer, u8 length) 
{
#if defined(OS_FREERTOS)
	if(xSemaphoreTakeRecursive( mutexI2C, portMAX_DELAY) == pdTRUE ) {
#endif		
		if(I2C_Write(pBuffer, length, address, EEPROM_SLAVE_ADDR)) {
			#if defined(OS_FREERTOS)
			xSemaphoreGiveRecursive(mutexI2C);
			#endif
			return I2C_SUCCESS;
		}
		#if defined(OS_FREERTOS)
		xSemaphoreGiveRecursive(mutexI2C);
		#endif
#if defined(OS_FREERTOS)		
	}
#endif	

	return I2C_FAILURE;
}
#endif

int UpgradeFlagSet(void)
{
	u8 UpgradeFlag;
	
	UpgradeFlag = 0x80;
	if(eeprom_write(EPROM_ADDR_UPGRADE_FLAG, &UpgradeFlag, 1) == I2C_FAILURE)
		return IF_ERR_I2C;
	
	return IF_ERR_OK;
}


int GetMacAddr(u8 *mac)
{
	int ret;
	u8 invalid_mac1[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	u8 invalid_mac2[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	u8 data[6];


	ret = eeprom_read(EPROM_ADDR_MAC, data, 6);
	if(ret) {
		if((memcmp(data, invalid_mac1, 6) == 0) || (memcmp(data, invalid_mac2, 6) == 0))
			return IF_ERR_MAC_INVALID;
		else 
			memcpy(mac, data, 6);	
	} else {
		return IF_ERR_I2C;
	}

	return IF_ERR_OK;
}

int SetMacAddr(u8 *mac)
{
	u8 invalid_mac1[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	u8 invalid_mac2[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	if((memcmp(mac, invalid_mac1, 6) == 0) || (memcmp(mac, invalid_mac2, 6) == 0)) {
		return IF_ERR_MAC_INVALID;
	}

	if(eeprom_write(EPROM_ADDR_MAC, mac, 6) == I2C_FAILURE)
		return IF_ERR_I2C;
	
	return IF_ERR_OK;
}

int SetNameID(u8 *DeviceName, u8 *SerialNumber)
{
	//printf("DeviceName   = %s, len=%d\r\n", DeviceName,strlen((char *)DeviceName));
	//printf("SerialNumber = %s, len=%d\r\n", SerialNumber, strlen((char *)SerialNumber));
	if(strlen((char *)DeviceName) > MAX_DEVICE_NAME_LEN) 
		return IF_ERR_OVERLENGTH;
	
	if(eeprom_page_write(EPROM_ADDR_DEVICE_NAME, DeviceName, (u8)(MAX_DEVICE_NAME_LEN + 1)) != I2C_SUCCESS)
		return IF_ERR_I2C;

	return IF_ERR_OK;
}

int GetNameID(u8 *DeviceName, u8 *SerialNumber)
{
	if(eeprom_read(EPROM_ADDR_DEVICE_NAME, DeviceName, (u8)(MAX_DEVICE_NAME_LEN+1)) != I2C_SUCCESS)
		return IF_ERR_I2C;
	
	if(isalnum(DeviceName[0]) == 0)
		memset(DeviceName, 0, MAX_DEVICE_NAME_LEN+1);

#if 0
	if(ManuInfo_Read_SerialNumber((char *)SerialNumber) != BDINFO_SUCCESS)
		return IF_ERR_I2C;
#endif
	if(isalnum(SerialNumber[0]) == 0)
		memset(SerialNumber, 0, MAX_SERIAL_NUMBER_LEN+1);
	
	return IF_ERR_OK;
}


int SetIP(u8 *pIpConfig)
{
	if(eeprom_write(EPROM_ADDR_IP, pIpConfig, 12) != I2C_SUCCESS)
		return IF_ERR_I2C;
	
	return IF_ERR_OK;
}

int GetIP(u8 *pIpConfig)
{
	if(eeprom_read(EPROM_ADDR_IP, pIpConfig, 12) != I2C_SUCCESS)
		return IF_ERR_I2C;
	
	return IF_ERR_OK;
}

int GetUartEnable(u8 port, u8 *uart_enable)
{
	u8	CfgUartEn=0;
	
	if(eeprom_read(EPROM_ADDR_UART_EN(port), &CfgUartEn, 1) == I2C_SUCCESS) {
		*uart_enable = CfgUartEn;
		return IF_ERR_OK;
	} else {
		return IF_ERR_I2C;
	}
}

int GetUartBaudRate(u8 port, u32 *uart_baudrate)
{
	u8	CfgBaudRate=0;
	
	if(eeprom_read(EPROM_ADDR_BAUDRATE(port), &CfgBaudRate, 1) == I2C_SUCCESS) {
		*uart_baudrate = baudrate_list[CfgBaudRate];
		return IF_ERR_OK;
	} else {
		return IF_ERR_I2C;
	}
}

int GetUartWordLength(u8 port, u16 *uart_word_length)
{
	u8	CfgWordLength=0;

	if(eeprom_read(EPROM_ADDR_WORDLENGTH(port), &CfgWordLength, 1) == I2C_SUCCESS) {
		*uart_word_length = wordlength_list[CfgWordLength];
		return IF_ERR_OK;
	} else {
		return IF_ERR_I2C;
	}
}

int GetUartStopBits(u8 port, u16 *uart_stop_bits)
{
	u8	CfgStopBits=0;
	
	if(eeprom_read(EPROM_ADDR_STOPBITS(port), &CfgStopBits, 1) == I2C_SUCCESS) {
		*uart_stop_bits = stopbits_list[CfgStopBits];
		return IF_ERR_OK;
	} else {
		return IF_ERR_I2C;
	}
}

int GetUartParity(u8 port, u16 *uart_parity)
{
	u8	CfgParity=0;

	if(eeprom_read(EPROM_ADDR_PARITY(port), &CfgParity, 1) == I2C_SUCCESS) {
		*uart_parity = stopbits_list[CfgParity];
		return IF_ERR_OK;
	} else {
		return IF_ERR_I2C;
	}
}


int GetUartFlowCtrl(u8 port, u16 *uart_flow_control)
{
	u8	CfgFlowCtrl=0;

	if(eeprom_read(EPROM_ADDR_FLOWCTRL(port), &CfgFlowCtrl, 1) == I2C_SUCCESS) {
		*uart_flow_control = flowctrl_list[CfgFlowCtrl];
		return IF_ERR_OK;
	} else {
		return IF_ERR_I2C;
	}
}

int GetUartWorkMode(u8 port, u8 *uart_work_mode)
{
	u8	CfgUartWordMode=0xFF;

	if(eeprom_read(EPROM_ADDR_WORK_MODE(port), &CfgUartWordMode, 1) == I2C_SUCCESS) {
		*uart_work_mode = CfgUartWordMode;
		return IF_ERR_OK;
	} else {
		return IF_ERR_I2C;
	}
}



int GetUartFrameSplit(u8 port, u16 *max_interval, u16 *max_datalen)
{

	return IF_ERR_I2C;

}

int GetUartModeTcpServer(u8 port, u16 *listen_port)
{
	if(eeprom_read(EPROM_ADDR_UARTCFG_TCP_SERVER(port), (u8 *)listen_port, 2) == I2C_SUCCESS) {
		return IF_ERR_OK;
	} 

	return IF_ERR_I2C;	
}

int GetUartModeTcpClient(u8 port, uartcfg_tcpclient_t *pServersInfo, u8 info_num)
{
	if(eeprom_read(EPROM_ADDR_UARTCFG_TCP_CLIENT(port), (u8 *)pServersInfo, sizeof(uartcfg_tcpclient_t) * info_num) == I2C_SUCCESS) {
		return IF_ERR_OK;
	} 

	return IF_ERR_I2C;
}

int GetUartModeUdp(u8 port, uartcfg_udp_t *pUdpInfo)
{
	if(eeprom_read(EPROM_ADDR_UARTCFG_UDP(port), (u8 *)pUdpInfo, sizeof(uartcfg_udp_t)) == I2C_SUCCESS) {
		return IF_ERR_OK;
	} 

	return IF_ERR_I2C;
}

int GetUartModeUdpMulticast(u8 port, uartcfg_udp_multicast_t *pUdpMulticastInfo)
{
	if(eeprom_read(EPROM_ADDR_UARTCFG_UDP_MULTICAST(port), (u8 *)pUdpMulticastInfo, sizeof(uartcfg_udp_multicast_t)) == I2C_SUCCESS) {
		return IF_ERR_OK;
	} 

	return IF_ERR_I2C;
}


int SetUartConfiguration(u8 port, uartcfg_t *uartcfg)
{
	if(eeprom_page_write(EPROM_ADDR_UART_EN(port), (u8 *)uartcfg, sizeof(uartcfg_t)) == I2C_SUCCESS) {
		return IF_ERR_OK;
	} 
	return IF_ERR_I2C;
}

int GetUartConfiguration(u8 port, uartcfg_t *uartcfg)
{
	if(eeprom_read(EPROM_ADDR_UART_EN(port), (u8 *)uartcfg, sizeof(uartcfg_t)) == I2C_SUCCESS) {
		return IF_ERR_OK;
	} 
	return IF_ERR_I2C;
}

/*

int UpgradeFlagSet(void)
{
	int ret;
	u8 UpgradeFlag;
	
	UpgradeFlag = 0x80;
	return eeprom_write(EPROM_ADDR_UPGRADE_FLAG, &UpgradeFlag, 1);
}

int UpgradeFlagClear(void)
{
	u8 UpgradeFlag;
	
	UpgradeFlag = 0x00;
	return eeprom_write(EPROM_ADDR_UPGRADE_FLAG, &UpgradeFlag, 1);
}

*/

