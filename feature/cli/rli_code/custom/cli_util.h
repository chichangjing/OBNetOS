#ifndef _CLI_UTIL_H
#define _CLI_UTIL_H

#include "rc.h"
#include "rcc.h"

#define  ALL_ZERO_IP			0x00000000
#define  ALL_ONE_IP				0xFFFFFFFF
#define  IP_CLASS_A_0			0
#define  IP_CLASS_A_1			1
#define  IP_CLASS_B				2
#define  IP_CLASS_C				3
#define  IP_CLASS_D				0xE
#define  IP_CLASS_E				0xF
#define  IP_CLASS_A_NET			0x7F000000
#define  IP_CLASS_A_HOST		0x00FFFFFF
#define  IP_CLASS_B_NET			0x3FFF0000
#define  IP_CLASS_B_HOST		0x0000FFFF
#define  IP_CLASS_C_NET			0x1FFFFF00
#define  IP_CLASS_C_HOST 		0x000000FF
#define  IP_CLASS_A_NET_MASK	(~IP_CLASS_A_HOST)
#define  IP_CLASS_B_NET_MASK	(~IP_CLASS_B_HOST)
#define  IP_CLASS_C_NET_MASK	(~IP_CLASS_C_HOST)

#define CHK_OK						0
#define CHK_ERR						(-1)
#define CHK_ERR_INVALID_IPADDR		(-100)
#define CHK_ERR_INVALID_NETMASK		(-101)
#define CHK_ERR_IP_BROADCAST		(-102)
#define CHK_ERR_IP_SUBNET_ZERO		(-103)
#define CHK_ERR_IP_IS_SUBNET		(-104)
#define CHK_ERR_INVALID_IP_MASK_GW	(-105)
#define CHK_ERR_INVALID_MAC_ADDR	(-200)

/********************************************************************************
	Debug module define
 ********************************************************************************/
#define DBG_ALL					0xFFFFFFFF
#define DBG_NMS					0x00000001
#define DBG_OBRING				0x00000002
#define DBG_CLI					0x00000004
#define DBG_CONFIG				0x00000008
#define DBG_UART				0x00000010
#define DBG_CMD					0x00001000

#define OB_DEBUG(m, fmt, params...) cli_debug(m,fmt,##params)

/********************************************************************************
	Functions
 ********************************************************************************/
unsigned short cli_htons(unsigned short n);
unsigned short cli_ntohs(unsigned short n);
unsigned long cli_htonl(unsigned long n);
unsigned long cli_ntohl(unsigned long n);

void cli_extract_mac(unsigned char *macAddr, char *macStr);
int check_mac(unsigned char *mac_addr);
int check_ipaddress(unsigned int ip_address);
int check_netmask(unsigned int netmask);
int check_ip_mask(unsigned int ip, unsigned int netmask);
int check_ip_mask_gateway(unsigned int ip, unsigned int mask, unsigned int gateway);

void cli_debug_init(void);
void cli_printf(cli_env *pCliEnv, const char *fmt, ...);
void cli_debug(int module, char* fmt, ...);
void cli_dump(int module, unsigned char *buf, int len);
void debug_module_clear(void);
int	cli_debug_module_add(cli_env *pCliEnv, unsigned int module);
unsigned int cli_debug_module_del(cli_env *pCliEnv, unsigned int module);
unsigned int cli_debug_module_read(cli_env *pCliEnv);

#endif

