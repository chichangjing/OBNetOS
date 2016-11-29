
/*******************************************************************
 * Filename     : hal_swif_txrx.c
 * Description  : Hardware Abstraction Layer for L2 Switch Tx/Rx
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *******************************************************************/
#include "mconfig.h"

/* Standard includes. */
#include <stdio.h>
#include <string.h>

/* LwIP includes */
#include "lwip/pbuf.h"
#include "lwip/err.h"

/* HAL for L2 includes */
#include "hal_swif_error.h"
#include "hal_swif_types.h"
#include "hal_swif_txrx.h"

#include "obring.h"
/* Other includes */
#include "nms_if.h"

extern uint8 EthTxBuffer[];
extern uint8 EthRxBuffer[];
extern uint8 MultiAddress[];
extern uint8 NeigSearchMultiAddr[];
extern uint8 RingMgmtMultiDA[];
extern uint8 OUI_Extended_EtherType[];
extern uint8 DevMac[];
extern uint8 MacAllOne[];
extern uint8 OBRING_PROTOCOL_ID[];

#if SWITCH_CHIP_BCM53286
struct pbuf *bcm53286_rx(uint8 *rxbuf, uint16 len)
{
	struct pbuf *p=NULL;
	struct pbuf *q;
	uint8 *pkt_data = (uint8 *)(rxbuf + BCM53286_BRCM_HDR_SIZE);
	uint16 l=0;

	if( (memcmp(pkt_data, RingMgmtMultiDA, 6) == 0) && \
		(memcmp(&pkt_data[12], OUI_Extended_EtherType, 2) == 0) && \
		(memcmp(&rxbuf[12+5], OBRING_PROTOCOL_ID, 2) == 0)) {
#if MODULE_RING		
		//Ring_Frame_Process(rxbuf, len);
#endif
	} else if ( (memcmp(&pkt_data[12], OUI_Extended_EtherType, 2) == 0) && \
			  	((memcmp(pkt_data, DevMac, 6) == 0) || (memcmp(pkt_data, MacAllOne, 6) == 0) || (memcmp(pkt_data, NeigSearchMultiAddr, 6) == 0))) {
#if MODULE_OBNMS
		NMS_Msg_Receive(rxbuf, len);
#endif
	} else {
		p = pbuf_alloc(PBUF_RAW, len-BCM53286_BRCM_HDR_SIZE, PBUF_POOL);
		if (p != NULL) { 
			for (q = p; q != NULL; q = q->next) {
				memcpy((u8_t*)q->payload, (u8_t*)&pkt_data[l], q->len);
				l = l + q->len;
			} 
		}
	}

	return p;
}


void bcm53286_tx(struct pbuf *p, uint8 *dma_buf)
{
	struct pbuf *q;
	uint8 *tx_buf=(uint8 *)&(EthTxBuffer[0]);
	uint8 *pkt_buf=(uint8 *)&(EthTxBuffer[BCM53286_BRCM_HDR_SIZE]);
	uint16 l=0;
	
	l = 0;
	for(q = p; q != NULL; q = q->next) {
		memcpy((u8_t*)&pkt_buf[l], q->payload, q->len);
		l = l + q->len;
	}
	
	tx_buf[0]=0xF0;
	tx_buf[1]=0x00;
	tx_buf[2]=0x00;
	tx_buf[3]=0x00;
	tx_buf[4]=0x00;
	tx_buf[5]=0x00;
	tx_buf[6]=0x00;
	tx_buf[7]=0x00;

	memcpy(dma_buf, tx_buf, l+BCM53286_BRCM_HDR_SIZE);

	return;
}
#endif

#if SWITCH_CHIP_BCM5396

static int _bcm5396_crc_table_created = 0;
static uint32 _bcm5396_crc_table[256];

uint32 bcm5396_swap32(uint32 u32data)
{
    u32data = (u32data << 16) | (u32data >> 16);
    return (u32data & 0xff00ffff) >> 8 | (u32data & 0xffff00ff) << 8;
}

uint32 bcm5396_crc32(uint32 crc, uint8 *data, uint32 len)
{
    uint32 i, j, accum;

    if (!_bcm5396_crc_table_created) {
	for (i = 0; i < 256; i++) {
	    accum = i;
	    for (j = 0; j < 8; j++) {
		if (accum & 1) {
		    accum = accum >> 1 ^ 0xedb88320UL;
		} else {
		    accum = accum >> 1;
		}
	    }
	    _bcm5396_crc_table[i] = bcm5396_swap32(accum);
	}
	_bcm5396_crc_table_created = 1;
    }

    for (i = 0; i < len; i++) {
	crc = crc << 8 ^ _bcm5396_crc_table[crc >> 24 ^ data[i]];
    }

    return crc;
}

void bcm5396_tagged_buf_add_crc32(uint8 *buffer, uint16 *len)
{
	uint32 i, crc32;
	uint8 tag_bak[BCM5396_BRCM_HDR_SIZE];
	uint8 *crc_buf;
	
	/* Backup Switch TAG */
	for(i=0; i<BCM5396_BRCM_HDR_SIZE; i++) {
		tag_bak[i] = buffer[12+i];
	}

	/* Untag Switch TAG */
	for(i=0; i<*len-12-BCM5396_BRCM_HDR_SIZE; i++) {
		buffer[12+i] = buffer[12+BCM5396_BRCM_HDR_SIZE+i];
	}
	*len -= BCM5396_BRCM_HDR_SIZE;

	/* Calculate CRC32 value */
    crc32 = ~ bcm5396_crc32(~0, &buffer[0], *len);

	/* Tag Switch TAG */
	for(i=0; i<*len-12; i++) {
		buffer[*len-1+BCM5396_BRCM_HDR_SIZE-i] = buffer[*len-1-i];
	}
	for(i=0; i<BCM5396_BRCM_HDR_SIZE; i++) {
		buffer[12+i] = tag_bak[i];
	}
	*len += BCM5396_BRCM_HDR_SIZE;

	/* Added CRC32 at the buffer end */
    crc_buf = &buffer[*len];
    *crc_buf++ = (uint8)(crc32 >> 24);
    *crc_buf++ = (uint8)(crc32 >> 16);
    *crc_buf++ = (uint8)(crc32 >> 8);
    *crc_buf++ = (uint8)(crc32);
	*len += 4;
}

struct pbuf *bcm5396_rx(uint8 *rxbuf, uint16 len)
{
	struct pbuf *p=NULL;
	struct pbuf *q;
	uint16 l=0;

	if( (memcmp(rxbuf, RingMgmtMultiDA, 6) == 0) && \
		(memcmp(&rxbuf[12+BCM5396_BRCM_HDR_SIZE], OUI_Extended_EtherType, 2) == 0) && \
		(memcmp(&rxbuf[12+BCM5396_BRCM_HDR_SIZE+5], OBRING_PROTOCOL_ID, 2) == 0)) {
#if MODULE_RING		
		//Ring_Frame_Process(rxbuf, len);
#endif
	} else if ( (memcmp(&rxbuf[12+BCM5396_BRCM_HDR_SIZE], OUI_Extended_EtherType, 2) == 0) && \
			  	((memcmp(rxbuf, DevMac, 6) == 0) || (memcmp(rxbuf, MacAllOne, 6) == 0) || (memcmp(rxbuf, NeigSearchMultiAddr, 6) == 0))) {
#if MODULE_OBNMS
		NMS_Msg_Receive(rxbuf, len);
#endif
	} else {
		memcpy(&EthRxBuffer[0], rxbuf, 12);
		memcpy(&EthRxBuffer[12], &rxbuf[12+BCM5396_BRCM_HDR_SIZE], len-12-BCM5396_BRCM_HDR_SIZE);
		p = pbuf_alloc(PBUF_RAW, len-BCM5396_BRCM_HDR_SIZE, PBUF_POOL);
		if (p != NULL) { 
			for (q = p; q != NULL; q = q->next) {
				memcpy((u8_t*)q->payload, (u8_t*)&EthRxBuffer[l], q->len);
				l = l + q->len;
			} 
		} else {
          //printf("Warning: pbuf alloc error!\r\n");
        }
	}

	return p;
}

void bcm5396_tx(struct pbuf *p, uint8 *dma_buf)
{
	struct pbuf *q;
	uint16 l, origLen;
	uint32 crc;
	uint8 *crcbuf;
	
	l = 0;
	for(q = p; q != NULL; q = q->next) {
		memcpy((u8_t*)&EthTxBuffer[l], q->payload, q->len);
		l = l + q->len;
	}

	if(l<64) {
		origLen = l;
		l=64;
		memset(&EthTxBuffer[l], 0, l-origLen);
	}
			
	memcpy(dma_buf, &EthTxBuffer[0], 12);
	dma_buf[12] = 0x88;
	dma_buf[13] = 0x74;
	dma_buf[14] = 0x00;
	dma_buf[15] = 0x00;	
	dma_buf[16] = 0x00;
	dma_buf[17] = 0x00;		
	memcpy(&dma_buf[12+BCM5396_BRCM_HDR_SIZE], &EthTxBuffer[12], l-12);

    crc = ~ bcm5396_crc32(~0, &EthTxBuffer[0], l);
    crcbuf = &dma_buf[l + BCM5396_BRCM_HDR_SIZE];
    *crcbuf++ = (uint8)(crc >> 24);
    *crcbuf++ = (uint8)(crc >> 16);
    *crcbuf++ = (uint8)(crc >> 8);
    *crcbuf++ = (uint8)(crc);

	return;
}
#endif


#if SWITCH_CHIP_BCM53101
struct pbuf *bcm53101_rx(uint8 *rxbuf, uint16 len)
{
	struct pbuf *p=NULL;
	struct pbuf *q;
	uint16 l=0;
    unsigned char ob_res[2] = {0x99, 0x0b};
	unsigned char ob_org_code[3] = {0x0c, 0xa4, 0x2a};

#if OBRING_DEV
		if( (memcmp(rxbuf, RingMgmtMultiDA, 6) == 0) && (len == MAX_RING_MSG_SIZE) && \
			(memcmp(&rxbuf[21+BCM53101_BRCM_HDR_SIZE], ob_org_code, 3) == 0) && \
			(memcmp(&rxbuf[26+BCM53101_BRCM_HDR_SIZE], ob_res, 2) == 0)) 
#else
	if( (memcmp(rxbuf, MultiAddress, 6) == 0) && \
		(memcmp(&rxbuf[12+BCM53101_BRCM_HDR_SIZE], OUI_Extended_EtherType, 2) == 0) && \
		(memcmp(&rxbuf[12+BCM53101_BRCM_HDR_SIZE+5], OBRING_PROTOCOL_ID, 2) == 0)) 
#endif /* OBRING_DEV */
	{
        
#if MODULE_RING
        //buffer_dump_console(rxbuf, len);
#if OBRING_DEV		
		obring_frame_receive(rxbuf, len);
#else
		Ring_Frame_Process(rxbuf, len);
#endif /* OBRING_DEV */
#endif /* MODULE_RING */
	} else if ( (memcmp(&rxbuf[16], OUI_Extended_EtherType, 2) == 0) && \
			  	((memcmp(rxbuf, DevMac, 6) == 0) || (memcmp(rxbuf, MacAllOne, 6) == 0) || (memcmp(rxbuf, NeigSearchMultiAddr, 6) == 0))) {
#if MODULE_OBNMS
		NMS_Msg_Receive(rxbuf, len);
#endif
	} else {	
        memcpy(&EthRxBuffer[0], rxbuf, 12);
        memcpy(&EthRxBuffer[12], &rxbuf[16], len-16);
        p = pbuf_alloc(PBUF_RAW, len-BCM53101_BRCM_HDR_SIZE, PBUF_POOL);
        if (p != NULL) { 
            for (q = p; q != NULL; q = q->next) {
                memcpy((u8_t*)q->payload, (u8_t*)&EthRxBuffer[l], q->len);
                l = l + q->len;
            } 
        }
    }
		
	return p;
}

void bcm53101_tx(struct pbuf *p, uint8 *dma_buf)
{
	struct pbuf *q;
	uint16 l=0;

	l = 0;
	for(q = p; q != NULL; q = q->next) {
		memcpy((u8_t*)&EthTxBuffer[l], q->payload, q->len);
		l = l + q->len;
	}

	memcpy(dma_buf, EthTxBuffer, 12);
	dma_buf[12] = 0x00;
	dma_buf[13] = 0x00;
	dma_buf[14] = 0x00;
	dma_buf[15] = 0x00;	
	memcpy(&dma_buf[12+BCM53101_BRCM_HDR_SIZE], &EthTxBuffer[12], l-12);

	return;
}
#endif


#if SWITCH_CHIP_BCM53115
struct pbuf *bcm53115_rx(uint8 *rxbuf, uint16 len)
{
	struct pbuf *p=NULL;
	struct pbuf *q;
	uint16 l=0;

	if( (memcmp(rxbuf, RingMgmtMultiDA, 6) == 0) && \
		(memcmp(&rxbuf[12+BCM53115_BRCM_HDR_SIZE], OUI_Extended_EtherType, 2) == 0) && \
		(memcmp(&rxbuf[12+BCM53115_BRCM_HDR_SIZE+5], OBRING_PROTOCOL_ID, 2) == 0)) {
#if MODULE_RING		
		Ring_Frame_Process(rxbuf, len);
#endif
	} else if ( (memcmp(&rxbuf[16], OUI_Extended_EtherType, 2) == 0) && \
			  	((memcmp(rxbuf, DevMac, 6) == 0) || (memcmp(rxbuf, MacAllOne, 6) == 0) || (memcmp(rxbuf, NeigSearchMultiAddr, 6) == 0))) {
#if MODULE_OBNMS
		NMS_Msg_Receive(rxbuf, len);
#endif
	} else {	
        memcpy(&EthRxBuffer[0], rxbuf, 12);
        memcpy(&EthRxBuffer[12], &rxbuf[16], len-16);
        p = pbuf_alloc(PBUF_RAW, len-BCM53115_BRCM_HDR_SIZE, PBUF_POOL);
        if (p != NULL) { 
            for (q = p; q != NULL; q = q->next) {
                memcpy((u8_t*)q->payload, (u8_t*)&EthRxBuffer[l], q->len);
                l = l + q->len;
            } 
        }
    }
		
	return p;
}

void bcm53115_tx(struct pbuf *p, uint8 *dma_buf)
{
	struct pbuf *q;
	uint16 l=0;

	l = 0;
	for(q = p; q != NULL; q = q->next) {
		memcpy((u8_t*)&EthTxBuffer[l], q->payload, q->len);
		l = l + q->len;
	}

	memcpy(dma_buf, EthTxBuffer, 12);
	dma_buf[12] = 0x00;
	dma_buf[13] = 0x00;
	dma_buf[14] = 0x00;
	dma_buf[15] = 0x00;	
	memcpy(&dma_buf[12+BCM53115_BRCM_HDR_SIZE], &EthTxBuffer[12], l-12);

	return;
}
#endif

#if SWITCH_CHIP_88E6095
struct pbuf *m88e6095_rx(uint8 *rxbuf, uint16 len)
{
	struct pbuf *p=NULL;
	struct pbuf *q;
	uint16 l=0;
	unsigned char ob_res[2] = {0x99, 0x0b};
	unsigned char ob_org_code[3] = {0x0c, 0xa4, 0x2a};

#if OBRING_DEV
		if( (memcmp(rxbuf, RingMgmtMultiDA, 6) == 0) && (len == MAX_RING_MSG_SIZE) && \
			(memcmp(&rxbuf[21+M88E6095_HDR_SIZE], ob_org_code, 3) == 0) && \
			(memcmp(&rxbuf[26+M88E6095_HDR_SIZE], ob_res, 2) == 0)) 
#else
	if( (memcmp(rxbuf, MultiAddress, 6) == 0) && \
		(memcmp(&rxbuf[12+M88E6095_HDR_SIZE], OUI_Extended_EtherType, 2) == 0) && \
		(memcmp(&rxbuf[12+M88E6095_HDR_SIZE+5], OBRING_PROTOCOL_ID, 2) == 0)) 
#endif
	{
#if MODULE_RING	
#if OBRING_DEV		
		obring_frame_receive(rxbuf, len);
#else
		Ring_Frame_Process(rxbuf, len);
#endif /* OBRING_DEV */
#endif /* MODULE_RING */
	} else if ( (memcmp(&rxbuf[16], OUI_Extended_EtherType, 2) == 0) && \
			  	((memcmp(rxbuf, DevMac, 6) == 0) || (memcmp(rxbuf, MacAllOne, 6) == 0) || (memcmp(rxbuf, NeigSearchMultiAddr, 6) == 0))) {
#if MODULE_OBNMS
		NMS_Msg_Receive(rxbuf, len);
#endif
	} else {
		memcpy(&EthRxBuffer[0], rxbuf, 12);
		memcpy(&EthRxBuffer[12], &rxbuf[16], len-16);
		p = pbuf_alloc(PBUF_RAW, len-M88E6095_HDR_SIZE, PBUF_POOL);
		if (p != NULL) { 
			for (q = p; q != NULL; q = q->next) {
				memcpy((u8_t*)q->payload, (u8_t*)&EthRxBuffer[l], q->len);
				l = l + q->len;
			} 
		}
	}

	return p;
}

void m88e6095_tx(struct pbuf *p, uint8 *dma_buf)
{
	struct pbuf *q;
	uint16 l=0;

	l = 0;
	for(q = p; q != NULL; q = q->next) {
		memcpy((u8_t*)&EthTxBuffer[l], q->payload, q->len);
		l = l + q->len;
	}

	memcpy(dma_buf, EthTxBuffer, 12);
	dma_buf[12] = 0xC0;
	dma_buf[13] = 0x00;
	dma_buf[14] = 0x00;
	dma_buf[15] = 0x01;	
	memcpy(&dma_buf[12+M88E6095_HDR_SIZE], &EthTxBuffer[12], l-12);

	return;
}
#endif

