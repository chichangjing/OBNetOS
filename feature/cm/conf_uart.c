
/*************************************************************
 * Filename     : conf_uart.c
 * Description  : API for UART server configuration
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/

#include "mconfig.h"

/* Standard includes */
#include "stdio.h"
#include "ctype.h"
#include "string.h"

/* BSP includes */
#include "stm32f2xx.h"
#include "soft_i2c.h"
#include "conf_map.h"
#include "conf_comm.h"
#include "conf_uart.h"

const unsigned int baudrate_list[] = {110,300,600,1200,2400,4800,9600,14400,19200,28800,38400,57600,115200,230400,460800,921600};
const unsigned short wordlength_list[] = {USART_WordLength_8b, USART_WordLength_9b};
const unsigned short stopbits_list[] = {USART_StopBits_1,USART_StopBits_0_5,USART_StopBits_2,USART_StopBits_1_5};
const unsigned short parity_list[] = {USART_Parity_No,USART_Parity_Even,USART_Parity_Odd};
const unsigned short flowctrl_list[] = {USART_HardwareFlowControl_None,USART_HardwareFlowControl_RTS,USART_HardwareFlowControl_CTS,USART_HardwareFlowControl_RTS_CTS};


int GetUartEnable(u8 port, u8 *uart_enable)
{
	u8	CfgUartEn=0;
	
	if(eeprom_read(NVRAM_UART_EN(port), &CfgUartEn, 1) == I2C_SUCCESS) {
		*uart_enable = CfgUartEn;
		return CONF_ERR_NONE;
	} else {
		return CONF_ERR_I2C;
	}
}

int GetUartBaudRate(u8 port, u32 *uart_baudrate)
{
	u8	CfgBaudRate=0;
	
	if(eeprom_read(NVRAM_BAUDRATE(port), &CfgBaudRate, 1) == I2C_SUCCESS) {
		*uart_baudrate = baudrate_list[CfgBaudRate];
		return CONF_ERR_NONE;
	} else {
		return CONF_ERR_I2C;
	}
}

int GetUartWordLength(u8 port, u16 *uart_word_length)
{
	u8	CfgWordLength=0;

	if(eeprom_read(NVRAM_WORDLENGTH(port), &CfgWordLength, 1) == I2C_SUCCESS) {
		*uart_word_length = wordlength_list[CfgWordLength];
		return CONF_ERR_NONE;
	} else {
		return CONF_ERR_I2C;
	}
}

int GetUartStopBits(u8 port, u16 *uart_stop_bits)
{
	u8	CfgStopBits=0;
	
	if(eeprom_read(NVRAM_STOPBITS(port), &CfgStopBits, 1) == I2C_SUCCESS) {
		*uart_stop_bits = stopbits_list[CfgStopBits];
		return CONF_ERR_NONE;
	} else {
		return CONF_ERR_I2C;
	}
}

int GetUartParity(u8 port, u16 *uart_parity)
{
	u8	CfgParity=0;

	if(eeprom_read(NVRAM_PARITY(port), &CfgParity, 1) == I2C_SUCCESS) {
		*uart_parity = stopbits_list[CfgParity];
		return CONF_ERR_NONE;
	} else {
		return CONF_ERR_I2C;
	}
}

int GetUartFlowCtrl(u8 port, u16 *uart_flow_control)
{
	u8	CfgFlowCtrl=0;

	if(eeprom_read(NVRAM_FLOWCTRL(port), &CfgFlowCtrl, 1) == I2C_SUCCESS) {
		*uart_flow_control = flowctrl_list[CfgFlowCtrl];
		return CONF_ERR_NONE;
	} else {
		return CONF_ERR_I2C;
	}
}

int GetUartWorkMode(u8 port, u8 *uart_work_mode)
{
	u8	CfgUartWordMode=0xFF;

	if(eeprom_read(NVRAM_WORK_MODE(port), &CfgUartWordMode, 1) == I2C_SUCCESS) {
		*uart_work_mode = CfgUartWordMode;
		return CONF_ERR_NONE;
	} else {
		return CONF_ERR_I2C;
	}
}

int GetUartFrameSplit(u8 port, u16 *max_interval, u16 *max_datalen)
{
	return CONF_ERR_I2C;
}

int GetUartModeTcpServer(u8 port, u16 *listen_port)
{
	if(eeprom_read(NVRAM_UARTCFG_MODE_BASE(port), (u8 *)listen_port, 2) == I2C_SUCCESS) {
		return CONF_ERR_NONE;
	} 

	return CONF_ERR_I2C;	
}

int GetUartModeTcpClient(u8 port, uartcfg_tcpclient_t *pServersInfo, u8 info_num)
{
	if(eeprom_read(NVRAM_UARTCFG_MODE_BASE(port), (u8 *)pServersInfo, sizeof(uartcfg_tcpclient_t) * info_num) == I2C_SUCCESS) {
		return CONF_ERR_NONE;
	} 

	return CONF_ERR_I2C;
}

int GetUartModeUdp(u8 port, uartcfg_udp_t *pUdpInfo)
{
	if(eeprom_read(NVRAM_UARTCFG_MODE_BASE(port), (u8 *)pUdpInfo, sizeof(uartcfg_udp_t)) == I2C_SUCCESS) {
		return CONF_ERR_NONE;
	} 

	return CONF_ERR_I2C;
}

int GetUartModeUdpMulticast(u8 port, uartcfg_udp_multicast_t *pUdpMulticastInfo)
{
	if(eeprom_read(NVRAM_UARTCFG_MODE_BASE(port), (u8 *)pUdpMulticastInfo, sizeof(uartcfg_udp_multicast_t)) == I2C_SUCCESS) {
		return CONF_ERR_NONE;
	} 

	return CONF_ERR_I2C;
}

int SetUartConfiguration(u8 port, uartcfg_t *uartcfg)
{
	if(eeprom_page_write(NVRAM_UART_EN(port), (u8 *)uartcfg, sizeof(uartcfg_t)) == I2C_SUCCESS) {
		return CONF_ERR_NONE;
	} 
	return CONF_ERR_I2C;
}

int GetUartConfiguration(u8 port, uartcfg_t *uartcfg)
{
	if(eeprom_read(NVRAM_UART_EN(port), (u8 *)uartcfg, sizeof(uartcfg_t)) == I2C_SUCCESS) {
		return CONF_ERR_NONE;
	} 
	return CONF_ERR_I2C;
}