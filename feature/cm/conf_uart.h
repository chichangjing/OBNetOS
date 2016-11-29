
#ifndef __CONF_UART_H
#define __CONF_UART_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f2xx.h"


#define MAX_SERVER_NUMBER		8

enum {
	UART_MODE_TCP_SERVER = 0x00,
	UART_MODE_TCP_CLIENT,
	UART_MODE_UDP,
	UART_MODE_UDP_MULTICAST,
	UART_MODE_NUM,
};

typedef struct uarcfg_tcp_server {
	unsigned short	server_port;
} uartcfg_tcpsrv_t;

typedef struct uarcfg_tcp_client {
	unsigned char	valid;
	unsigned char	reserved;
	unsigned short	server_port;
	unsigned int 	server_ip;
} uartcfg_tcpclient_t;

typedef struct uartcfg_udp {
	unsigned short	local_port;
	unsigned short	remote_port;
	unsigned int 	remote_ip;
} uartcfg_udp_t;

typedef struct uartcfg_udp_multicast {
	unsigned short	local_port;
	unsigned short	remote_port;
	unsigned int 	multicast_ip;
} uartcfg_udp_multicast_t;


typedef __packed struct _uartcfg
{
	u8	UartEn;
	u8	Baudrate;
	u8	DataBits;
	u8	StopBits;
	u8	Parity;
	u8	FlowCtrl;
	u8	Reserved1[2];
	
	u8	WorkMode;
	u8	Reserved2[7];
	union {
		uartcfg_tcpsrv_t ListenPort;		/* TcpServer mode configuration */
		uartcfg_tcpclient_t TcpSrvList[8];	/* TcpClient mode configuration */
		uartcfg_udp_t Udp;					/* Udp mode configuration */
		uartcfg_udp_multicast_t UdpMcast;	/* Udp multicast mode configuration */
	} ModeConfig;
	
} uartcfg_t;

int GetUartEnable(u8 port, u8 *uart_enable);
int GetUartBaudRate(u8 port, u32 *uart_baudrate);
int GetUartWordLength(u8 port, u16 *uart_word_length);
int GetUartStopBits(u8 port, u16 *uart_stop_bits);
int GetUartParity(u8 port, u16 *uart_parity);
int GetUartFlowCtrl(u8 port, u16 *uart_flow_control);
int GetUartWorkMode(u8 port, u8 *uart_work_mode);
int GetUartFrameSplit(u8 port, u16 *max_interval, u16 *max_datalen);

int GetUartModeTcpServer(u8 port, u16 *listen_port);
int GetUartModeTcpClient(u8 port, uartcfg_tcpclient_t *pServersInfo, u8 info_num);
int GetUartModeUdp(u8 port, uartcfg_udp_t *pUdpInfo);
int GetUartModeUdpMulticast(u8 port, uartcfg_udp_multicast_t *pUdpMulticastInfo);

int SetUartConfiguration(u8 port, uartcfg_t *uartcfg);
int GetUartConfiguration(u8 port, uartcfg_t *uartcfg);

#ifdef __cplusplus
}
#endif

#endif

