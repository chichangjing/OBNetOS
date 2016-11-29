/**
  ******************************************************************************
  * @file    netconf.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    07-October-2011
  * @brief   Network connection configuration
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

#include "mconfig.h"

#if MODULE_LWIP
/* Includes ------------------------------------------------------------------*/
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "main.h"
#include "netconf.h"
#include "tcpip.h"
#include <stdio.h>
#include <string.h>

#include "misc_drv.h"
#include "soft_i2c.h"
#include "cli_util.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define MAX_DHCP_TRIES 5

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct netif xnetif; /* network interface structure */

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
void LwIP_Init(void)
{
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;

	unsigned char cfg_ip[4];
	unsigned char cfg_netmask[4];
	unsigned char cfg_gatewayip[4];

	extern unsigned char DevMac[];
	extern dev_base_info_t	DeviceBaseInfo;
	
	/* Create tcp_ip stack thread */
	tcpip_init( NULL, NULL );	

	/* IP address setting */
	if(I2C_Read(cfg_ip, 4, EPROM_ADDR_IP, EEPROM_SLAVE_ADDR) != I2C_SUCCESS) {
		IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
		DeviceBaseInfo.IpAddress[0] = IP_ADDR0;
		DeviceBaseInfo.IpAddress[1] = IP_ADDR1;
		DeviceBaseInfo.IpAddress[2] = IP_ADDR2;
		DeviceBaseInfo.IpAddress[3] = IP_ADDR3;
	} else {	
		IP4_ADDR(&ipaddr, cfg_ip[0], cfg_ip[1], cfg_ip[2], cfg_ip[3]);
		if(check_ipaddress(cli_htonl(ipaddr.addr)) < 0) {
			IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
			DeviceBaseInfo.IpAddress[0] = IP_ADDR0;
			DeviceBaseInfo.IpAddress[1] = IP_ADDR1;
			DeviceBaseInfo.IpAddress[2] = IP_ADDR2;
			DeviceBaseInfo.IpAddress[3] = IP_ADDR3;			
		} else {
			DeviceBaseInfo.IpAddress[0] = cfg_ip[0];
			DeviceBaseInfo.IpAddress[1] = cfg_ip[1];
			DeviceBaseInfo.IpAddress[2] = cfg_ip[2];
			DeviceBaseInfo.IpAddress[3] = cfg_ip[3];
		}
	}

	if(I2C_Read(cfg_netmask, 4, EPROM_ADDR_NETMASK, EEPROM_SLAVE_ADDR) != I2C_SUCCESS)
		IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
	else {
		IP4_ADDR(&netmask, cfg_netmask[0], cfg_netmask[1], cfg_netmask[2], cfg_netmask[3]);
		if(check_netmask(cli_htonl(netmask.addr)) < 0) {
			IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
		}		
	}

	if(I2C_Read(cfg_gatewayip, 4, EPROM_ADDR_GATEWAY, EEPROM_SLAVE_ADDR) != I2C_SUCCESS)
		IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	else {
		IP4_ADDR(&gw, cfg_gatewayip[0], cfg_gatewayip[1], cfg_gatewayip[2], cfg_gatewayip[3]);
		if(check_ip_mask_gateway(cli_htonl(ipaddr.addr), cli_htonl(netmask.addr), cli_htonl(gw.addr)) < 0) {
			//printf("Check ip/netmask/gw error\r\n");
			IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
		}
	}

	printf("Ethernet Configuration ...\r\n");
	printf("   Physical Address : %02x-%02x-%02x-%02x-%02x-%02x\r\n", DevMac[0], DevMac[1], DevMac[2], DevMac[3], DevMac[4], DevMac[5]);
	printf("   IP Address       : %s\r\n", ip_ntoa(&ipaddr));
	printf("   Subnet Mask      : %s\r\n", ip_ntoa(&netmask));
	printf("   Default Gateway  : %s\r\n", ip_ntoa(&gw));

	netif_add(&xnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

	/*  Registers the default network interface. */
	netif_set_default(&xnetif);

	/*  When the netif is fully configured this function must be called.*/
	netif_set_up(&xnetif);
}

#endif

