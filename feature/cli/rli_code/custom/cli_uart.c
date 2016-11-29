
/*************************************************************
 * Filename     : cli_uart.c
 * Description  : Uart server for CLI
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/

#include "mconfig.h"

#if MODULE_UART_SERVER
/* Standard includes */
#include <string.h>
#include <stdlib.h>

/* Kernel includes. */

/* LwIP includes */
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"

/* BSP includes */
#include "robo_drv.h"
#include "flash_if.h"
#include "soft_i2c.h"
#include "usart_if.h"

/* Other includes */
#include "cli_sys.h"
#include "cli_util.h"

#include "conf_comm.h"
#include "conf_uart.h"
#include "uart_server.h"

extern int uart_get_socket_connect(u8 uart_port, u32 conn_index, struct sockaddr_in *sockaddr);


void show_uart_running_stats(cli_env *pCliEnv, ubyte uart_port)
{
	ubyte uart_enable;
	ubyte working_mode;
	uart_dev_t	*pUartDev;
	ubyte2 listen_port;
	struct sockaddr_in client_addr;
	ubyte4 i, conn_num;
	uartcfg_udp_t udpcfg;
	uartcfg_udp_multicast_t udpmulticfg;
	struct in_addr ipaddress;
	
	if(GetUartEnable(uart_port, &uart_enable) != CONF_ERR_NONE) {
		cli_printf(pCliEnv, "Error: read eeprom failed\r\n");
		return;
	}

	if(uart_enable != 0x01) {
		cli_printf(pCliEnv, "UART port %d is disable\r\n", uart_port);
		return;
	}

	pUartDev = GetUartDev(uart_port);

	cli_printf(pCliEnv, "\r\n%s(port%d) stats list :\r\n", (uart_port==0)? "RS485":"RS232", uart_port);
	cli_printf(pCliEnv, "   Uart property       : %d, %s-%d-%d, FlowCtrl: none\r\n", 
							 pUartDev->property.baudrate,
							(pUartDev->property.parity == USART_Parity_No)? "N" : \
							(pUartDev->property.parity == USART_Parity_Even)? "E" : \
							(pUartDev->property.parity == USART_Parity_Odd)? "O" : "?", 
							 pUartDev->property.databits, 
							 pUartDev->property.stopbits);

	if(GetUartWorkMode(uart_port, &working_mode) != CONF_ERR_NONE) {
		cli_printf(pCliEnv, "Error: read eeprom failed\r\n");
		return;
	}
			
	cli_printf(pCliEnv, "   Working mode        : 0x%02x (%s)\r\n", 
							working_mode, 
							(working_mode == 0x00)? "TCP Server Mode" : \
							(working_mode == 0x01)? "TCP Client Mode" : \
							(working_mode == 0x02)? "UDP Mode" : \
							(working_mode == 0x03)? "UDP Multicast Mode" : "Unkown");

	switch(working_mode) {
		case UART_MODE_TCP_SERVER:
			if(GetUartModeTcpServer(uart_port, &listen_port) != CONF_ERR_NONE) {
				cli_printf(pCliEnv, "Error: read eeprom failed\r\n");
				return;
			}
			
			cli_printf(pCliEnv, "   Listen port         : %d\r\n", ntohs(listen_port));
			cli_printf(pCliEnv, "   Connect socket list : ");

			conn_num = 0;
			for(i=0; i<SOCKET_LIST_SIZE; i++) {
				if(uart_get_socket_connect(uart_port, i, &client_addr) < 0)
					continue;
				if(conn_num == 0)
					cli_printf(pCliEnv, "%s:%d\r\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
				else
					cli_printf(pCliEnv, "                       : %s:%d\r\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
				conn_num++;
			}
			if(conn_num == 0)
				cli_printf(pCliEnv, "No client connect\r\n");
		break;

		case UART_MODE_TCP_CLIENT:
			cli_printf(pCliEnv, "   Connect socket list : ");

			conn_num = 0;
			for(i=0; i<SOCKET_LIST_SIZE; i++) {
				if(uart_get_socket_connect(uart_port, i, &client_addr) < 0)
					continue;
				if(conn_num == 0)
					cli_printf(pCliEnv, "%s:%d\r\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
				else
					cli_printf(pCliEnv, "                       : %s:%d\r\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
				conn_num++;
			}
			if(conn_num == 0)
				cli_printf(pCliEnv, "No client connect\r\n");			
		break;
		
		case UART_MODE_UDP:
			if(GetUartModeUdp(uart_port, &udpcfg) != CONF_ERR_NONE) {
				cli_printf(pCliEnv, "Error: read eeprom failed\r\n");
				return;
			}
			ipaddress.s_addr = udpcfg.remote_ip;
			cli_printf(pCliEnv, "   UDP mode configuration ... \r\n");
			cli_printf(pCliEnv, "       LocalPort       : %d\r\n", ntohs(udpcfg.local_port));
			cli_printf(pCliEnv, "       RemotePort      : %d\r\n", ntohs(udpcfg.remote_port));
			cli_printf(pCliEnv, "       RemoteIP        : %s\r\n", inet_ntoa(ipaddress));
		break;

		case UART_MODE_UDP_MULTICAST:
			if(GetUartModeUdpMulticast(uart_port, &udpmulticfg) != CONF_ERR_NONE) {
				cli_printf(pCliEnv, "Error: read eeprom failed\r\n");
				return;
			}			

			ipaddress.s_addr = udpmulticfg.multicast_ip;
			cli_printf(pCliEnv, "   UDP multicast configuration ... \r\n");
			cli_printf(pCliEnv, "       MultiLocalPort  : %d\r\n", ntohs(udpmulticfg.local_port));
			cli_printf(pCliEnv, "       MultiRemotePort : %d\r\n", ntohs(udpmulticfg.remote_port));
			cli_printf(pCliEnv, "       Multicast IP    : %s\r\n", inet_ntoa(ipaddress));
		break;
		
		default:
		break;
	}
	
	cli_printf(pCliEnv, "   Uart rx fifo size   : %d\r\n", pUartDev->fifo.rxSize);
	cli_printf(pCliEnv, "   Uart tx fifo size   : %d\r\n", pUartDev->fifo.txSize);
	cli_printf(pCliEnv, "   Count txdone event  : %d\r\n", pUartDev->stat.count_txdone);
	cli_printf(pCliEnv, "   Count rxidle event  : %d\r\n", pUartDev->stat.count_rxidle);
	cli_printf(pCliEnv, "   Count rxovf  event  : %d\r\n", pUartDev->stat.count_rxovf);
	cli_printf(pCliEnv, "   Count rxfull event  : %d\r\n", pUartDev->stat.count_rxfull);
	cli_printf(pCliEnv, "   Count OverRun error : %d\r\n", pUartDev->stat.count_overrun);
	cli_printf(pCliEnv, "   Count uart tx bytes : %d\r\n", pUartDev->stat.tx_bytes);
	cli_printf(pCliEnv, "   Count uart rx bytes : %d\r\n", pUartDev->stat.rx_bytes);
	cli_printf(pCliEnv, "   Count timeout ethtx : %d\r\n", pUartDev->stat.count_tim_ethtx);	
	cli_printf(pCliEnv, "   Count overflw ethtx : %d\r\n", pUartDev->stat.count_ovf_ethtx);	
	cli_printf(pCliEnv, "\r\n");

	return;
}

RLSTATUS cli_show_uart_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get optional parameter */
    if (OK == RCC_DB_RetrieveParam(pParams, "port", mShowUart_Port, &pParamDescr ))
    {
        pVal = (sbyte*)(pParamDescr->pValue);
    }

    /* TO DO: Add your handler code here */
    {
		ubyte uart_port;
		
		if(pVal != NULL) {
        	CONVERT_StrTo(pVal, &uart_port, kDTuchar);
			if(uart_port > 1) {
				cli_printf(pCliEnv, "\r\nError: Invalid port number, should be 0 or 1\r\n\r\n");
				return STATUS_RCC_NO_ERROR;
			}

			show_uart_running_stats(pCliEnv, uart_port);
		} else {
			show_uart_running_stats(pCliEnv, 0);
			show_uart_running_stats(pCliEnv, 1);
		}
	}
	
	return status;
}

#else
#include "cli_sys.h"

RLSTATUS cli_show_uart_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
	
	return status;
}

#endif

