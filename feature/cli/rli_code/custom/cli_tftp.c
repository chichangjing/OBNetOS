/*************************************************************
 * Filename     : cli_tftp.c
 * Description  : tftp command interface
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/

/* Standard includes */
#include <stdlib.h>
#include <string.h>

/* LwIP includes */
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"   
#include "lwip/inet.h"

/* BSP includes */
#include "flash_if.h"

/* Other includes */
#include "cli_sys.h"
#include "cli_util.h"

#include "conf_sys.h"
#include "ob_image.h"

/***************************************************************
	TFTP message formats are:
	Type          Op #     Format without header
	             2 bytes   string    1 byte   string   1 byte
	            ---------------------------------------------
	TFTP_RRQ/  | [01|02] | filename |  0    |  mode   |   0  |
	TFTP_WRQ    ---------------------------------------------
	             2 bytes   2 bytes   n bytes
	            -----------------------------
	TFTP_DATA  |   03    | block #  |  data  |
	            -----------------------------
	             2 bytes   2 bytes
	            -------------------
	TFTP_ACK   |   04    | block # |
	            -------------------
	             2 bytes   2 bytes     string  1 byte
	            -------------------------------------
	TFTP_ERROR |   05    | ErrorCode | ErrMsg |   0  |
	            -------------------------------------
 ***************************************************************/

#define TFTP_RRQ			1 	/* read request */
#define TFTP_WRQ			2	/* write request */
#define TFTP_DATA			3	/* data */
#define TFTP_ACK			4	/* ACK */
#define TFTP_ERROR			5	/* error */
#define TFTP_OACK			6	/* OACK */

#define TFTP_PORT			69
#define FLASH_WRITE_START	ADDR_FLASH_SECTOR_8
#define FLASH_WRITE_END		ADDR_FLASH_SECTOR_11
#define FLASH_SECTOR_SIZE	0x20000
#define TFTP_BUFFER_SIZE	516	/* 512 + 4 */

unsigned char tftp_buffer[TFTP_BUFFER_SIZE];

int tftp_get(cli_env *pCliEnv, sbyte *tftp_server, sbyte *filename)
{
	int ret, tftp_sock, timeout;
	struct sockaddr_in tftp_addr, from_addr;
	unsigned int length, totalsize;
	unsigned short rxblk_num, tftpblock, barlen;
	socklen_t fromlen;
	unsigned int write_address;
	unsigned int flash_sector_idx, flash_sector_erase_ready;
	unsigned int repeat, continue_flag;
	
	/* connect to tftp server */
    inet_aton(tftp_server, (struct in_addr *)&(tftp_addr.sin_addr));
    tftp_addr.sin_family = AF_INET;
    tftp_addr.sin_port = htons(TFTP_PORT);

	if((tftp_sock = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		cli_printf(pCliEnv, "socket create failed!\r\n");
		return -1;
	}
	timeout = 8000; /* 8 seconds */
	lwip_setsockopt(tftp_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	repeat = 3;
	while(repeat-- > 0) {
		cli_printf(pCliEnv, "Erase flash ... ");
		FLASH_If_Init();
		FLASH_If_Erase(FLASH_WRITE_START, FLASH_WRITE_END);
		cli_printf(pCliEnv, "done\r\n");
		
		/* TFTP request */
		tftp_buffer[0] = 0;			/* Opcode */
		tftp_buffer[1] = TFTP_RRQ; 	/* TFTP Request */
		length = sprintf((char *)&tftp_buffer[2], "%s", filename) + 2;
		tftp_buffer[length] = 0; length ++;
		length += sprintf((char*)&tftp_buffer[length], "%s", "binary");
		tftp_buffer[length] = 0; length ++;
		#if 1
		length += sprintf((char*)&tftp_buffer[length], "%s", "timeout");
		tftp_buffer[length] = 0; length ++;
		length += sprintf((char*)&tftp_buffer[length], "%d", 8);
		tftp_buffer[length] = 0; length ++;
		#endif
		
		fromlen = sizeof(struct sockaddr_in);


		//cli_printf(pCliEnv, "TFTP transfer start ...\r\n");
		/* send request */	
		lwip_sendto(tftp_sock, tftp_buffer, length, 0, (struct sockaddr *)&tftp_addr, sizeof(struct sockaddr_in));
		totalsize = 0;
		rxblk_num = 0;
		tftpblock = 0;
		barlen = 0;
		flash_sector_idx = 0;
		flash_sector_erase_ready = 0;
		do {
			continue_flag = 0;
			length = lwip_recvfrom(tftp_sock, tftp_buffer, sizeof(tftp_buffer), 0, (struct sockaddr *)&from_addr, &fromlen);
			if (length >= 4) {
				if((tftp_buffer[0] == 0) && (tftp_buffer[1] == TFTP_DATA)) {
					write_address = FLASH_WRITE_START + totalsize;
					totalsize = totalsize + length - 4;
					if(totalsize > FLASH_SECTOR_SIZE * (flash_sector_idx + 1)) {
						flash_sector_erase_ready = 0;
						flash_sector_idx++;
					}
						
					if(flash_sector_erase_ready == 0) {
						//cli_printf(pCliEnv, "Erase address 0x%08x\r\n", FLASH_WRITE_START + FLASH_SECTOR_SIZE*flash_sector_idx);
						//FLASH_If_Init();
						//FLASH_If_Erase(FLASH_WRITE_START + FLASH_SECTOR_SIZE*flash_sector_idx, FLASH_WRITE_START + FLASH_SECTOR_SIZE*(flash_sector_idx+1) - 1);
						flash_sector_erase_ready = 1;
					}
					FLASH_If_Write((uint32_t *)&write_address, (uint32_t *)&tftp_buffer[4], length-4);

					tftpblock = ntohs(*(unsigned short *)&tftp_buffer[2]);
					rxblk_num++; 
					if(rxblk_num >= 10) {	/* Receive 5k bytes, will print '#' */
						cli_printf(pCliEnv, "#");
						rxblk_num = 0;
						barlen++;
						if(barlen>=50) {	/* Max 50 '#' in a line */
							cli_printf(pCliEnv, "\r\n");
							barlen = 0;
						}
					}
					tftp_buffer[0] = 0; tftp_buffer[1] = TFTP_ACK; /* opcode */	
					tftp_buffer[2] = (unsigned char)((tftpblock >> 8) & 0xff);
					tftp_buffer[3] = (unsigned char)(tftpblock & 0xff);					
					lwip_sendto(tftp_sock, tftp_buffer, 4, 0, (struct sockaddr *)&from_addr, fromlen);
				} else if ((tftp_buffer[0] == 0) && (tftp_buffer[1] == TFTP_ERROR)) {
					unsigned short errcode;
					if(totalsize > 0)
						cli_printf(pCliEnv, "\r\n");
					cli_printf(pCliEnv, "TFTP error %d, %s.\r\n", (((errcode=tftp_buffer[2])<<8)|tftp_buffer[3]), &tftp_buffer[4]);
					goto exit;
				} else if ((tftp_buffer[0] == 0) && (tftp_buffer[1] == TFTP_OACK)) {
					tftp_buffer[0] = 0; tftp_buffer[1] = TFTP_ACK; /* opcode */	
					tftp_buffer[2] = (unsigned char)((tftpblock >> 8) & 0xff);
					tftp_buffer[3] = (unsigned char)(tftpblock & 0xff);
					lwip_sendto(tftp_sock, tftp_buffer, 4, 0, (struct sockaddr *)&from_addr, fromlen);
					continue_flag = 1;
				} else {
					if(totalsize > 0)
						cli_printf(pCliEnv, "\r\n");
					cli_printf(pCliEnv, "TFTP error %d, don't know why.\r\n", tftp_buffer[1]);
					goto exit;
				}
			}

		} while ((length == TFTP_BUFFER_SIZE) || (continue_flag == 1));

		if(length == 0) {
			cli_printf(pCliEnv, "TFTP request timeout.\r\n");
		} else {
			
			if((ret = OB_Check_Upgrade_Image(FLASH_WRITE_START, NULL, NULL)) == IHCHK_OK) {
				conf_set_upgrade_flag();
				cli_printf(pCliEnv, "\r\nReceived %d bytes and verify successfully\r\n", totalsize);
				break;
			} else {
                conf_clear_upgrade_flag();
				//cli_printf(pCliEnv, "\r\nError: Invalid image%s\r\n", (repeat>0)? ", repeat tftp":"");
				if(ret == IHCHK_ERR_MAGIC)
					cli_printf(pCliEnv, "\r\nError: Image magic check failed\r\n");
				else if(ret == IHCHK_ERR_HCRC)
					cli_printf(pCliEnv, "\r\nError: Image head crc check failed\r\n");
				else if(ret == IHCHK_ERR_DCRC)
					cli_printf(pCliEnv, "\r\nError: Image data crc check failed\r\n");	
				else if(ret == IHCHK_ERR_NAME)
					cli_printf(pCliEnv, "\r\nError: Image name check failed\r\n");
				else
					cli_printf(pCliEnv, "\r\nError: don't kown why!\r\n");
				
				goto exit;
			}
		}
	}
	
exit:	
	lwip_close(tftp_sock);
	
    return 0;
	
}

RLSTATUS cli_exec_tftp_handler(cli_env *pCliEnv, paramList *pParams, sbyte *pAuxBuf)
{
    RLSTATUS    status = OK;
    sbyte       *pVal1 = NULL;
    paramDescr  *pParamDescr1;
    sbyte       *pVal2 = NULL;
    paramDescr  *pParamDescr2;
    sbyte       *pVal3 = NULL;
    paramDescr  *pParamDescr3;

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "server-ip", mTftp_Server_ip, &pParamDescr1 );
    if ( OK != status )
    {
		return(status);
    } else pVal1 = (sbyte*)(pParamDescr1->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "get|put", mTftp_GetQppput, &pParamDescr2 );
    if ( OK != status )
    {
		return(status);
    } else pVal2 = (sbyte*)(pParamDescr2->pValue);

    /* get required parameter */
    status = RCC_DB_RetrieveParam(pParams, "filename", mTftp_Filename, &pParamDescr3 );
    if ( OK != status )
    {
		return(status);
    } else pVal3 = (sbyte*)(pParamDescr3->pValue);

    /* TO DO: Add your handler code here */

    {
        ubyte4 server_ip;
		sbyte pIp[16];
		
		status = CONVERT_StrTo(pVal1, &server_ip, kDTipaddress);
		if(check_ipaddress((ubyte4)cli_htonl(server_ip)) < 0) {
			RCC_EXT_WriteStr(pCliEnv, "Error: Invalid ip address\r\n");
			return FALSE;
		}
		status = CONVERT_ToStr(&server_ip, pIp, kDTipaddress); 

		tftp_get(pCliEnv, pVal1, pVal3);
	}


    return status;
}



