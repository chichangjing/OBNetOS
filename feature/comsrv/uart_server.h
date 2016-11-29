

#ifndef __UART_SERVER_H
#define __UART_SERVER_H

#define MAX_LISTEN_SOCK 		10
#define SOCKET_BUF_LEN 			128
#define SOCKET_LIST_SIZE 		8

#define TCP_SERVER_PORT_BASE	6010

void UartServerStart(void);

#endif


