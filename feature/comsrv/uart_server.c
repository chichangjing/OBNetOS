
/*************************************************************
 * Filename     : uart_server.c
 * Description  : uart server
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/
#include "mconfig.h"

#if MODULE_UART_SERVER

/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "os_mutex.h"

/* LwIP include */
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"

/* BSP include */
#include "soft_i2c.h"
#include "usart_if.h"

/* Other include */
#include "uart_server.h"
#include "conf_comm.h"
#include "conf_uart.h"


struct socket_conn {
    OS_MUTEX_T sock_mutex;
    int used;
    int socket;
    struct sockaddr_in remote;
} socket_conn_t;

#define UART_RX_BUFFER_SIZE 1024
#define UART_TX_BUFFER_SIZE 1024

extern uart_dev_t UartDevice[];
struct socket_conn socket_list[COM_PORT_MAX][SOCKET_LIST_SIZE];
static u8 UartRxBuffer[COM_PORT_MAX][UART_RX_BUFFER_SIZE];
static u8 UartTxBuffer[COM_PORT_MAX][UART_TX_BUFFER_SIZE];
static u8 ComPort[COM_PORT_MAX] = {0, 1};
xSemaphoreHandle xSemUartTx[COM_PORT_MAX] = { NULL };
xSemaphoreHandle xSemUartRx[COM_PORT_MAX] = { NULL };
static u8 UartWorkingMode[COM_PORT_MAX] = {0};
static u16 TcpSrvListenPort[COM_PORT_MAX] = {0};
static uartcfg_tcpclient_t TcpServerList[COM_PORT_MAX][SOCKET_LIST_SIZE];
static u8 usEthRxBuffer[COM_PORT_MAX][UART_RX_BUFFER_SIZE];
static u8 usEthTxBuffer[COM_PORT_MAX][UART_TX_BUFFER_SIZE];
static uartcfg_udp_multicast_t UdpMulticastInfo[COM_PORT_MAX] = {0};
static uartcfg_udp_t UdpInfo[COM_PORT_MAX] = {0};

static unsigned short rx_buf_offset[COM_PORT_MAX] = {0};
unsigned int uart_tim[COM_PORT_MAX] = {0};

void socket_conn_init(void)
{
    int i, port;

	for(port=0; port<COM_PORT_MAX; port++) {
	    memset(socket_list[port],0,sizeof(socket_conn_t));
	    for( i = 0 ; i < SOCKET_LIST_SIZE ; i++ ) {
			os_mutex_init(&(socket_list[port][i].sock_mutex));
	    }
	}
}


void tcp_listen_task(void *arg)
{
	u8 com_port = *(u8 *)arg;
	u16 listenPort;
	struct sockaddr_in local;
	int listenfd;		
	fd_set readfds, tmpfds;	

	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = TcpSrvListenPort[com_port];
	
	if((listenfd = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		printf("Create socket failed\r\n");
		return;
	}
	
	if(lwip_bind(listenfd, (struct sockaddr *)&local, sizeof(local)) < 0) {
		lwip_close(listenfd);
		printf("Bind failed\r\n");
		return;
	}
	
	if (lwip_listen(listenfd, MAX_LISTEN_SOCK) < 0) {
		lwip_close(listenfd);
		printf("Listen failed.\r\n");
		return;
	}

	for(;;) {
		FD_ZERO(&readfds);
		FD_ZERO(&tmpfds);
		FD_SET(listenfd, &readfds);			

		tmpfds = readfds;
		if (lwip_select(listenfd+1, &tmpfds, 0, 0, 0) == 0) continue;

		if(FD_ISSET(listenfd, &tmpfds)) {
			int i;
			socklen_t addrlen = sizeof(struct sockaddr_in);

			for(i=0 ; i<SOCKET_LIST_SIZE; i++) {
				if(os_mutex_lock(&(socket_list[com_port][i].sock_mutex), OS_MUTEX_NO_WAIT) == OS_MUTEX_ERROR_TIMEOUT)
					continue;
				
				if(socket_list[com_port][i].used) {
					os_mutex_unlock(&(socket_list[com_port][i].sock_mutex));
					continue;
				}
				
				if((socket_list[com_port][i].socket = accept(listenfd, (struct sockaddr *)&(socket_list[com_port][i].remote), &addrlen)) < 0) {
					printf("-> Accept failed\r\n");
				} else {
					int optval;
					
					lwip_setsockopt(socket_list[com_port][i].socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
					socket_list[com_port][i].used = 1;
					printf("-> Accept connect(%d): %s:%d\r\n", i, inet_ntoa(socket_list[com_port][i].remote.sin_addr), ntohs(socket_list[com_port][i].remote.sin_port));
				}
				
				os_mutex_unlock(&(socket_list[com_port][i].sock_mutex));
			}
			
            if(i == SOCKET_LIST_SIZE) {
                int sock;
                struct sockaddr cliaddr;
                socklen_t clilen;

				if((sock = accept(listenfd, &cliaddr, &clilen)) >= 0)
                    lwip_close(sock);
				
                printf("-> Accept and close connect\r\n");
            }			
		} else {
			printf("Error: should not reach here!\n");
		}
	}
}

void tcp_connect_task(void *arg)
{
	int already_connected;
	u8 com_port = *(u8 *)arg;
	
	for(;;) {
		int i, sock_index;
		
		for(i=0; i<MAX_SERVER_NUMBER; i++) {
            int optval;
            int tmpsock;
			struct sockaddr_in tmpaddr;
			
			if(TcpServerList[com_port][i].valid != 0x01)
				continue;

			already_connected = 0;
			for(sock_index=0; sock_index<SOCKET_LIST_SIZE; sock_index++) {
				os_mutex_lock(&(socket_list[com_port][sock_index].sock_mutex),OS_MUTEX_WAIT_FOREVER);
				if(socket_list[com_port][sock_index].used) {
					if(	(TcpServerList[com_port][i].server_ip == socket_list[com_port][sock_index].remote.sin_addr.s_addr) && \
						(TcpServerList[com_port][i].server_port == socket_list[com_port][sock_index].remote.sin_port) ) {
						already_connected = 1;
						os_mutex_unlock(&(socket_list[com_port][sock_index].sock_mutex));
						break;
					}
				}
				os_mutex_unlock(&(socket_list[com_port][sock_index].sock_mutex));
			}

			if(already_connected == 1) 
				continue;

			if((tmpsock = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
				printf("Create socket failed\r\n");
				continue;
            }
			optval = 1;
            lwip_setsockopt(tmpsock, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
			
            tmpaddr.sin_family = AF_INET;
            tmpaddr.sin_port = TcpServerList[com_port][i].server_port;
            tmpaddr.sin_addr.s_addr = TcpServerList[com_port][i].server_ip;
            memset(&(tmpaddr.sin_zero), 0, sizeof(tmpaddr.sin_zero));
            if(lwip_connect(tmpsock,(struct sockaddr *)&(tmpaddr),sizeof(tmpaddr)) == -1) {
				//printf("Can't connect to %s:%d\r\n", inet_ntoa(tmpaddr.sin_addr), ntohs(tmpaddr.sin_port));
				lwip_close(tmpsock);
				continue;
			} else {
				printf("Connected to %s:%d\r\n", inet_ntoa(tmpaddr.sin_addr), ntohs(tmpaddr.sin_port));
			}
			
            /* Find an empty slot to store the socket. */
            for( sock_index = 0 ; sock_index < SOCKET_LIST_SIZE ; sock_index++ ) {
				if(os_mutex_lock(&(socket_list[com_port][sock_index].sock_mutex), OS_MUTEX_NO_WAIT) == OS_MUTEX_ERROR_TIMEOUT)
					continue;
				if(socket_list[com_port][sock_index].used) {
					os_mutex_unlock(&(socket_list[com_port][sock_index].sock_mutex));
					continue;
				}

                socket_list[com_port][sock_index].socket = tmpsock;
                socket_list[com_port][sock_index].remote = tmpaddr;
                socket_list[com_port][sock_index].used = 1;
                os_mutex_unlock(&(socket_list[com_port][sock_index].sock_mutex));
                break;
            }
            /* Can't find any empty slot */
            if( sock_index == SOCKET_LIST_SIZE ) {
                printf("No space to store socket, close socket!\r\n");
                lwip_close(tmpsock);
            }	
		}	
	vTaskDelay(1000);
	}
}


void udp_mode_entry(void *arg)
{
	u8 com_port = *(u8 *)arg;
	struct conn_session *session;
	int udp_sock;
	int optval = 1;
	struct sockaddr_in local, remote;
	unsigned short local_port,remote_port;
	
	if((udp_sock = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("Create udp socket failed\r\n");
		vTaskDelete( NULL );
		return;
	}

	lwip_setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));

	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = UdpInfo[com_port].local_port;

	if (lwip_bind(udp_sock,(struct sockaddr *)&local, sizeof(struct sockaddr)) < 0) {
		printf("Bind error\n");
		lwip_close(udp_sock);
		vTaskDelete( NULL );
		return;
	}

	memset(&remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = UdpInfo[com_port].remote_ip;
	remote.sin_port = UdpInfo[com_port].remote_port;

    os_mutex_lock(&(socket_list[com_port][0].sock_mutex), OS_MUTEX_WAIT_FOREVER);
    socket_list[com_port][0].socket = udp_sock;
    socket_list[com_port][0].remote = remote;	
    socket_list[com_port][0].used = 1;
    os_mutex_unlock(&(socket_list[com_port][0].sock_mutex));
	
	vTaskDelete( NULL );
}


void udp_multicast_mode_entry(void *arg)
{
	u8 com_port = *(u8 *)arg;
	int udp_sock;
	int optval = 1;
	struct sockaddr_in local, remote;
	unsigned short local_port,remote_port;
	int ret;
	struct ip_mreq mreq;
	
	if((udp_sock = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("Create udp socket failed\r\n");
		vTaskDelete( NULL );
		return;
	}

	lwip_setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));

	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = UdpMulticastInfo[com_port].local_port;

	if (lwip_bind(udp_sock,(struct sockaddr *)&local, sizeof(struct sockaddr)) < 0) {
		printf("Bind error\n");
		lwip_close(udp_sock);
		vTaskDelete( NULL );
		return;
	}

	memset(&remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = UdpMulticastInfo[com_port].multicast_ip;
	remote.sin_port = UdpMulticastInfo[com_port].remote_port;

	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	mreq.imr_multiaddr.s_addr = remote.sin_addr.s_addr;
	if((ret=lwip_setsockopt(udp_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq))) != ERR_OK) {
		printf("join group failed, ret=%d\n", ret);
		lwip_close(udp_sock);
		vTaskDelete( NULL );
		return;
	}

    os_mutex_lock(&(socket_list[com_port][0].sock_mutex), OS_MUTEX_WAIT_FOREVER);
    socket_list[com_port][0].socket = udp_sock;
    socket_list[com_port][0].remote = remote;	
    socket_list[com_port][0].used = 1;
    os_mutex_unlock(&(socket_list[com_port][0].sock_mutex));
	
	vTaskDelete( NULL );
}

void uart_send_task(void *arg)
{
	u8 com_port = *(u8 *)arg;
	int maxfd;
	fd_set readfds, tmpfds;
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = 100 * 1000;
	
	for(;;) {
		int i;
		
		FD_ZERO(&readfds);
		maxfd = 0;
		for(i=0; i<SOCKET_LIST_SIZE; i++) {
			if( socket_list[com_port][i].used ) {
				if(maxfd <= socket_list[com_port][i].socket)
					maxfd = socket_list[com_port][i].socket + 1;
				FD_SET(socket_list[com_port][i].socket, &readfds);
			}
		}
		
		if(maxfd == 0) {
			vTaskDelay(100);
			continue;
		}

		tmpfds = readfds;
		if (lwip_select(maxfd, &tmpfds, NULL, 0, &timeout) == 0) 
			continue;

		for(i=0; i<SOCKET_LIST_SIZE; i++) {
			if(os_mutex_lock(&(socket_list[com_port][i].sock_mutex),10) == OS_MUTEX_ERROR_TIMEOUT) {
				continue;
			}
			
			if(socket_list[com_port][i].used && FD_ISSET(socket_list[com_port][i].socket, &tmpfds)) {
				int rxlen;

				rxlen = lwip_recv(socket_list[com_port][i].socket, usEthRxBuffer[com_port], UART_TX_BUFFER_SIZE, MSG_DONTWAIT);
				if((rxlen == 0) || (rxlen == -1)) {
					printf("(%d)-> %s:%d is disconnect\r\n", i, inet_ntoa(socket_list[com_port][i].remote.sin_addr), ntohs(socket_list[com_port][i].remote.sin_port));
					lwip_close(socket_list[com_port][i].socket);
					socket_list[com_port][i].used = 0;
				} else {
					int uart_txCnt = 0;
					int uart_tx_offset = 0;

					UartLedFlash(com_port);
					do {
						if(xSemaphoreTake(xSemUartTx[com_port], portMAX_DELAY) != pdTRUE) {}
						if(com_port == COM_PORT_0)
							RS485TxEnable(com_port);	
						uart_txCnt = UartWrite(com_port, usEthRxBuffer[com_port]+uart_tx_offset, rxlen-uart_tx_offset);						
						UartDevice[com_port].stat.tx_bytes += uart_txCnt;	
						uart_tx_offset+=uart_txCnt;
						
					} while(uart_tx_offset < rxlen);
				}
			}
            os_mutex_unlock(&(socket_list[com_port][i].sock_mutex));
		}
	}
}

void uart_recv_task(void *arg)
{
	int ret;
	u8 com_port = *(u8 *)arg;
	u16 max_interval,max_datalen;
	u8 working_mode;
	u16 read_len;
	
	for(;;) {
		if(xSemaphoreTake(xSemUartRx[com_port], portMAX_DELAY) == pdTRUE) {

			read_len = UartRead(com_port, usEthTxBuffer[com_port]+rx_buf_offset[com_port], UART_RX_BUFFER_SIZE-rx_buf_offset[com_port]);
			UartDevice[com_port].stat.rx_bytes += read_len;
			
			if(rx_buf_offset[com_port] == 0) {
				if(read_len == 0) {
					continue;
				} else {
					uart_tim[com_port] = 200;
				}			
			}
			
			rx_buf_offset[com_port] += read_len;
			if((rx_buf_offset[com_port] < UART_RX_BUFFER_SIZE/2) && (uart_tim[com_port] > 0)) {
				continue;
			}
			if(uart_tim[com_port] == 0)
				UartDevice[com_port].stat.count_tim_ethtx++;
			if(rx_buf_offset[com_port] >= UART_RX_BUFFER_SIZE/2)
				UartDevice[com_port].stat.count_ovf_ethtx++;		

			UartLedFlash(com_port);
			
			if((UartWorkingMode[com_port] == UART_MODE_TCP_SERVER) || (UartWorkingMode[com_port] == UART_MODE_TCP_CLIENT)) {
				int i;
				
				for(i=0; i<SOCKET_LIST_SIZE; i++) {
					if(os_mutex_lock(&(socket_list[com_port][i].sock_mutex),10) != OS_MUTEX_SUCCESS)
						continue;
					if(socket_list[com_port][i].used) {
						#if 1
						if(lwip_send(socket_list[com_port][i].socket, usEthTxBuffer[com_port], rx_buf_offset[com_port], 0) < 0) {
							lwip_close(socket_list[com_port][i].socket);
							socket_list[com_port][i].used = 0;
							printf("Connect lost!\r\n");
						}
						#else
						//lwip_send(socket_list[com_port][i].socket, usEthTxBuffer[com_port], rx_buf_offset[com_port], 0);
						#endif
					}
					os_mutex_unlock(&(socket_list[com_port][i].sock_mutex));
				}
			} else if((UartWorkingMode[com_port] == UART_MODE_UDP) || (UartWorkingMode[com_port] == UART_MODE_UDP_MULTICAST)) {
				if(os_mutex_lock(&(socket_list[com_port][0].sock_mutex),10) == OS_MUTEX_SUCCESS) {
					if(socket_list[com_port][0].used) {
						lwip_sendto(socket_list[com_port][0].socket, usEthTxBuffer[com_port], rx_buf_offset[com_port], 0, (struct sockaddr *)&(socket_list[com_port][0].remote), sizeof(struct sockaddr));
					}
					os_mutex_unlock(&(socket_list[com_port][0].sock_mutex));
				}
			}
			rx_buf_offset[com_port] = 0;
			/*
			if((UartDevice[com_port].uart)->CR1 & (uint16_t)0x0020 == 0)
				(UartDevice[com_port].uart)->CR1 |= (uint16_t)0x0020;

			if(UartDevice[com_port].stat.count_rxfull>0) {
				UartDevice[com_port].stat.count_rxfull = 0;
				USART_ITConfig(UartDevice[com_port].uart, USART_IT_RXNE, ENABLE);
				USART_ITConfig(UartDevice[com_port].uart, USART_IT_IDLE, ENABLE);
			}
			if(UartDevice[com_port].stat.count_rxovf>0) {
				UartDevice[com_port].stat.count_rxovf = 0;
				USART_ITConfig(UartDevice[com_port].uart, USART_IT_RXNE, ENABLE);
				USART_ITConfig(UartDevice[com_port].uart, USART_IT_IDLE, ENABLE);
			}	
			*/
		}
	}
}

int uart_get_socket_connect(u8 uart_port, u32 conn_index, struct sockaddr_in *sockaddr)
{
	if(socket_list[uart_port][conn_index].used) {
		sockaddr->sin_addr.s_addr = socket_list[uart_port][conn_index].remote.sin_addr.s_addr;
		sockaddr->sin_port = socket_list[uart_port][conn_index].remote.sin_port;
		return 0;
	} else
		return -1;
}

void UartServerStart(void)
{
	int i;
	u8 port;
	u8 CfgUartEnable=0;
	u8 CfgUartWorkMode=0;
	uart_setup_t uart_set;
	USART_InitTypeDef USART_InitStructure;

	socket_conn_init();
	
	for(port=0; port<COM_PORT_MAX; port++) {		
		if(GetUartEnable(port, &CfgUartEnable) != CONF_ERR_NONE)
			CfgUartEnable = 0xFF;
		
		/* Uart enable */
		if(CfgUartEnable == 0x01) { 
			uart_set.rxSize 	= UART_RX_BUFFER_SIZE;
			uart_set.rxBuf 		= UartRxBuffer[port];
			uart_set.txSize 	= UART_TX_BUFFER_SIZE;
			uart_set.txBuf 		= UartTxBuffer[port];
			UartDevSetup(port, uart_set);
			
			if(xSemUartTx[port] == NULL) {
				vSemaphoreCreateBinary(xSemUartTx[port]);
				//xSemUartTx[port] = xSemaphoreCreateCounting(50,0);
			}
			xSemaphoreGive(xSemUartTx[port]);
			
			if(xSemUartRx[port] == NULL) {
				vSemaphoreCreateBinary(xSemUartRx[port]);
				//xSemUartRx[port] = xSemaphoreCreateCounting(50,0);
			}

			/* BaudRate */
			if(GetUartBaudRate(port, &(USART_InitStructure.USART_BaudRate)) != CONF_ERR_NONE) 
				USART_InitStructure.USART_BaudRate = 9600;
			UartDevice[port].property.baudrate = USART_InitStructure.USART_BaudRate;
			
			/* DataBits */
			if(GetUartWordLength(port, &(USART_InitStructure.USART_WordLength)) != CONF_ERR_NONE) 
				USART_InitStructure.USART_WordLength = USART_WordLength_8b;
			
			if(USART_InitStructure.USART_WordLength == USART_WordLength_8b)
				UartDevice[port].property.databits = 8;
			else if(USART_InitStructure.USART_WordLength == USART_WordLength_9b)
				UartDevice[port].property.databits = 9;
			else 
				UartDevice[port].property.databits = 0;

			/* StopBits */
			if(GetUartStopBits(port, &(USART_InitStructure.USART_StopBits)) != CONF_ERR_NONE) 
				USART_InitStructure.USART_StopBits = USART_StopBits_1;
			
			if(USART_InitStructure.USART_StopBits == USART_StopBits_1)
				UartDevice[port].property.stopbits = 1;
			else if(USART_InitStructure.USART_StopBits == USART_StopBits_2)
				UartDevice[port].property.stopbits = 2;
			else 
				UartDevice[port].property.stopbits = 0;
			
			/* Parity */
			if(GetUartParity(port, &(USART_InitStructure.USART_Parity)) != CONF_ERR_NONE) 
				USART_InitStructure.USART_Parity = USART_Parity_No;
			
			UartDevice[port].property.parity = USART_InitStructure.USART_Parity;

			/* FlowCtrl */
			if(GetUartFlowCtrl(port, &(USART_InitStructure.USART_HardwareFlowControl)) != CONF_ERR_NONE) 
				USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
			UartDevice[port].property.flowctrl = USART_HardwareFlowControl_None;

			/* Mode */
			USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;


			if(GetUartWorkMode(port, &CfgUartWorkMode) != CONF_ERR_NONE)
				CfgUartWorkMode = 0x01;	/* Default TCP Client mode */

			UartWorkingMode[port] = CfgUartWorkMode;
			switch(CfgUartWorkMode) {
				case UART_MODE_TCP_SERVER:	
				if(GetUartModeTcpServer(port, &TcpSrvListenPort[port]) != CONF_ERR_NONE)
					break;
				
				printf("COM-Port%d working in TCP Server mode, Listen port %d\r\n", port, ntohs(TcpSrvListenPort[port]));				
				xTaskCreate(uart_recv_task, "tUartRx", configMINIMAL_STACK_SIZE * 1, &ComPort[port], tskIDLE_PRIORITY + 10, NULL);
				sys_thread_new("tUartTx", uart_send_task, &ComPort[port], configMINIMAL_STACK_SIZE * 2, tskIDLE_PRIORITY + 8);
				sys_thread_new("tTcpServer", tcp_listen_task, &ComPort[port], configMINIMAL_STACK_SIZE * 1, tskIDLE_PRIORITY + 8);
				break;

				case UART_MODE_TCP_CLIENT:
				if(GetUartModeTcpClient(port, TcpServerList[port], MAX_SERVER_NUMBER) != CONF_ERR_NONE) {
					break;
				}
				
				printf("COM-Port%d working in TCP Client mode, Servers list :\r\n", port);
				for(i=0; i<MAX_SERVER_NUMBER; i++) {
					struct in_addr addrin;
					
					addrin.s_addr = TcpServerList[port][i].server_ip;
					if(TcpServerList[port][i].valid == 0x01)
						printf(" # ip:%16s, port: %d\r\n", inet_ntoa(addrin), ntohs(TcpServerList[port][i].server_port));
				}
				xTaskCreate(uart_recv_task, "tUartRx", configMINIMAL_STACK_SIZE * 1, &ComPort[port], tskIDLE_PRIORITY + 10, NULL);
				sys_thread_new("tUartTx", uart_send_task, &ComPort[port], configMINIMAL_STACK_SIZE * 2, tskIDLE_PRIORITY + 8);
				sys_thread_new("tTcpClient", tcp_connect_task, &ComPort[port], configMINIMAL_STACK_SIZE * 1, tskIDLE_PRIORITY + 8);
				break;

				case UART_MODE_UDP:
				if(GetUartModeUdp(port, &UdpInfo[port]) != CONF_ERR_NONE)
					break;
				
				printf("COM-Port%d working in UDP mode\r\n", port);			
				sys_thread_new("tUartRx", uart_recv_task, &ComPort[port], configMINIMAL_STACK_SIZE * 1, tskIDLE_PRIORITY + 10);
				sys_thread_new("tUartTx", uart_send_task, &ComPort[port], configMINIMAL_STACK_SIZE * 2, tskIDLE_PRIORITY + 8);
				xTaskCreate(udp_mode_entry, "tUdpMode", configMINIMAL_STACK_SIZE * 1, &ComPort[port], tskIDLE_PRIORITY + 8, NULL);
				break;

				case UART_MODE_UDP_MULTICAST:
				if(GetUartModeUdpMulticast(port, &UdpMulticastInfo[port]) != CONF_ERR_NONE)
					break;
				
				printf("COM-Port%d working in UDP multicast mode\r\n", port);
				sys_thread_new("tUartRx", uart_recv_task, &ComPort[port], configMINIMAL_STACK_SIZE * 1, tskIDLE_PRIORITY + 10);
				sys_thread_new("tUartTx", uart_send_task, &ComPort[port], configMINIMAL_STACK_SIZE * 2, tskIDLE_PRIORITY + 8);
				xTaskCreate(udp_multicast_mode_entry, "tUdpMulti", configMINIMAL_STACK_SIZE * 1, &ComPort[port], tskIDLE_PRIORITY + 8, NULL);
				break;
			}

			UartLowInit(port, &USART_InitStructure);
			if(port == COM_PORT_0) {
				RS485GpioInit(port);
				RS485RxEnable(port);
			}
		}
	}
}

#endif

