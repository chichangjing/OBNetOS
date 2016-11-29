
#ifndef __NMS_UART_H__
#define __NMS_UART_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f2xx.h"
#include "conf_uart.h"

typedef __packed struct obnet_set_uart
{
	u8	GetCode;
	u8	RetCode;
	u8	UartEn;
	u8	UartPort;
	u8	Baudrate;
	u8	DataBits;
	u8	StopBits;
	u8	Parity;
	u8	FlowCtrl;
	u8	WorkMode;
	union {
		uartcfg_tcpsrv_t ListenPort;			/* TcpServer mode configuration */	
		uartcfg_tcpclient_t TcpSrvList[8];		/* TcpClient mode configuration */
		uartcfg_udp_t Udp;						/* Udp mode configuration */
		uartcfg_udp_multicast_t UdpMcast;		/* Udp multicast mode configuration */
	} ModeConfig;
} OBNET_REQ_SET_UART, *POBNET_REQ_SET_UART, OBNET_RSP_GET_UART, *POBNET_RSP_GET_UART;

typedef struct obnet_get_uart
{
	u8	GetCode;
	u8	RetCode;
	u8	ComPort;
} OBNET_REQ_GET_UART, *POBNET_REQ_GET_UART;

void Rsp_SetUart(u8 *DMA, u8 *RequestID,u8 *UartConfig);
void Rsp_GetUart(u8 *DMA, u8 *RequestID, u8 *MsgGetUartCfg);

#ifdef __cplusplus
}
#endif

#endif

