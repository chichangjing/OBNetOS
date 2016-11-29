/*************************************************************
 * Filename     : nms_upgrade.c
 * Description  : API for NMS interface
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/

#include "mconfig.h"

#if MODULE_OBNMS
/* Standard includes. */
#include "stdio.h"
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* LwIP includes */
#include "lwip/inet.h"

/* BSP include */
#include "stm32f2xx.h"
#include "flash_if.h"

/* Other includes */
#include "nms_comm.h"
#include "nms_if.h"
#include "nms_sys.h"

#include "conf_comm.h"
#include "conf_sys.h"
#include "ob_image.h"


//#define NMS_UPGRADE_DEBUG

/* Private define ------------------------------------------------------------*/
#define FLASH_UPGRADE_START		ADDR_FLASH_SECTOR_8
#define FLASH_UPGRADE_END		ADDR_FLASH_SECTOR_11

#define ST_SRECODE_MAX_COUNT	125

#define ST_SRECODE_S0			0x00
#define ST_SRECODE_S3			0x03
#define ST_SRECODE_S5			0x05
#define ST_SRECODE_S7			0x07

#define SREC_DATA_LINE			(0)
#define SREC_BEGIN_LINE			(1)
#define SREC_END_LINE			(2)
#define SREC_ERR_TYPE			(-1)
#define SREC_ERR_COUNT			(-2)
#define SREC_ERR_CHECKSUM		(-3)

/* Private variables ---------------------------------------------------------*/
static u32 FlashWriteIndex=0;
extern u8 NMS_TxBuffer[];

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

int SREC_2_BIN(u8 *pInputBuf, u8 *pOutputBuf, u8 *datalen)
{
	int i,j;
	unsigned char srec_type;
	unsigned char srec_addr_len;
	unsigned char srec_count, date_count;
	int ret = 0;
	unsigned char check_count = 0;
	unsigned char check_sum = 0;

	/*record data*/
	if(*pInputBuf == ST_SRECODE_S3) {
		srec_type		= *pInputBuf & 0x0F;
		srec_addr_len	= srec_type + 1;
		srec_count	= *(pInputBuf + 1);

		if(srec_count > ST_SRECODE_MAX_COUNT) {
			#ifdef NMS_UPGRADE_DEBUG
			printf("Error: s-record count should fewer than 125 bytes\r\n");
			#endif
			ret = SREC_ERR_COUNT;
		}

		check_count = srec_count + 1;
		for(i=0; i<check_count; i++)
			check_sum += *(pInputBuf+1+i);

		date_count = srec_count - (srec_addr_len + 1);

		for (i=srec_addr_len+2, j=0; i<srec_addr_len+date_count+2; i++, j++)
			*(pOutputBuf+j) = *(pInputBuf+i);

		*datalen = date_count;

		/*check frame*/
		check_sum = ~(check_sum % 0x100);
		if(check_sum)
			ret = SREC_ERR_CHECKSUM;
		else
			ret = SREC_DATA_LINE;
	} else if (*pInputBuf == ST_SRECODE_S0) {	/*record data begin*/
		ret = SREC_BEGIN_LINE;
	} else if (*pInputBuf == ST_SRECODE_S7 || *pInputBuf == ST_SRECODE_S5) {	/*record data end*/
		ret = SREC_END_LINE;
	} else
		ret = SREC_ERR_TYPE;

	return ret;
}



void RspNMS_FirmwareUpgradeStart(u8 *DMA, u8 *RequestID)
{
	OBNET_SET_RSP RspData;
	u16 RspLength;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	RspData.GetCode = CODE_FIRMWARE_START;
	/***************************************************************/
	/* To add */
	FLASH_If_Init();
	if(FLASH_If_Erase(FLASH_UPGRADE_START, FLASH_UPGRADE_END) == 0) {
		RspData.RetCode = 0x00;
		RspData.Res = 0x00;
	} else {
		RspData.RetCode = 0x01;	/* Return error */
		RspData.Res = 0x01;		/* Error code */
	}
	/***************************************************************/
	
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspData, sizeof(OBNET_SET_RSP));
	RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);
}



void RspNMS_FirmwareUpgradeDoing(u8 *DMA, u8 *RequestID, u8 *DataBuffer)
{
	OBNET_SET_RSP RspData;
	u16 RspLength;
	int ret;
	u8	OutputBuf[ST_SRECODE_MAX_COUNT];
	u8	datalen;
	u16	currentReqID;
	static u16 previousReqID = 0;
	u32	FlashWriteAddress, retf;

	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	RspData.GetCode = CODE_FIRMWARE;
	/***************************************************************/
	/* To add */
	ret = SREC_2_BIN(DataBuffer, OutputBuf, &datalen);
	if(ret < 0) {
		RspData.RetCode = 0x01;
		if(ret == SREC_ERR_TYPE) {			/* s-record type unkown */
			RspData.Res = 0x01;	
		} else if(ret == SREC_ERR_COUNT) {	/* s-record count error */
			RspData.Res = 0x02;
		} else								/* s-record checksum error */
			RspData.Res = 0x03;
	} else {
		if(ret == SREC_BEGIN_LINE) {									/* Process Begin line of s-record data */
			#ifdef NMS_UPGRADE_DEBUG
			printf("Process the begin line of s-record data\r\n");
			#endif
			previousReqID = ntohs(*(u16 *)(RequestID));
			FlashWriteIndex = 0;
			RspData.RetCode = 0x00;
			RspData.Res = 0x00;	
		} else if(ret == SREC_END_LINE) {								/* Process End line of s-record data */
			#ifdef NMS_UPGRADE_DEBUG
			printf("Process the end line of s-record data, FlashWriteIndex=%d\r\n", FlashWriteIndex);
			#endif
			FlashWriteIndex = 0;
			RspData.RetCode = 0x00;
			RspData.Res = 0x00;			
		} else {														/* Process Data line of s-record data */
			currentReqID = ntohs(*(u16 *)(RequestID));
			if(currentReqID == previousReqID) {
				#ifdef NMS_UPGRADE_DEBUG
				printf("Error: Double frame is received\r\n");
				#endif
				RspData.RetCode = 0x01;
				RspData.Res = 0x0A;/* not define */
				return;
			} else if (currentReqID != previousReqID + 1) {
				#ifdef NMS_UPGRADE_DEBUG
				printf("Error: RequestID != previousReqID + 1\r\n");
				#endif
				RspData.RetCode = 0x01;
				RspData.Res = 0x0B;	/* not define */
			} else {
				FlashWriteAddress = FLASH_UPGRADE_START + FlashWriteIndex;
				retf = FLASH_If_Write(&FlashWriteAddress,(uint32_t *)(OutputBuf),datalen);
				if(retf) {					/* Flash write error */
					RspData.RetCode = 0x01;
					RspData.Res = 0x0e;	
				} else {					/* Flash_write success */
					FlashWriteIndex += datalen;
					previousReqID = currentReqID;
					RspData.RetCode = 0x00;
					RspData.Res = 0x00;	
				}
			}
		}
	}
	/***************************************************************/
	
	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspData, sizeof(OBNET_SET_RSP));
	RspSend(NMS_TxBuffer, RspLength+ + SWITCH_TAG_LEN);
}


void RspNMS_FirmwareUpgradeComplete(u8 *DMA, u8 *RequestID)
{
	OBNET_SET_RSP RspData;
	u16 RspLength;


	memset(NMS_TxBuffer, 0, MSG_MAXSIZE);

	RspLength = PAYLOAD_OFFSET + sizeof(OBNET_SET_RSP);
	if (RspLength < MSG_MINSIZE)
		RspLength = MSG_MINSIZE;
	
	/* fill the frame header */
	PrepareEtherHead(DMA);
	PrepareOBHead(MSG_RESPONSE, RspLength, RequestID);

	/* fill the response data */
	RspData.GetCode = CODE_FIRMWARE_COMPLETE;
	if(OB_Check_Upgrade_Image(FLASH_UPGRADE_START, NULL, NULL) == IHCHK_OK) {
		conf_set_upgrade_flag();
		RspData.RetCode = 0x00;
		RspData.Res = 0x00;		
	} else {
        conf_clear_upgrade_flag();
		RspData.RetCode = 0x01;
		RspData.Res = 0x00;
	}

	/* prepare the data to send */
	memcpy(&NMS_TxBuffer[PAYLOAD_OFFSET], (u8 *)&RspData, sizeof(OBNET_SET_RSP));
	RspSend(NMS_TxBuffer, RspLength + SWITCH_TAG_LEN);
}
#endif

