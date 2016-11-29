
/*************************************************************
 * Filename     : nms_uart.c
 * Description  : API for NMS interface
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/

#include "mconfig.h"

/* Standard includes */
#include "string.h"

/* Kernel includes */

/* BSP includes */
#include "stm32f2xx.h"

/* Other includes */
#include "nms_comm.h"
#include "nms_if.h"
#include "nms_uart.h"
#include "conf_comm.h"
#include "conf_uart.h"

extern u8 NMS_TxBuffer[];

void Rsp_SetUart(u8 *DMA, u8 *RequestID,u8 *UartConfig)
{
	POBNET_REQ_SET_UART pUartCfg = (POBNET_REQ_SET_UART)UartConfig;
	u8 UartPort;
	uartcfg_t CfgData;
	OBNET_SET_RSP RspSet;	
	u16 RspLength;
	int ret;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/**************************************************/
	/* fill the response data */
	/**************************************************/
	/* To add */
	memset(&RspSet, 0, sizeof(OBNET_SET_RSP));
	RspSet.GetCode = CODE_SET_UART;
	
#if MODULE_UART_SERVER	
	memset(&CfgData, 0x00, sizeof(uartcfg_t));
	UartPort = pUartCfg->UartPort - 1;
	CfgData.UartEn		= pUartCfg->UartEn;
	CfgData.Baudrate	= pUartCfg->Baudrate;
	CfgData.DataBits	= pUartCfg->DataBits;
	CfgData.StopBits	= pUartCfg->StopBits;
	CfgData.Parity		= pUartCfg->Parity;
	CfgData.FlowCtrl	= pUartCfg->FlowCtrl;
	CfgData.WorkMode	= pUartCfg->WorkMode;

	switch(CfgData.WorkMode) {
		case UART_MODE_TCP_SERVER:
			memcpy((void *)&(CfgData.ModeConfig.ListenPort), (void *)&(pUartCfg->ModeConfig.ListenPort), sizeof(uartcfg_tcpsrv_t));
		break;

		case UART_MODE_TCP_CLIENT:
			memcpy((void *)(CfgData.ModeConfig.TcpSrvList), (void *)(pUartCfg->ModeConfig.TcpSrvList), MAX_SERVER_NUMBER * sizeof(uartcfg_tcpclient_t));
		break;

		case UART_MODE_UDP:
			memcpy((void *)&(CfgData.ModeConfig.Udp), (void *)&(pUartCfg->ModeConfig.Udp), sizeof(uartcfg_udp_t));
		break;

		case UART_MODE_UDP_MULTICAST:
			memcpy((void *)&(CfgData.ModeConfig.UdpMcast), (void *)&(pUartCfg->ModeConfig.UdpMcast), sizeof(uartcfg_udp_multicast_t));
		break;	

		default:
		break;
	}

	if(UartPort > 1) {
		RspSet.RetCode = 0x01;
		RspSet.Res = 0x00;
	} else {
		
		if((ret = SetUartConfiguration(UartPort, &CfgData)) == CONF_ERR_NONE) {
			RspSet.RetCode = 0x00;
			RspSet.Res = 0x00;
		} else {
			RspSet.RetCode = 0x01;
			RspSet.Res = 0x00;
		}
	}
#else
	RspSet.RetCode = 0x01;
	RspSet.Res = RSP_ERR_FEATURE_NOT_SUPPORT;
#endif

	/************************************************/
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspSet, sizeof(OBNET_SET_RSP));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);			
}


void Rsp_GetUart(u8 *DMA, u8 *RequestID, u8 *MsgGetUartCfg)
{
	POBNET_REQ_GET_UART pMsg = (POBNET_REQ_GET_UART)MsgGetUartCfg;	
	OBNET_RSP_GET_UART RspGetUart;
	uartcfg_t UartConfig;
	u16 RspLength;
	u8 UartPort;
	int ret;
	
	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_RSP_GET_UART);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/**************************************************/
	/* fill the response data */
	/**************************************************/
	/* To add */
	memset(&RspGetUart, 0, sizeof(OBNET_RSP_GET_UART));
	RspGetUart.GetCode = CODE_GET_UART;

#if MODULE_UART_SERVER
	memset(&UartConfig, 0, sizeof(uartcfg_t));
	UartPort = pMsg->ComPort - 1;
	ret = GetUartConfiguration(UartPort, &UartConfig);
	if(ret == CONF_ERR_NONE) {
		RspGetUart.RetCode = 0x00;
		
		if(UartConfig.UartEn == 0x01)			/* UartEnable */
			RspGetUart.UartEn = 0x01;
		else
			RspGetUart.UartEn = 0x00;

		RspGetUart.UartPort = pMsg->ComPort;	/* Com port */

		if(UartConfig.Baudrate > 0x0E)
			RspGetUart.Baudrate = 0x0C;			/* Use default baudrate: 115200 */
		else
			RspGetUart.Baudrate = UartConfig.Baudrate;
		
		if(UartConfig.DataBits > 0x01)
			RspGetUart.DataBits = 0x00;			/* Use default databits: 8 */
		else
			RspGetUart.DataBits = UartConfig.DataBits;

		if(UartConfig.StopBits > 0x01)
			RspGetUart.StopBits = 0x00;			/* Use default stopbits: 1 */
		else
			RspGetUart.StopBits = UartConfig.StopBits;

		if(UartConfig.Parity > 0x02)
			RspGetUart.Parity = 0x00;			/* Use default Parity: None */
		else
			RspGetUart.Parity = UartConfig.Parity;
		
		RspGetUart.FlowCtrl = 0x00;	    		/* Use default Parity: None */

		if(UartConfig.WorkMode > 0x03)
			RspGetUart.WorkMode = 0x03;			/* Use default mode: Udp multicast */
		else
			RspGetUart.WorkMode = UartConfig.WorkMode;
		
		switch(UartConfig.WorkMode) {
			case UART_MODE_TCP_SERVER:
				memcpy((void *)&(RspGetUart.ModeConfig.ListenPort), (void *)&(UartConfig.ModeConfig.ListenPort), sizeof(uartcfg_tcpsrv_t));
			break;

			case UART_MODE_TCP_CLIENT:
				memcpy((void *)(RspGetUart.ModeConfig.TcpSrvList), (void *)(UartConfig.ModeConfig.TcpSrvList), MAX_SERVER_NUMBER * sizeof(uartcfg_tcpclient_t));
			break;

			case UART_MODE_UDP:
				memcpy((void *)&(RspGetUart.ModeConfig.Udp), (void *)&(UartConfig.ModeConfig.Udp), sizeof(uartcfg_udp_t));
			break;

			case UART_MODE_UDP_MULTICAST:
				memcpy((void *)&(RspGetUart.ModeConfig.UdpMcast), (void *)&(UartConfig.ModeConfig.UdpMcast), sizeof(uartcfg_udp_multicast_t));
			break;	

			default:
			break;
		}	
	} else {
		RspGetUart.RetCode = 0x01;
	}
	
#else
	RspGetUart.RetCode = 0x01;
#endif
	/**************************************************/
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspGetUart, sizeof(OBNET_RSP_GET_UART));
	if(RspLength == MSG_MINSIZE)
		RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);	
	else
		RspSend(NMS_TxBuffer, RspLength);	
}

