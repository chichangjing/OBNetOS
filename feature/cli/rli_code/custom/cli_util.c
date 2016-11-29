
/*************************************************************
 * Filename     : cli_util.c
 * Description  : Utility interface of CLI
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/
 
/* Standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "os_mutex.h"

#include "cli_util.h"

unsigned int gDbgModule[kRCC_MAX_CLI_TASK+1] = {0};
OS_MUTEX_T stCliDbgMutex;

/* host byte order convert to network byte order. */
unsigned short cli_htons(unsigned short n)
{
  return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

/* network byte order convert to host byte order. */
unsigned short cli_ntohs(unsigned short n)
{
  return cli_htons(n);
}

/* host byte order convert to network byte order. */
unsigned long cli_htonl(unsigned long n)
{
  return ((n & 0xff) << 24) | ((n & 0xff00) << 8) | ((n & 0xff0000UL) >> 8) | ((n & 0xff000000UL) >> 24);
}

/* network byte order convert to host byte order. */
unsigned long cli_ntohl(unsigned long n)
{
  return cli_htonl(n);
}


void cli_extract_mac(unsigned char *macAddr, char *macStr)
{
	char *s = macStr;   
	char *e;
	int i;
	
	for (i=0; i<6; ++i) {
		strtoul(s, &e, 16);
		if((unsigned int)e - (unsigned int)s > 2) {
			char tmp[3];
			memcpy(tmp, s, 2);
			tmp[2] =0;
			macAddr[i] = s ? strtoul(tmp, NULL, 16) : 0;
			s += 2;
		}
		else {
			macAddr[i] = s ? strtoul(s, &e, 16) : 0;
			if (s) s = (*e) ? e+1 : e;
		}
	}
}


int check_mac(unsigned char *mac_addr)
{
	const unsigned char mac_all_00[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	const unsigned char mac_all_ff[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	if((memcmp(mac_addr, mac_all_00, 6) == 0) || (memcmp(mac_addr, mac_all_ff, 6) == 0))
		return CHK_ERR_INVALID_MAC_ADDR;

	return CHK_OK;
}

/*****************************************************************************
 * function    : check_ip_mask
 * description : It is responsible for IP address sematics checking. The IP
 *               address error conditions are following:
 *                 1. All 0s
 *                 2. All 1s
 *                 3. Class D
 *                 4. Class E
 *                 5. Class A, B, C with net id 0
 *                 6. Class A, B, C with net id 1
 * parameter   :
 * return      : 0 = OK
 *              -1 = Error
 *****************************************************************************/
int check_ipaddress(unsigned int ip_address)
{
	int ret_val;
	unsigned int ip_class ;                            
	                        
	if(ip_address == ALL_ONE_IP) {
	    return CHK_ERR_INVALID_IPADDR;
	}
	     
	ret_val = CHK_OK;                            
	ip_class = ip_address >> 30;

	switch( ip_class ) { 
	    case IP_CLASS_A_0:
	        if(ip_address == 0)
	            return CHK_OK;

	    case IP_CLASS_A_1:
	        if(((ip_address & IP_CLASS_A_NET) == IP_CLASS_A_NET) ||
	           ((ip_address & IP_CLASS_A_NET) == ALL_ZERO_IP) ||
	           ((ip_address & IP_CLASS_A_HOST) == IP_CLASS_A_HOST) ||
	           ((ip_address & IP_CLASS_A_HOST) == ALL_ZERO_IP)) {
	            ret_val = CHK_ERR_INVALID_IPADDR;
	        }
	        break ;
	    case IP_CLASS_B:     
	        if(((ip_address & IP_CLASS_B_NET) == IP_CLASS_B_NET)||
	           ((ip_address & IP_CLASS_B_NET) == ALL_ZERO_IP) ||
	           ((ip_address & IP_CLASS_B_HOST) == IP_CLASS_B_HOST) ||
	           ((ip_address & IP_CLASS_B_HOST) == ALL_ZERO_IP)) {
	            ret_val = CHK_ERR_INVALID_IPADDR;
	        }
	        break ;      
	    case IP_CLASS_C:
	        ip_class = ip_address >> 28;
	        
	        switch( ip_class )
	        {
	            case IP_CLASS_D: /* Class D, and E not allowed */
	            case IP_CLASS_E:
	               ret_val = CHK_ERR_INVALID_IPADDR;
	               break ;

	            default: /* real IP_CLASS_C */
	               if(((ip_address & IP_CLASS_C_NET) == IP_CLASS_C_NET)||
	                  ((ip_address & IP_CLASS_C_NET) == ALL_ZERO_IP) ||
	                  ((ip_address & IP_CLASS_C_HOST) == IP_CLASS_C_HOST) ||
	                  ((ip_address & IP_CLASS_C_HOST) == ALL_ZERO_IP)) {
	                    ret_val = CHK_ERR_INVALID_IPADDR;
	                }
	               break ;        
	        } /* end of switch ip_class in case IP_CLASS_C */                     
	     break ;
	     
	  default:
	     ret_val = CHK_ERR_INVALID_IPADDR;
	     break;
	} /* end of switch */         

	return ret_val;
}


/*****************************************************************************
 * function    : check_ip_mask
 * description : check netmask
 * parameter   :
 * return      : 0 = OK
 *              -1 = Error
 *****************************************************************************/
int check_netmask(unsigned int netmask)
{
	unsigned int temp_netmask;

	/* subnet mask all 0s and 1s checking.*/
	if( netmask == ALL_ZERO_IP || netmask == ALL_ONE_IP) {
		return CHK_ERR_INVALID_NETMASK;
	}
	
	/* subnet mask contiqous 1s checking */
	temp_netmask = netmask ;    
	while(temp_netmask & 0x80000000)
		temp_netmask = temp_netmask << 1;

	if( temp_netmask & ALL_ONE_IP ) {
		return CHK_ERR_INVALID_NETMASK;
	}
	
	return CHK_OK;
}


/*****************************************************************************
 * function    : check_ip_mask
 * description : check the relationship ip and mask
 * parameter   :
 * return      : 0 = OK
 *              -1 = Error
 *****************************************************************************/
int check_ip_mask(unsigned int ip, unsigned int netmask)
{
    int ret = CHK_OK;
    
#if 0
    /* check ip */   
    if(ip == 0 || check_ipaddress(ip) < 0)
        return -1;
#endif

    ret = check_netmask(netmask);
    if (ret != CHK_OK)
		return ret;

    /* check relationship */
	if ((ip & ( ~netmask ) | netmask) == ALL_ONE_IP)
	    ret = CHK_ERR_IP_BROADCAST;
	    
	if ((ip & netmask ) == ALL_ZERO_IP )
	    ret = CHK_ERR_IP_SUBNET_ZERO;
	 
    if ((ip & ( ~netmask ))== ALL_ZERO_IP)
        ret = CHK_ERR_IP_IS_SUBNET;
	
    return ret;

}

/*****************************************************************************
 * function    : check_ip_mask_gateway
 * description : check the relationship ip, mask and gateway
 * parameter   :
 * return      : 0 = OK
 *              -1 = Error
 *****************************************************************************/
int check_ip_mask_gateway(unsigned int ip, unsigned int mask, unsigned int gateway)
{
	if (check_ip_mask(ip, mask) < 0)
		goto _fail;

	if (check_ipaddress(gateway) < 0)
		goto _fail;

	if (gateway != ALL_ZERO_IP) {
		if ((ip & mask) != (mask & gateway))
			goto _fail;

		if ((gateway & ( ~mask ) | mask) == ALL_ONE_IP)
			goto _fail;

		if ((gateway & ( ~mask ) == ALL_ZERO_IP))
			goto _fail;
	}
	
	return CHK_OK;

_fail:
	return CHK_ERR_INVALID_IP_MASK_GW;
}

void cli_port_vec2list(unsigned int vec, char *port_string)
{
	
}

void cli_debug_init(void)
{
	os_mutex_init(&stCliDbgMutex);
}


void cli_printf(cli_env *pCliEnv, const char *fmt, ...)
{
    //char *buf = &OutputBuffer[0];
    char buf[128];
    va_list args;

    if (pCliEnv==NULL)
        return;

	buf[0] = '\0';
	
    va_start(args, fmt);
    vsprintf(buf, fmt, args); 
    //va_end(args);

    if (kRCC_CONN_CONSOLE == MCONN_GetConnType(pCliEnv)) {
		printf("%s", buf);
	} else {
		RCC_EnableFeature(pCliEnv, kRCC_FLAG_RAW);
		RCC_EXT_WriteStr(pCliEnv, buf);
		RCC_DisableFeature(pCliEnv, kRCC_FLAG_RAW);
	}

	
    return;
}

void cli_debug(int module, char* fmt, ...)
{
	int index;
	cli_env *pCliEnv;
    //char *buf = &OutputBuffer[0];
    int count;
    char buf[128];
    va_list args;

    for(index=0; index<kRCC_MAX_CLI_TASK+1; index++) {
		if((gDbgModule[index] & module) == 0)
			continue;
		
        if(NULL == (pCliEnv = RCC_TELNETD_GetSession(index)))
            continue;

        if(NULL == CLIENV(pCliEnv))
            continue;

		if(fmt == NULL)
			return;

		buf[0] = '\0';
		
	    va_start(args, fmt);
	    vsprintf(buf, fmt, args); 
	    //va_end(args);

	    if (kRCC_CONN_CONSOLE == MCONN_GetConnType(pCliEnv)) {
			printf("%s", buf);
		} else {
			RCC_EnableFeature(pCliEnv, kRCC_FLAG_RAW);
			RCC_EXT_WriteStr(pCliEnv, buf);
			RCC_DisableFeature(pCliEnv, kRCC_FLAG_RAW);
			//RCC_DisableFeature(pCliEnv, kRCC_FLAG_ECHO);
			//RCC_EnableFeature(pCliEnv, kRCC_FLAG_INPUT);
		}
    }

	return;
}

void cli_dump(int module, unsigned char *buf, int len)
{
	unsigned int i, nbytes, linebytes;
	unsigned char *cp=buf;

	nbytes = len;
	do {
		unsigned char	linebuf[16];
		unsigned char	*ucp = linebuf;
		cli_debug(module, "     ");
		linebytes = (nbytes > 16)?16:nbytes;
		for (i=0; i<linebytes; i+= 1) {
			cli_debug(module, "%02X ", (*ucp++ = *cp));
			cp += 1;
		}
		cli_debug(module, "\r\n");
		nbytes -= linebytes;

	} while (nbytes > 0);
}

void debug_module_clear(void)
{
	//memset(gDbgModule, 0, kRCC_MAX_CLI_TASK+1);
	gDbgModule[0] = 0;
	gDbgModule[1] = 0;
}

int	cli_debug_module_add(cli_env *pCliEnv, unsigned int module)
{
    int index;
    cli_env *pOther;
    ubyte4   ipAddr;
    sbyte    buffer[16];

    for(index=0; index<kRCC_MAX_CLI_TASK+1; index++) {
        if(NULL == (pOther = RCC_TELNETD_GetSession(index)))
            continue;

        if(NULL == CLIENV(pOther))
            continue;
		
		if(pCliEnv == pOther) {
			gDbgModule[index] |= module;
			return 0;
		}
    }

	ipAddr = OS_SPECIFIC_GET_ADDR(pCliEnv);
	CONVERT_ToStr(&ipAddr, buffer, kDTipaddress);
	cli_printf(pCliEnv, "Error: writing module to CLI session %s\r\n", buffer);

    return -1;
}

unsigned int cli_debug_module_del(cli_env *pCliEnv, unsigned int module)
{
    int index;
    cli_env *pOther;
    ubyte4   ipAddr;
    sbyte    buffer[16];

    for(index=0; index<kRCC_MAX_CLI_TASK+1; index++) {
        if(NULL == (pOther = RCC_TELNETD_GetSession(index)))
            continue;

        if(NULL == CLIENV(pOther))
            continue;
		
		if(pCliEnv == pOther) {
			gDbgModule[index] &= ~module;
			return 0;
		}
    }

	ipAddr = OS_SPECIFIC_GET_ADDR(pCliEnv);
	CONVERT_ToStr(&ipAddr, buffer, kDTipaddress);
	cli_printf(pCliEnv, "Error: writing module to CLI session %s\r\n", buffer);

    return -1;
}

unsigned int cli_debug_module_read(cli_env *pCliEnv)
{
    int index;
	unsigned int module = 0;
    cli_env *pOther;
    ubyte4   ipAddr;
    sbyte    buffer[16];

    if (pCliEnv==NULL)
        return gDbgModule[0];
	
    for(index=0; index<kRCC_MAX_CLI_TASK+1; index++) {
        if(NULL == (pOther = RCC_TELNETD_GetSession(index)))
            continue;

        if(NULL == CLIENV(pOther))
            continue;
		
		if(pCliEnv == pOther) {
			module = gDbgModule[index];
			return module;
		}
    }

	ipAddr = OS_SPECIFIC_GET_ADDR(pCliEnv);
	CONVERT_ToStr(&ipAddr, buffer, kDTipaddress);
	cli_printf(pCliEnv, "Error: reading module from CLI session %s\r\n", buffer);


    return module;
}

