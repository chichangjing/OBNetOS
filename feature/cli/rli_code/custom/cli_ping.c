
/*************************************************************
 * Filename     : cli_ping.c
 * Description  : ping command interface
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/

#include <stdlib.h>
#include <string.h>

#include "cli_sys.h"
#include "cli_util.h"

#include "lwip/ip.h"
#include "lwip/sockets.h"
#include "lwip/tcpip.h"
#include "lwip/mem.h"
#include "lwip/ip_addr.h"   
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"
#include "lwip/sys.h"


#define DEFAULT_PING_DATA_LEN   32
#define DEFAULT_PING_COUNT      4
#define DEFAULT_PING_TIMEOUT    1000    /* 1000 ms */
#define DEFAULT_PING_ID         0x0300
#define ICMP_ECHO_HDR_SIZE		8
#define PING_REPLY_BUF_SIZE		96		/* > 32 + 8 + 20 */

typedef struct _ping_msg {
    uint8_t		type;		    /* Type = 8 */
    uint8_t		code;		    /* Code = 0 */
    uint16_t	checksum;	    /* Internet Checksum */
    uint16_t	id;			    /* Identification  */
    uint16_t	seq_num;        /* Sequence Number */
    uint8_t		data[DEFAULT_PING_DATA_LEN];
} ping_msg_t;

typedef struct _ping_log {
    uint16_t CheckSumErr;	    /* Check sum Error Count */
    uint16_t UnreachableMSG;    /* Count of receiving unreachable message from a peer */
    uint16_t TimeExceedMSG;	    /* Count of receiving time exceeded message from a peer */
    uint16_t UnknownMSG;	    /* Count of receiving unknown message from a peer */
    uint16_t ARPErr;		    /* count of fail to send ARP to the specified peer */
    uint16_t PingRequest;	    /* Count of sending ping-request message to the specified peer */
    uint16_t PingReply;	        /* Count of receiving ping reply message from the specifed peer */
    uint16_t Loss;		        /* Count of timeout  */
}ping_log_t;

typedef struct _ping_param {
	ubyte4	count;
	ubyte4	dst_ipaddr;
	cli_env	*vty;
}ping_param_t;

static ping_log_t pinglog;

int ping_process(ping_param_t *param)
{
    int icmp_sock;
	ubyte ping_rxbuf[PING_REPLY_BUF_SIZE];
    struct sockaddr_in pingto;
    sbyte buffer[64];
    ubyte4 count;
    uint8_t  is_received; 
    uint16_t random_seqnum;
	ping_msg_t *pmsg_ping_request;
	ping_msg_t *pmsg_ping_reply;
	int timeout;
    uint16_t i;
	portTickType start_time, end_time;
	
    /* Initialize the message of ping-request and ping-replay */
	if(!(pmsg_ping_request = (ping_msg_t *)pvPortMalloc(sizeof(ping_msg_t)))) {
		RCC_EXT_WriteStr(param->vty, "pvPortMalloc failed!\r\n");
		return -1;
	}
	
    random_seqnum = (uint16_t)rand();               	/* ping-request's sequence number to random integer value */
	pmsg_ping_request->type = 0x08;                 	/* ping-request - ICMP */
	pmsg_ping_request->code = 0x00;                		/* always 0 */
	pmsg_ping_request->checksum = 0;               		/* calculating checksum of ping-request packet */
	pmsg_ping_request->id = cli_htons(DEFAULT_PING_ID); /* ping-request ID */
	for(i = 0 ; i < DEFAULT_PING_DATA_LEN; i++)
		pmsg_ping_request->data[i] = 'a' + i % 23;

	/* Initialize the log of ping */
	memset((void *)&pinglog, 0, sizeof(ping_log_t));

    /* Create sock */
	if((icmp_sock = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0) {
		RCC_EXT_WriteStr(param->vty, "socket create failed!\r\n");
		vPortFree(pmsg_ping_request);
		return -1;
	}
	timeout = DEFAULT_PING_TIMEOUT;
	lwip_setsockopt(icmp_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	
    memset(&pingto, 0, sizeof(pingto));
	pingto.sin_len = sizeof(pingto);
	pingto.sin_family = AF_INET;
	pingto.sin_addr.s_addr = param->dst_ipaddr;  

    sprintf(buffer, "\r\nPinging %s with %d bytes of data:\r\n\r\n", inet_ntoa(pingto.sin_addr), (sizeof(ping_msg_t)-8));	
    RCC_EXT_WriteStr(param->vty, buffer);

    count = param->count;

	while(count-- != 0) {
		is_received = 0;
		pmsg_ping_request->seq_num = cli_htons(random_seqnum++);
		pmsg_ping_request->checksum = 0;
		pmsg_ping_request->checksum = inet_chksum(pmsg_ping_request, sizeof(ping_msg_t));

		pinglog.PingRequest++;

		start_time = xTaskGetTickCount();
		if(lwip_sendto(icmp_sock, pmsg_ping_request, sizeof(ping_msg_t), 0, (struct sockaddr *)&pingto, sizeof(struct sockaddr)) == sizeof(ping_msg_t)) {
			int rxlen, fromlen;
			struct sockaddr_in from;
			struct ip_hdr *iphdr;
			struct _ip_addr *addr;
			
			while((rxlen = lwip_recvfrom(icmp_sock, ping_rxbuf, sizeof(ping_rxbuf), 0, (struct sockaddr *)&from, (socklen_t *)&fromlen)) > 0) {

				if (rxlen >= (sizeof(struct ip_hdr) + ICMP_ECHO_HDR_SIZE)) {
					
					iphdr = (struct ip_hdr *)ping_rxbuf;
					pmsg_ping_reply = (ping_msg_t *)(ping_rxbuf + (IPH_HL(iphdr)*4));

					sprintf(buffer, "Reply from %s", inet_ntoa(from.sin_addr));	
					RCC_EXT_WriteStr(param->vty, buffer);
					
					if( inet_chksum(pmsg_ping_reply, sizeof(ping_msg_t)) != 0) {                           
							pinglog.CheckSumErr++; 
							is_received = 1;
							RCC_EXT_WriteStr(param->vty, ": Checksum Error");
					} else if(pmsg_ping_reply->type == 0) {
						if((pmsg_ping_reply->id != pmsg_ping_request->id) || (pmsg_ping_reply->seq_num != pmsg_ping_request->seq_num)) {
							RCC_EXT_WriteStr(param->vty, ": Unmatched ID/SeqNum");
							pinglog.UnknownMSG++;
						} else {		
							is_received = 1;
							sprintf(buffer, ": bytes=%d, time<=%dms TTL=%d", rxlen-IP_HLEN-8,(xTaskGetTickCount()-start_time)*portTICK_RATE_MS, IPH_TTL(iphdr));
							RCC_EXT_WriteStr(param->vty, buffer);
							pinglog.PingReply++;
						}
					} else if(pmsg_ping_reply->type == 3) {		/* If the packet is unreachable message */
						is_received = 1;
						RCC_EXT_WriteStr(param->vty, ": Destination unreachable");
						pinglog.UnreachableMSG++;
					} else if(pmsg_ping_reply->type == 11) {	/* If the packet is time exceeded message */		
						is_received = 1;
						RCC_EXT_WriteStr(param->vty, ": TTL expired in transit");
						pinglog.TimeExceedMSG++;
					} else {								/* if the packet is unknown message */		
						sprintf(buffer, ": Unknown message (type = 0x%02X)", pmsg_ping_reply->type);	
						RCC_EXT_WriteStr(param->vty, buffer);
						pinglog.UnknownMSG++;
					}
					
					RCC_EXT_WriteStr(param->vty, "\r\n");
					break;
				}
			}

			if(rxlen <= 0) {
				RCC_EXT_WriteStr(param->vty, "Request timed out.\r\n");
			}
		} else {
			RCC_EXT_WriteStr(param->vty, "ICMP echo request send error\r\n");
		}

		vTaskDelay(500);
	}

	RCC_EXT_WriteStr(param->vty, "\r\n");
	
	lwip_close(icmp_sock);
	vPortFree(pmsg_ping_request);
    
    return 0;
}



RLSTATUS cli_exec_ping_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal = NULL;
    paramDescr  *pParamDescr;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "host ip", mPing_Host_ip, &pParamDescr );
    if ( OK != status )
    {
		return(status);
    } else pVal = (sbyte*)(pParamDescr->pValue);
	

    /* TO DO: Add your handler code here */
    {
        ubyte4 dst_ipaddr;
		ping_param_t ping_param;
		
        CONVERT_StrTo(pVal, &dst_ipaddr, kDTipaddress);
		if(check_ipaddress((ubyte4)cli_htonl(dst_ipaddr)) < 0) {
			RCC_EXT_WriteStr( pCliEnv, "Error: Invalid ip address\r\n");
			return FALSE;
		}

		memset(&ping_param, 0, sizeof(ping_param_t));
		
		ping_param.dst_ipaddr = dst_ipaddr;
		ping_param.count = DEFAULT_PING_COUNT;
		ping_param.vty = pCliEnv;
		ping_process(&ping_param);
        
    }

	return status;
}



