/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "mconfig.h"

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "netif/etharp.h"
#include "err.h"
#include "ethernetif.h"

#include "main.h"
#include "stm32f2x7_eth.h"
#include <string.h>

#if MODULE_RING
#include "ob_ring.h"
#endif
#if MODULE_OBNMS
#include "nms_if.h"
#endif

#define netifMTU                                (1500)
#define netifINTERFACE_TASK_STACK_SIZE		( 350 )
#define netifINTERFACE_TASK_PRIORITY		( configMAX_PRIORITIES - 1 )
#define netifGUARD_BLOCK_TIME			( 250 )
/* The time to block waiting for input. */
#define emacBLOCK_TIME_WAITING_FOR_INPUT	( ( portTickType ) 100 )

/* Define those to better describe your network interface. */
#define IFNAME0 's'
#define IFNAME1 't'


static struct netif *s_pxNetIf = NULL;
xSemaphoreHandle s_RxSemaphore = NULL;        
xSemaphoreHandle s_TxSemaphore = NULL;
	
/* Ethernet Rx & Tx DMA Descriptors */
extern ETH_DMADESCTypeDef  DMARxDscrTab[ETH_RXBUFNB], DMATxDscrTab[ETH_TXBUFNB];

/* Ethernet Receive buffers  */
extern uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE]; 

/* Ethernet Transmit buffers */
extern uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE]; 

/* Global pointers to track current transmit and receive descriptors */
extern ETH_DMADESCTypeDef  *DMATxDescToSet;
extern ETH_DMADESCTypeDef  *DMARxDescToGet;

/* Global pointer for last received frame infos */
extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;

static void ethernetif_input( void * pvParameters );
static void arp_timer(void *arg);

extern unsigned char DevMac[];
unsigned char MacAllOne[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
unsigned char MultiAddress[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };
#if CREDIT_NEIG_SEARCH_MULTI_ADDR
unsigned char NeigSearchMultiAddr[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };
#else
unsigned char NeigSearchMultiAddr[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x0E };
#endif
unsigned char OUI_Extended_EtherType[2] = { 0x88, 0xB7 };
unsigned char OBRING_PROTOCOL_ID[2] = { 0x52, 0x31 };				/* OB-Ring Protocol */
unsigned char OB_OrgCode[3] = {0x0c, 0xa4, 0x2a};					/* {0x0c, 0xa4, 0x2a} */
unsigned char EthRxBuffer[ETH_MAX_PACKET_SIZE+BRCM_HEADER_SIZE];

#if SWITCH_CHIP_88E6095
extern struct pbuf *m88e6095_rx(unsigned char *rxbuf, unsigned short len);
extern void m88e6095_tx(struct pbuf *p, unsigned char *dma_buf);
#elif SWITCH_CHIP_BCM53101
extern struct pbuf *bcm53101_rx(unsigned char *rxbuf, unsigned short len);
extern void bcm53101_tx(struct pbuf *p, unsigned char *dma_buf);
#elif SWITCH_CHIP_BCM53115
extern struct pbuf *bcm53115_rx(unsigned char *rxbuf, unsigned short len);
extern void bcm53115_tx(struct pbuf *p, unsigned char *dma_buf);
#elif SWITCH_CHIP_BCM53286
extern struct pbuf *bcm53286_rx(unsigned char *rxbuf, unsigned short len);
extern void bcm53286_tx(struct pbuf *p, unsigned char *dma_buf);
#elif SWITCH_CHIP_BCM5396
extern struct pbuf *bcm5396_rx(unsigned char *rxbuf, unsigned short len);
extern void bcm5396_tx(struct pbuf *p, unsigned char *dma_buf);
#endif

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif)
{
  uint32_t i;

  /* set netif MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;
	
  /* set netif MAC hardware address */
  memcpy(netif->hwaddr, DevMac, 6);
  
  /* set netif maximum transfer unit */
  netif->mtu = 1500;

  /* Accept broadcast address and ARP traffic */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
  /* igmp */
  netif->flags |= NETIF_FLAG_IGMP;
  
  s_pxNetIf =netif;

  /* create binary semaphore used for informing ethernetif of frame reception */
#if 1 
  if (s_RxSemaphore == NULL)
  {
    s_RxSemaphore= xSemaphoreCreateCounting(20,0);
  }
#else
  if (s_RxSemaphore == NULL)
  {
    vSemaphoreCreateBinary(s_RxSemaphore);
    xSemaphoreTake( s_RxSemaphore, 0);
  }
#endif

  /* initialize MAC address in ethernet MAC */ 
  ETH_MACAddressConfig(ETH_MAC_Address0, netif->hwaddr); 
  
  /* Initialize Tx Descriptors list: Chain Mode */
  ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
  /* Initialize Rx Descriptors list: Chain Mode  */
  ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);
  
  /* Enable Ethernet Rx interrrupt */
  { 
    for(i=0; i<ETH_RXBUFNB; i++)
    {
      ETH_DMARxDescReceiveITConfig(&DMARxDscrTab[i], ENABLE);
    }
  }

#ifdef CHECKSUM_BY_HARDWARE
  /* Enable the checksum insertion for the Tx frames */
  {
    for(i=0; i<ETH_TXBUFNB; i++)
    {
      ETH_DMATxDescChecksumInsertionConfig(&DMATxDscrTab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
    }
  } 
#endif

  /* create the task that handles the ETH_MAC */
  sys_thread_new("tEthRx", ethernetif_input, NULL, netifINTERFACE_TASK_STACK_SIZE, netifINTERFACE_TASK_PRIORITY);

  /* Enable MAC and DMA transmission and reception */
  ETH_Start();   
}


/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */


u8_t EthTxBuffer[ETH_MAX_PACKET_SIZE + BRCM_HEADER_SIZE];

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	struct pbuf *q;
	uint32_t l = 0;
	unsigned char *buffer;

	if (s_TxSemaphore == NULL) {
		vSemaphoreCreateBinary (s_TxSemaphore);
	}
	
	if (xSemaphoreTake(s_TxSemaphore, netifGUARD_BLOCK_TIME)) {
		buffer =  (unsigned char *)(DMATxDescToSet->Buffer1Addr);

#if 1
		l = 0;
		for(q = p; q != NULL; q = q->next) 
			l += q->len;

		if(l > ETH_MAX_PACKET_SIZE) {
			xSemaphoreGive(s_TxSemaphore);
			return ERR_BUF;
		} else {
		#if 0
			l = 0;
			for(q = p; q != NULL; q = q->next) {
				memcpy((u8_t*)&EthTxBuffer[l], q->payload, q->len);
				l = l + q->len;
			}

			if(l<60) l=60;

			memcpy(buffer, EthTxBuffer, 12);
			
#if SWITCH_CHIP_88E6095			
			buffer[12] = 0xC0;
			buffer[13] = 0x00;
			buffer[14] = 0x00;
			buffer[15] = 0x01;
#elif SWITCH_CHIP_BCM53101			
			buffer[12] = 0x00;
			buffer[13] = 0x00;
			buffer[14] = 0x00;
			buffer[15] = 0x00;		
#endif
			memcpy(&buffer[16], &EthTxBuffer[12], l-12);
			ETH_Prepare_Transmit_Descriptors(l+4);
		#else
#if SWITCH_CHIP_88E6095			
		m88e6095_tx(p, buffer);
		ETH_Prepare_Transmit_Descriptors(l+4);
#elif SWITCH_CHIP_BCM53101
		if(l<60) l=60;
		bcm53101_tx(p, buffer);
		ETH_Prepare_Transmit_Descriptors(l+4);		
#elif SWITCH_CHIP_BCM53115
		if(l<60) l=60;
		bcm53115_tx(p, buffer);
		ETH_Prepare_Transmit_Descriptors(l+4);	
#elif SWITCH_CHIP_BCM53286
		if(l<60) l=60;
		bcm53286_tx(p, buffer);
		ETH_Prepare_Transmit_Descriptors(l+8);
#elif SWITCH_CHIP_BCM5396
		if(l<64) l=64;
		bcm5396_tx(p, buffer);
		ETH_Prepare_Transmit_Descriptors(l+6+4);	/* 6 bytes BRCM Header + 4 bytes CRC32 */ 
#endif

		#endif
		}
#else
	for(q = p; q != NULL; q = q->next) {
      memcpy((u8_t*)&buffer[l], q->payload, q->len);
      l = l + q->len;
    }
    ETH_Prepare_Transmit_Descriptors(l);
#endif

		xSemaphoreGive(s_TxSemaphore);
	}

	return ERR_OK;
}


void EthSend(u8 *txBuffer, u16 len)
{
	u8 *buffer ;

	if (s_TxSemaphore == NULL) {
		vSemaphoreCreateBinary (s_TxSemaphore);
	}
	
	if (xSemaphoreTake(s_TxSemaphore, netifGUARD_BLOCK_TIME)) {
		buffer =  (u8 *)(DMATxDescToSet->Buffer1Addr);
		memcpy(buffer, txBuffer, len);
		ETH_Prepare_Transmit_Descriptors(len);
		xSemaphoreGive(s_TxSemaphore);
	}
}


static struct pbuf * rxFrameProcess(u8 *rxBuf, u16 len) 
{
	struct pbuf *p, *q;
	u32 l=0;
	
  	p = NULL;
	if((memcmp(rxBuf, MultiAddress, 6) == 0) && (memcmp(&rxBuf[16], OUI_Extended_EtherType, 2) == 0)) {
#if MODULE_RING		
		Ring_Frame_Process(rxBuf, len);
#endif
	} else if ((memcmp(rxBuf, MultiAddress, 6) != 0) && \
			  ((memcmp(&rxBuf[16], OUI_Extended_EtherType, 2) == 0) && \
			  ((memcmp(rxBuf, DevMac, 6) == 0) || (memcmp(rxBuf, MacAllOne, 6) == 0)))) {
#if MODULE_OBNMS
		NMS_Msg_Receive(rxBuf, len);
#endif
	} else {
		memcpy(&EthRxBuffer[0], rxBuf, 12);
		memcpy(&EthRxBuffer[12], &rxBuf[16], len-16);
		p = pbuf_alloc(PBUF_RAW, len-4, PBUF_POOL);
		if (p != NULL) { 
			for (q = p; q != NULL; q = q->next) {
				memcpy((u8_t*)q->payload, (u8_t*)&EthRxBuffer[l], q->len);
				l = l + q->len;
			} 
		}
	}

	return p;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */

static struct pbuf * low_level_input(struct netif *netif)
{
	struct pbuf *p, *q;
	u16_t len;
	uint32_t i=0, l=0;
	FrameTypeDef frame;
	u8 *buffer;
	__IO ETH_DMADESCTypeDef *DMARxNextDesc;

	p = NULL;

	/* Get received frame */
	frame = ETH_Get_Received_Frame_interrupt();

	/* check that frame has no error */
	if ((frame.descriptor->Status & ETH_DMARxDesc_ES) == (uint32_t)RESET) {
		/* Obtain the size of the packet and put it into the "len" variable. */
		len = frame.length;
		buffer = (u8 *)frame.buffer;

		#if 1
		if(len == 0)
			return NULL;
#if SWITCH_CHIP_88E6095
		p = m88e6095_rx(buffer, len);
#elif SWITCH_CHIP_BCM53101
		p = bcm53101_rx(buffer, len);
#elif SWITCH_CHIP_BCM53286
		p = bcm53286_rx(buffer, len);
#elif SWITCH_CHIP_BCM5396
		p = bcm5396_rx(buffer, len);
#elif SWITCH_CHIP_BCM53115
        p = bcm53115_rx(buffer, len);
#endif
		//p = rxFrameProcess(buffer, len);
		#else
	    /* We allocate a pbuf chain of pbufs from the pool. */
	    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
	    /* Copy received frame from ethernet driver buffer to stack buffer */
	    if (p != NULL)
	    { 
	      for (q = p; q != NULL; q = q->next)
	      {
	        memcpy((u8_t*)q->payload, (u8_t*)&buffer[l], q->len);
	        l = l + q->len;
	      } 
	    }
		#endif
	}

	/* Release descriptors to DMA */
	/* Check if received frame with multiple DMA buffer segments */
	if (DMA_RX_FRAME_infos->Seg_Count > 1) {
		DMARxNextDesc = DMA_RX_FRAME_infos->FS_Rx_Desc;
	} else {
		DMARxNextDesc = frame.descriptor;
	}

	/* Set Own bit in Rx descriptors: gives the buffers back to DMA */
	for (i=0; i<DMA_RX_FRAME_infos->Seg_Count; i++) {  
		DMARxNextDesc->Status = ETH_DMARxDesc_OWN;
		DMARxNextDesc = (ETH_DMADESCTypeDef *)(DMARxNextDesc->Buffer2NextDescAddr);
	}

	/* Clear Segment_Count */
	DMA_RX_FRAME_infos->Seg_Count = 0;

	/* When Rx Buffer unavailable flag is set: clear it and resume reception */
	if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET)  {
		/* Clear RBUS ETHERNET DMA flag */
		ETH->DMASR = ETH_DMASR_RBUS;

		/* Resume DMA reception */
		ETH->DMARPDR = 0;
	}
	
	return p;
}

/**
 * This function is the ethernetif_input task, it is processed when a packet 
 * is ready to be read from the interface. It uses the function low_level_input() 
 * that should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
void ethernetif_input( void * pvParameters )
{
	struct pbuf *p;

	for( ;; ) {
		if (xSemaphoreTake( s_RxSemaphore, emacBLOCK_TIME_WAITING_FOR_INPUT)==pdTRUE) {
TRY_GET_NEXT_FRAME:
			p = low_level_input( s_pxNetIf );
			if(p != NULL) {
				if (ERR_OK != s_pxNetIf->input( p, s_pxNetIf)) {
					pbuf_free(p);
				} else {
					goto TRY_GET_NEXT_FRAME;
				}
			}
			vTaskDelay(1);
			taskYIELD();
		}
	}
}  
      
/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;

  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  /* initialize the hardware */
  low_level_init(netif);
  
  etharp_init();
  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);

  return ERR_OK;
}


static void arp_timer(void *arg)
{
  etharp_tmr();
  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}
