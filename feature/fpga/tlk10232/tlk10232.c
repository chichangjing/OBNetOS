

#include "tlk10232.h"
#include "halsw_mdio.h"
#include "hal_io.h"
#include "delay.h"

#if (defined PHY_TEST)
#include "i2c_ee.h"
#include "common.h"
#include "halsw_i2c.h"
#endif

void TlkValueWrite(unsigned int phyadd, unsigned int devadd, unsigned int  regadd, unsigned int value)
{
	MdioCL45WriteA(phyadd, devadd, regadd);
	MdioCL45WriteV(phyadd, devadd, value);
}

unsigned short int TlkValueRead(unsigned int phyadd, unsigned int devadd, unsigned int  regadd)
{
	unsigned short int ret;
	MdioCL45WriteA(phyadd, devadd, regadd);
	ret = MdioCL45Read(phyadd, devadd);
	return ret;
}

unsigned short int TlkValueIncRead(unsigned int phyadd, unsigned int devadd, unsigned int  regadd)
{
	unsigned short int ret;
	MdioCL45WriteA(phyadd, devadd, regadd);
	ret = MdioCL45IncRead(phyadd, devadd);
	return ret;
}




void TlkSwReset(unsigned int phyadd)
{
	/* software reset GBL_CTL1[15] = 1 */
	TlkValueWrite(phyadd, VSPDRADDR , GBL_CTL1, 0x8000);
}

#if 0
void TlkHwReset(void)
{
	TLK10232RSTCTL(0);
	Delay_Ms(10);
	TLK10232RSTCTL(1);
}
#endif


void MannualModeConf(unsigned char chan, unsigned char isauto, void (*pFunction)(void))
{
	unsigned int val;
#if (defined PHY_TEST)
	unsigned short int reg[3], regaddr;
#endif
	unsigned int blinkto = 50;
	if (!isauto)
	{
		TlkValueWrite(chan, 0x07, 0x0000, 0x2000);    //disable Auto negotiation
	}
	else
	{
		TlkValueWrite(chan, 0x07, 0x0000, 0x3000);    //Enable Auto negotiation
	}
	val = (chan == PHYCH2) ?  0x0b03 : 0x0b00;
	TlkValueWrite(chan, 0x1e, 0x0001, val);    //
	/* hs rx clk en */
//	val = (chan == PHYCH2) ?  0x2f88 : 0x2f80;
//	TlkValueWrite(chan, 0x1e, 0x000d, val);
	TlkValueWrite(chan, 0x01, 0x0096, 0x0000);	//disable Link Training
	TlkValueWrite(chan, 0x1e, 0x8020, 0x03ff);	//reserved register settings
#if !(defined PHY_TEST)
	/*****************S8002 AND S8001**************/
//	TlkValueWrite(chan, 0x1e, 0x04, 0xd500);      //5500   //tlk10232 Link Training Optimization
	TlkValueWrite(chan, 0x1e, 0x04, 0x5500);      //5500   //tlk10232 Link Training Optimization
//	TlkValueWrite(chan, 0x1e, 0x03, 0xb848);      //7848  660mv  HS_SWING [3:0]
	TlkValueWrite(chan, 0x1e, 0x03, 0x7848);      //7848  660mv  HS_SWING [3:0]
	TlkValueWrite(chan, 0x1e, 0x05, 0x2200);      //
//	TlkValueWrite(chan, 0x1e, 0x05, 0x2000);      //
	/************************40km******************************

	TlkValueWrite(0x0000, 0x1e, 0x04, 0x6500);      //d500   //tlk10232 Link Training Optimization
	TlkValueWrite(0x0000, 0x1e, 0x03, 0x6848);      //660mv  HS_SWING [3:0]
	TlkValueWrite(0x0000, 0x1e, 0x05, 0x2202);      //
	*********************************************************/
	//Write 30.3.15:12 HS_SWING [3:0]
	//Write 30.3.10 HS_EQHLD
	//Write 30.3.5:4 HS_AZCAL [1:0]
	//Write 30.3.2:0 HS_RATE_TX [2:0]
	//Write 30.4.15 HS_ENTRACK
	//Write 30.4.4:0 HS_TWCRF [4:0]
	//Write 30.5.12:8 HS_TWPOST1 [4:0]
	//Write 30.5.7:4 HS_TWPRE [3:0]
	//Write 30.5.3:0 HS_TWPOST2 [3:0]
	/* issue data path reset */
#else
	regaddr = (chan == PHYCHAN1) ? PHYREGADDR1 : PHYREGADDR2;
	At24_Sequential_Read(I2CDEV_0, E2DEVADD0, regaddr, (unsigned char *)&reg, sizeof(reg));
	reg[0] = SWP16(reg[0]);
	reg[1] = SWP16(reg[1]);
	reg[2] = SWP16(reg[2]);
	TlkValueWrite(chan, 0x1e, 0x03, reg[0]);
	TlkValueWrite(chan, 0x1e, 0x04, reg[1]);
	TlkValueWrite(chan, 0x1e, 0x05, reg[2]);
#endif
	val = TlkValueRead(chan, 0x1e, 0x000e);
	val |= 0x0008;
	TlkValueWrite(chan, 0x1e, 0x000e, val);
	Delay_Ms(1000);
	/* status read ,clear latch */
//	do{
	val = TlkValueRead(chan, 0x1e, 0x0f);
	val = TlkValueRead(chan, 0x1e, 0x0f);
	BlinkRunLed(blinkto);
	if (pFunction)
	{
		pFunction();
	}
//	}while((val & 0x1803) != 0x1803);
}

unsigned int chnstatus1, HS_status1, HS_status2;
unsigned int hserrcnt;
unsigned int lslnerrcnt[4];
unsigned int lanestatus[4];
unsigned int pmastatus[2];
unsigned int pcsstatus[2];
unsigned int  AN_BP_STATUS[30];


void Read_status(int chan)
{
	unsigned int val;
	unsigned int i;
	chnstatus1 = TlkValueRead(chan, 0x1e, 0x000f);    //CHANNEL_STATUS_1 to clear (16¡¯h5C03)
	/*
	sprintf(UART2_TX_BUF, "\nCHANNEL_STATUS_1       is: 0x%04x \n", chnstatus1);
	val = strlen(UART2_TX_BUF);
	USART2_Send(val);*/
	hserrcnt = TlkValueRead(chan, 0x1e, 0x0010);      //HS_ERROR_COUNTER to clear (16¡¯h0000)
	/*
	sprintf(UART2_TX_BUF, "HS_ERROR_COUNTER       is: 0x%04x \n", hserrcnt);
	val = strlen(UART2_TX_BUF);
	USART2_Send(val);*/
	lslnerrcnt[0] = TlkValueRead(chan, 0x1e, 0x0011); //LS_LN0_ERROR_COUNTER to clear (16¡¯h0000)
	lslnerrcnt[1] = TlkValueRead(chan, 0x1e, 0x0012); // LS_LN1_ERROR_COUNTER to clear (16¡¯h0000)
	lslnerrcnt[2] = TlkValueRead(chan, 0x1e, 0x0013); // LS_LN2_ERROR_COUNTER to clear (16¡¯h0000)
	lslnerrcnt[3] = TlkValueRead(chan, 0x1e, 0x0014); // LS_LN3_ERROR_COUNTER to clear (16¡¯h0000)
	/*
	sprintf(UART2_TX_BUF, "LS_LN0-3_ERROR_COUNTER is: 0x%04x,0x%04x,0x%04x,0x%04x \n", lslnerrcnt[0], lslnerrcnt[1], lslnerrcnt[2], lslnerrcnt[3]);
	val = strlen(UART2_TX_BUF);
	USART2_Send(val);*/
	/* lane */
	val =  TlkValueRead(chan, 0x1e, 0x000c);
	for (i = 0; i < 4; i++)                             //LS status registers for each lane to clear
	{
		val |= (i << 12);
		TlkValueWrite(chan, 0x1e, 0x000c, val);
		lanestatus[i] = TlkValueRead(chan, 0x1e, 0x0015);
	}
	/*
	sprintf(UART2_TX_BUF, "Lane0-3 Status         is: 0x%04x,0x%04x,0x%04x,0x%04x \n", lanestatus[0], lanestatus[1], lanestatus[2], lanestatus[3]);
	val = strlen(UART2_TX_BUF);
	USART2_Send(val);*/
	pmastatus[0] = TlkValueRead(chan, 0x01, 0x0001);    //PMA_STATUS_1 to clear
	pmastatus[1] = TlkValueRead(chan, 0x01, 0x0008);    //PMA_STATUS_2 to clear
	pcsstatus[0] = TlkValueRead(chan, 0x03, 0x0001);    //PCS_STATUS_1 to clear
	pcsstatus[1] = TlkValueRead(chan, 0x03, 0x0008);    //PCS_STATUS_2 to clear
	/*
	sprintf(UART2_TX_BUF, "PMA_STATUS_1,PMA_STATUS_2,PCS_STATUS_1,PCS_STATUS_2 is: 0x%04x,0x%04x,0x%04x,0x%04x \n", pmastatus[0], pmastatus[1], pcsstatus[0], pcsstatus[1]);
	val = strlen(UART2_TX_BUF);
	USART2_Send(val);*/
	/*
	    AN_BP_STATUS[0] = TlkValueRead(0x0000, 0x07, 0x0000);
	    AN_BP_STATUS[1] = TlkValueRead(0x0000, 0x07, 0x001);
	    AN_BP_STATUS[2] = TlkValueRead(0x0000, 0x07, 0x005);
	    AN_BP_STATUS[3] = TlkValueRead(0x0000, 0x07, 0x0010);
	    AN_BP_STATUS[4] = TlkValueRead(0x0000, 0x07, 0x0011);
	    AN_BP_STATUS[5] = TlkValueRead(0x0000, 0x07, 0x0012);
	    AN_BP_STATUS[6] = TlkValueRead(0x0000, 0x07, 0x0013);
	    AN_BP_STATUS[7] = TlkValueRead(0x0000, 0x07, 0x0014);
	    AN_BP_STATUS[8] = TlkValueRead(0x0000, 0x07, 0x0015);
	    AN_BP_STATUS[9] = TlkValueRead(0x0000, 0x07, 0x0016);
	    AN_BP_STATUS[10] = TlkValueRead(0x0000, 0x07, 0x0017);
	    AN_BP_STATUS[11] = TlkValueRead(0x0000, 0x07, 0x0018);
	    AN_BP_STATUS[12] = TlkValueRead(0x0000, 0x07, 0x0019);
	    AN_BP_STATUS[13] = TlkValueRead(0x0000, 0x07, 0x001a);
	    AN_BP_STATUS[14] = TlkValueRead(0x0000, 0x07, 0x001b);
	    AN_BP_STATUS[15] = TlkValueRead(0x0000, 0x07, 0x0030);
	    AN_BP_STATUS[16] = TlkValueRead(0x0000, 0x07, 0x0030);
	*/
}

int PMARxLinkRead(int chan)
{
	unsigned int val;
	val = TlkValueRead(chan, PAPDADDR, PMA_STU1);
	val = (val & 0x0002) ? val : TlkValueRead(chan, PAPDADDR, PMA_STU1);
	return ((val & 0x0002) ? 1 : 0);
}

int ChanHsStaRead(int chan)
{
	unsigned int val;
	val = TlkValueRead(chan, VSPDRADDR, CHAN_STU);
	val = TlkValueRead(chan, VSPDRADDR, CHAN_STU);
	return val;
}




