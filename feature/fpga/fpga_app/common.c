/*************************************************************
 * Filename     : 
 * Description  : 
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/
/* include -------------------------------------------------------------------*/
#include "mconfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "common.h"
#include "hal_uart.h"
#include "fifo.h"
#include "halsw_i2c.h"
#include "stdio.h"
#include "delay.h"
#include "string.h"
#include "hal_time.h"
#include "tlk10232.h"
#include "hal_io.h"
#include "upgrade.h"
#include "spi_flash.h"
#include "fpga_debug.h"
#include "tsensor.h"
#include "halcmd_msg.h"
#include "fpga_api.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const unsigned char lclmac[6] = {0x44, 0x44, 0x44, 0x44, 0x44, 0x44};
const unsigned char srcmac[6] = {0x44, 0x44, 0x44, 0x44, 0x44, 0x44};
const unsigned char rmtmac[6] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
const fpgainfo defaultfpgainfo = {{0x44, 0x44, 0x44, 0x44, 0x44, 0x44}, 
                                  {0x44, 0x44, 0x44, 0x44, 0x44, 0x44}, 
                                  {0x88, 0x88, 0x88, 0x88, 0x88, 0x88},
	                               0x00, 0x00, 0x00, 0x00
                                  };

const unsigned char card1name[19] = {0x47, 0x56, 0x33, 0x53, 0x2D, 0x48,0x4F,0x4E,0x55,0x45,0x2D,0x51,0x4D,0x5F,0x56,0x31,0x2E,0x30,0x30};
									//"GV3S-HONUE-QM"
const unsigned char card1coding[8] = {0xC0,0x01,0x03,0xFF,0xFF,0xFF,0xFF,0xFF};

unsigned char v_id[4] = {0};
extern unsigned char DevMac[6];

static boardinfo localboard;
static device_t deviceinfo;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
unsigned short int 	MsgLenGet(unsigned short int len)
{
	len = SWP16(len);
	return CONV16_14(len);
}

unsigned short int MsgLenSet(unsigned short int len)
{
	len = CONV14_16(len);
	return SWP16(len);
}

/* serial message variance */
typedef enum {REND = 0, RSTART} rstate;
#define NMSGHLD		0xf0
#define CMDHLD		0xfa
#define NMSGEND		0xfb


//serialbufftype * srlbuffp = NULL;

static fifo_t ut1fifo;
static serialbufftype ut1rx = {&ut1fifo};
static fifo_p ut1fp = &ut1fifo;
static unsigned char *ut1p = ut1rx.buff;
static unsigned char ut1rxstate;
static unsigned int ut1rxtimeout;


void UartR1TimeoutSet(void)
{
	unsigned int uctime;
	
	if (ut1rxstate)
	{
        SysTickGet(&uctime);
		U32TIMEOUT(uctime, ut1rxtimeout);
		if (uctime >= MBLINK)
		{
			ut1rxstate = REND;
            st_fpga_debug(ST_FPGA_INFO,"UpgradeRxMsg timeout %d",uctime);
		}
	}
}


int Uart1FifoGet(unsigned char *buf, unsigned int size)
{
	unsigned int ret;
	ret = Fifo_Read(ut1fp, buf,  size);
	return ret;
}

static unsigned int testtime, testtime2;
void Ut1Recv(unsigned char ch)
{
	serial10Gcmdp cmdp;
	unsigned short int len;
	ut1rx.cnt++;
	if (!ut1rxstate && (ch == NMSGHLD))
	{
		ut1rxstate = RSTART;
		ut1p = ut1rx.buff;
		ut1rx.cnt = 1;
		SysTickGet(&ut1rxtimeout);
		SysTickGet(&testtime);
	}
	if (ut1rxstate)
	{
		*(ut1p++) = ch;
		if (ch == NMSGEND)
		{
			if (ut1rx.cnt >= sizeof(cmdp->head))
			{
				cmdp = (serial10Gcmdp)(ut1rx.buff + sizeof(msghead));
				len = MsgLenGet(cmdp->head.len) + sizeof(cmdp->head) + sizeof(msghead);
				if (ut1rx.cnt == len)
				{
					ut1rxstate = REND;
//					*ut1p = ch;
					if ((ut1rx.buff[0] == NMSGHLD) && (ut1rx.buff[ut1rx.cnt - 1] == NMSGEND))
					{
						Fifo_Write(ut1fp, (unsigned char *)&ut1rx.cnt, sizeof(ut1rx.cnt));
						Fifo_Write(ut1fp, ut1rx.buff, ut1rx.cnt);
//						ut1msgcnt++;
					}
					ut1rx.cnt = 0;
					ut1p = ut1rx.buff;
				}
			}
		}
		if (ut1rx.cnt > sizeof(ut1rx.buff))
		{
			ut1rx.cnt  = 0;
			ut1rxstate = REND;
		}
	}
	/*
		if(ut1rxstate && (ch == NMSGEND)){
			ut1rxstate = REND;
			*ut1p = ch;
			if((ut1rx.buff[0] == NMSGHLD) && (ut1rx.buff[ut1rx.cnt - 1] == NMSGEND)){
				Fifo_Write(ut1fp, (unsigned char *)&ut1rx.cnt, sizeof(ut1rx.cnt));
				Fifo_Write(ut1fp, ut1rx.buff,ut1rx.cnt);
			}
			ut1rx.cnt = 0;
			ut1p = ut1rx.buff;
		}
		else if(ut1rxstate){
			*(ut1p++) = ch;
		}
	*/
}


static fifo_t ut4fifo;
static serialbufftype ut4rx = {&ut4fifo};
static fifo_p ut4fp = &ut4fifo;
static unsigned char *ut4p = ut4rx.buff;
static unsigned char ut4rxstate;
static unsigned int ut4rxtimeout;


void UartR4TimeoutSet(void)
{
	unsigned int uctime, lctime;
	SysTickGet(&uctime);
	lctime = uctime;
	if (ut4rxstate)
	{
		U32TIMEOUT(uctime, ut4rxtimeout);
		if (uctime >= MBLINK)
		{
			ut4rxstate = REND;
		}
	}
}

int Uart4FifoGet(unsigned char *buf, unsigned int size)
{
	unsigned int ret;
	ret = Fifo_Read(ut4fp, buf,  size);
	return ret;
}

static unsigned int testtime, testtime2;
void Ut4Recv(unsigned char ch)
{
	serial10Gcmdp cmdp;
	unsigned short int len;
	ut4rx.cnt++;
	if (!ut4rxstate && (ch == NMSGHLD))
	{
		ut4rxstate = RSTART;
		ut4p = ut4rx.buff;
		ut4rx.cnt = 1;
		SysTickGet(&ut4rxtimeout);
		SysTickGet(&testtime);
	}
	if (ut4rxstate)
	{
		*(ut4p++) = ch;
		if (ch == NMSGEND)
		{
			if (ut4rx.cnt >= sizeof(cmdp->head))
			{
				cmdp = (serial10Gcmdp)(ut4rx.buff + sizeof(msghead));
				len = MsgLenGet(cmdp->head.len) + sizeof(cmdp->head) + sizeof(msghead);
				if (ut4rx.cnt == len)
				{
					ut4rxstate = REND;
					*ut4p = ch;
					if ((ut4rx.buff[0] == NMSGHLD) && (ut4rx.buff[ut4rx.cnt - 1] == NMSGEND))
					{
						Fifo_Write(ut4fp, (unsigned char *)&ut4rx.cnt, sizeof(ut4rx.cnt));
						Fifo_Write(ut4fp, ut4rx.buff, ut4rx.cnt);
//						ut4msgcnt++;
					}
					ut4rx.cnt = 0;
					ut4p = ut4rx.buff;
				}
			}
		}
		if (ut4rx.cnt > sizeof(ut4rx.buff))
		{
			ut4rx.cnt  = 0;
			ut4rxstate = REND;
		}
	}
	/*
		if(ut4rxstate && (ch == NMSGEND)){
			ut4rxstate = REND;
			*ut4p = ch;
			if((ut4rx.buff[0] == NMSGHLD) && (ut4rx.buff[ut4rx.cnt - 1] == NMSGEND)){
				Fifo_Write(ut4fp, (unsigned char *)&ut4rx.cnt, sizeof(ut4rx.cnt));
				Fifo_Write(ut4fp, ut4rx.buff,ut4rx.cnt);
			}
			ut4rx.cnt = 0;
			ut4p = ut4rx.buff;
		}
		else if(ut4rxstate){
			*(ut4p++) = ch;
		}
	*/
}




#define CMSGHLD		0xf0
#define CMSGEND		0xff

//serialbufftype * srlbuffp = NULL;

static fifo_t ut2fifo;
static serialbufftype ut2rx = {&ut2fifo};
static fifo_p ut2fp = &ut2fifo;
static unsigned char *ut2p = ut2rx.buff;
static unsigned char ut2rxstate;
static unsigned int ut2rxtimeout;



void UartR2TimeoutSet(void)
{
	unsigned int uctime, lctime;
	SysTickGet(&uctime);
	lctime = uctime;
	if (ut2rxstate)
	{
		U32TIMEOUT(uctime, ut2rxtimeout);
		if (uctime >= MBLINK)
		{
			ut2rxstate = REND;
		}
	}
}


#if (defined PHY_TEST)


void Ut2Recv(unsigned char ch)
{
	ut2rx.cnt++;
	if (ch == CMSGHLD)
	{
		ut2rxstate = RSTART;
		ut2p = ut2rx.buff;
		ut2rx.cnt = 1;
	}
	if (ut2rxstate)
	{
		*(ut2p++) = ch;
		if (ch == CMSGEND)
		{
			if (ut2rx.cnt > 4)
			{
				if (ut2rx.cnt == (ut2rx.buff[4] + 5))
				{
					ut2rxstate = REND;
					if ((ut2rx.buff[0] == CMSGHLD) && (ut2rx.buff[ut2rx.cnt - 1] == CMSGEND))
					{
//						Fifo_Add_Value(ut2fp, ut2rx.cnt);
						Fifo_Write(ut2fp, (unsigned char *)&ut2rx.cnt, sizeof(ut2rx.cnt));
						Fifo_Write(ut2fp, ut2rx.buff, ut2rx.cnt);
					}
					ut2rx.cnt = 0;
					ut2p = ut2rx.buff;
				}
			}
		}
	}
	if (ut2rx.cnt > sizeof(ut2rx.buff))
	{
		ut2rxstate = REND;
		ut2rx.cnt = 0;
		ut2p = ut2rx.buff;
	}
}

#else
void Ut2Recv(unsigned char ch)
{
	
	ut2rx.cnt++;
	if (!ut2rxstate && (ch == CMSGHLD))
	{
		ut2rxstate = RSTART;
		ut2p = ut2rx.buff;
		ut2rx.cnt = 1;
		SysTickGet(&ut2rxtimeout);
	}
	if (ut2rxstate)
	{
		*(ut2p++) = ch;
		if (ch == CMSGEND)
		{	uart2cmdp ut2rxcmdp;
			unsigned char len;
			ut2rxcmdp = (uart2cmdp)ut2rx.buff;
			len = &ut2rxcmdp->len - &ut2rxcmdp->head + 1;
			if (ut2rx.cnt > len)
			{
				if (ut2rx.cnt == (len + ut2rxcmdp->len))
				{
					ut2rxstate = REND;
					if ((ut2rx.buff[0] == CMSGHLD) && (ut2rx.buff[ut2rx.cnt - 1] == CMSGEND))
					{
//						Fifo_Add_Value(ut2fp, ut2rx.cnt);
						Fifo_Write(ut2fp, (unsigned char *)&ut2rx.cnt, sizeof(ut2rx.cnt));
						Fifo_Write(ut2fp, ut2rx.buff, ut2rx.cnt);
					}
					ut2rx.cnt = 0;
					ut2p = ut2rx.buff;
				}
			}
		}
	}
	if (ut2rx.cnt > sizeof(ut2rx.buff))
	{
		ut2rxstate = REND;
		ut2rx.cnt = 0;
		ut2p = ut2rx.buff;
	}
}

/*void Ut2Recv(unsigned char ch)
{
	ut2rx.cnt++;
	if (ch == CMSGHLD)
	{
		ut2rxstate = RSTART;
		ut2p = ut2rx.buff;
		ut2rx.cnt = 1;
	}
	if (ut2rxstate && (ch == CMSGEND))
	{
		ut2rxstate = REND;
		*ut2p = ch;
		if ((ut2rx.buff[0] == CMSGHLD) && (ut2rx.buff[ut2rx.cnt - 1] == CMSGEND))
		{
//			Fifo_Add_Value(ut2fp, ut2rx.cnt);
			Fifo_Write(ut2fp, (unsigned char *)&ut2rx.cnt, sizeof(ut2rx.cnt));
			Fifo_Write(ut2fp, ut2rx.buff, ut2rx.cnt);
		}
		ut2rx.cnt = 0;
		ut2p = ut2rx.buff;
	}
	else if (ut2rxstate)
	{
		*(ut2p++) = ch;
	}
}*/
#endif


void DeviceInfoInit(void)
{
	device_p dev = &deviceinfo;

	dev->d_flashsize = (*(__IO u16*)(0x1FFF7A22));
	dev->d_id0 = *(__IO u32*)(0x1FFF7A10);
	dev->d_id1 = *(__IO u32*)(0x1FFF7A14);
	dev->d_id2 = *(__IO u32*)(0x1FFF7A18);
}

char* BoardInfoGet(void)
{
	At24_Sequential_Read(I2CDEV_0, E2DEVADD0, BOARDINFOADDR, (unsigned char *)&localboard, sizeof(boardinfo));
	if (localboard.mcutype != BOARDMCU) {
		localboard.mcutype = BOARDMCU;
		At24_Page_Write(I2CDEV_0, E2DEVADD0, BOARDINFOADDR, (unsigned char *)&localboard, sizeof(boardinfo));
	}

	if (strcmp((char *)localboard.boardname, BOARD_NAME)) {
		memset(localboard.boardname, 0, sizeof(localboard.boardname));
		memcpy(localboard.boardname, BOARD_NAME, sizeof(BOARD_NAME));
		At24_Page_Write(I2CDEV_0, E2DEVADD0, BOARDINFOADDR, (unsigned char *)&localboard, sizeof(boardinfo));
	}
    
	memcpy(v_id,(unsigned char *)&(localboard.remoteid),4);

	return (char *)&localboard;
}


void BoardInfoSet(void)
{
	At24_Page_Write(I2CDEV_0, E2DEVADD0, BOARDINFOADDR, (unsigned char *)&localboard, sizeof(boardinfo));
}


void PakDatConv(unsigned char *dist, unsigned char *src, unsigned char size)
{
	unsigned char i;
	unsigned char hch;
	if (size > 9)
	{
		return;
	}
	hch = src[size - 1];
	for (i = 0; i < size - 1; i++)
	{
		*dist = *src | ((hch & (1 << i)) ? 0x80 : 0x00);
	}
}
unsigned char GetFpgaVersion(unsigned int fcodeid)
{
	return ((fcodeid >> 24) & 0xff);
}

void GetFpgaImageName(unsigned int fcodeid, unsigned char *fimagename)
{
	unsigned char id[8];
	for(unsigned char i = 0; i < 8; i++)
	{
		id[i] = (fcodeid >> (28 - i * 4)) & 0x0f;
		if(id[i] <= 0x09)
			id[i] += 0x30;
		else
			id[i] += 0x37;
	}
	*(fimagename + 6) = id [6];
	*(fimagename + 8) = id [4];	
	*(fimagename + 9) = id [7];
	*(fimagename + 11) = id [2];
	*(fimagename + 12) = id [5];
	*(fimagename + 13) = id [3];
	*(fimagename + 15) = id [0];
	*(fimagename + 16) = id [1];
}

unsigned char CrcCreat(unsigned char *buf, unsigned int size)
{
	unsigned int i;
	unsigned char crc;
	crc = buf[0];
	for (i = 1; i < size; i++)
	{
		crc = ~(crc ^ buf[i]);
	}
	crc &= ~0x80;
	return crc;
}

static unsigned char mcuversion;
void FImageinfoCheck(void)
{
	upgradeif imageinfo;
	unsigned char version;
	unsigned int fcodeid = 0x00;
	unsigned char fimagename[50] = FIMAGENAME;
        unsigned char checkCount = 0;
        
	while(!fcodeid && (checkCount < 10))
	{
		SWI2cSequentialRead(I2CDEV_0, FPGAADD1, FCODEIDADD0, (unsigned char *)&fcodeid, sizeof(fcodeid));
		
#if BOARD_GV3S_HONUE_QM
		vTaskDelay(1000);
#else
		Delay_Ms(1000);
#endif
                checkCount++;
	}
	version = GetFpgaVersion(fcodeid);
	GetFpgaImageName(fcodeid, fimagename);
	At24_Sequential_Read(I2CDEV_0, E2DEVADD0, UPGRADEINFOADD3, (unsigned char *)&imageinfo, sizeof(imageinfo));
	if (!strcmp((char *)fimagename, (char *)imageinfo.image.imgname)) {
		if ((unsigned char)imageinfo.image.imgver != version) {
			imageinfo.image.imgver = version;
		}
	}
	else {
		imageinfo.image.imgver = version;
		memcpy(imageinfo.image.imgname, fimagename, sizeof(fimagename));
	}
	At24_Page_Write(I2CDEV_0, E2DEVADD0, UPGRADEINFOADD3, (unsigned char *)&imageinfo, sizeof(imageinfo));

}



//void MacInfoProcess(unsigned char *msg, unsigned char size)
void MacInfoProcess(int port, unsigned char *msg, int *size, int op)
{
	struct macinfo
	{
		unsigned char mactype;
		unsigned char mac[6];
	};
	struct macinfo *macinfop = (struct macinfo *)msg;
	unsigned int eoffset, foffset;
	unsigned char i, valid = 0;
	if (op)
	{
		unsigned char mactp[3];
		*size = ((*size) > sizeof(mactp)) ? sizeof(mactp) : *size ;
		memcpy(mactp, (unsigned char *)msg, *size);
		for (i = 0; i < *size; i++)
		{
			if (mactp[i] < 0x03)
			{
				macinfop->mactype = mactp[i];
				eoffset = FPGAINFOADD0 + port * FPGAINFOSIZE + mactp[i] * sizeof(macinfop->mac);
				At24_Sequential_Read(I2CDEV_0, E2DEVADD0, eoffset, macinfop->mac, sizeof(macinfop->mac));
				valid++;
				macinfop++;
			}
		}
		*size = valid * sizeof(struct macinfo);
	}
	else
	{
		for (i = 0; i < *size; i += sizeof(struct macinfo))
		{
			if (macinfop->mactype < 03)
			{
				foffset = FPGABACE + port * FPGABANKSIZE + macinfop->mactype * sizeof(macinfop->mac);
				eoffset = FPGAINFOADD0 + port * FPGAINFOSIZE + macinfop->mactype * sizeof(macinfop->mac);
				At24_Page_Write(I2CDEV_0, E2DEVADD0, eoffset, macinfop->mac, sizeof(macinfop->mac));
				SWI2cSequentialWrite(I2CDEV_0, FPGAADD1, foffset, macinfop->mac, sizeof(macinfop->mac));
			}
			macinfop++;
		}
	}
}


//void ChanMaskProcess(unsigned char *msg, unsigned char size)
void ChanMaskProcess(int port, unsigned char *msg, int *size, int op)
{
	struct chaninfo
	{
		unsigned char chan;
		unsigned char opmask;
	};
	struct chaninfo *chaninfop = (struct chaninfo *)msg;
	unsigned char i;
	unsigned int eoffset, foffset;
	fpgainfo temp;
	unsigned char chansw[2] ;
	foffset = FPGABACE + port * FPGABANKSIZE + (&temp.chansw1 - (unsigned char *)&temp);
	eoffset = FPGAINFOADD0 + port * FPGAINFOSIZE + (&temp.chansw1 - (unsigned char *)&temp);
	At24_Sequential_Read(I2CDEV_0, E2DEVADD0, eoffset, (unsigned char *)&chansw, sizeof(chansw));
	if (op)
	{
		if (chaninfop->chan & 0x80)
		{
			*(msg + 1) = chansw[0];
			*(msg + 2) = chansw[1];
			*size = sizeof(struct chaninfo) + 1;
		}
		else
		{
			unsigned char chantp[16];
			if (*size > sizeof(chantp))
			{
				*size = 0;
			}
			memcpy(chantp, (unsigned char *)msg, *size);
			for (i = 0; i < *size; i++)
			{
				chaninfop->chan = chantp[i];
				if (chaninfop->chan > 8)
				{
					chaninfop->opmask = (chansw[1] >> (chaninfop->chan & 0x07)) & 0x01;
				}
				else
				{
					chaninfop->opmask = (chansw[0] >> (chaninfop->chan & 0x07)) & 0x01;
				}
				chaninfop++;
			}
			*size = i * sizeof(struct chaninfo);
		}
	}
	else
	{
		if (chaninfop->chan & 0x80)
		{
			chansw[1] = *(((unsigned char *)&chaninfop->opmask) + 1);
			chansw[0] = chaninfop->opmask;
		}
		else
		{
			for (i = 0; i < *size; i += sizeof(struct chaninfo))
			{
				if (chaninfop->chan > 8)
				{
					chansw[1] = ~(1 << (chaninfop->chan & 0x07)) & chansw[1];
					chansw[1] = (chaninfop->opmask << (chaninfop->chan & 0x07)) | chansw[1];
				}
				else
				{
					chansw[0] = ~(1 << (chaninfop->chan & 0x07)) & chansw[0];
					chansw[0] = (chaninfop->opmask << (chaninfop->chan & 0x07)) | chansw[0];
				}
				chaninfop++;
			}
		}
		At24_Page_Write(I2CDEV_0, E2DEVADD0, eoffset, chansw, sizeof(chansw));
		SWI2cSequentialWrite(I2CDEV_0, FPGAADD1, foffset, chansw, sizeof(chansw));
	}
}

//void MacMaskProcess(unsigned char *msg, unsigned char size)
void MacMaskProcess(int port, unsigned char *msg, int *size, int op)
{
	struct macmaskinfo
	{
		unsigned char type;
		unsigned char opmask;
	};
	struct macmaskinfo *macmaskinfop = (struct macmaskinfo *)msg;
	unsigned char i;
	unsigned int eoffset, foffset;
	fpgainfo temp;
	unsigned char confmode;
	foffset = FPGABACE + port * FPGABANKSIZE + (&temp.confmode - (unsigned char *)&temp);
	eoffset = FPGAINFOADD0 + port * FPGAINFOSIZE + (&temp.confmode - (unsigned char *)&temp);
	SWI2cSequentialRead(I2CDEV_0, FPGAADD1, foffset, &confmode, sizeof(confmode));
	if (op)
	{
		unsigned char macmasktp[3];
		if (*size > sizeof(macmasktp))
		{
			*size = 0;
		}
		memcpy(macmasktp, (unsigned char *)msg, *size);
		for (i = 0; i < *size; i++)
		{
			macmaskinfop->type = macmasktp[i];
			macmaskinfop->opmask = ((1 << macmaskinfop->type) & confmode) >> macmaskinfop->type;
			macmaskinfop++;
		}
		*size = i * sizeof(struct macmaskinfo);
	}
	else
	{
		for (i = 0; i < *size; i += sizeof(struct macmaskinfo))
		{
			confmode &= ~(1 << macmaskinfop->type);
			confmode |= (macmaskinfop->opmask << macmaskinfop->type);
			macmaskinfop++;
		}
		At24_Page_Write(I2CDEV_0, E2DEVADD0, eoffset, (unsigned char *)&confmode, sizeof(confmode));
		SWI2cSequentialWrite(I2CDEV_0, FPGAADD1, foffset, (unsigned char *)&confmode, sizeof(confmode));
	}
}

void ColorModeProcess(int port, unsigned char *msg, int *size, int op)
{
	unsigned char colorbar1;
	unsigned int  foffset;
	fpgainfo temp;
	foffset = FPGABACE + port * FPGABANKSIZE + (&temp.colorbar1 - (unsigned char *)&temp);
//	eoffset = FPGAINFOADD0 + port * FPGAINFOSIZE + (&temp.confmode - (unsigned char *)&temp);
	SWI2cRandomRead(I2CDEV_0, FPGAADD1, foffset, &colorbar1);
	colorbar1 &= 0xf0;
	if (op)
	{
	}
	else
	{
		colorbar1 |= (*msg & 0x0f);
		SWI2cRandomWrite(I2CDEV_0, FPGAADD1, foffset, &colorbar1);
	}
}


#define OPTLINK		1
#define BOARDLINK	2
#define ALLLINK		3
void ColorChanProcess(int port, unsigned char *msg, int *size, int op)
{
	struct colorchanif
	{
		unsigned char colorchan;
		unsigned char colorset;
	};
	struct colorchanif *clrchanp = (struct colorchanif *)(msg + 2);
	unsigned char colortype = *msg;
	unsigned char clcchannum = *(msg + 1);
	unsigned int  foffset;
	unsigned char i;
	fpgainfo temp;
	unsigned char colorbar;
	foffset = FPGABACE + port * FPGABANKSIZE + (&temp.confmode - (unsigned char *)&temp);
	SWI2cSequentialRead(I2CDEV_0, FPGAADD1, foffset, &colortype, 1);
	colortype &= 0x3f;
	colortype |= (*msg << 6);
	foffset = FPGABACE + port * FPGABANKSIZE + (&temp.colorbar2 - (unsigned char *)&temp);
	SWI2cSequentialRead(I2CDEV_0, FPGAADD1, foffset, &colorbar, 1);
//	colorbar[0] = (colorbar[0] & (unsigned char )~(ALLLINK)) | colortype;
	colorbar &= 0x00;
	colorbar |= *(msg + 1);
	if (op)
	{
	}
	else
	{
//		for(i=0; i<clcchannum; i++){
//			colorbar[1] &= ~(1 << clrchanp->colorchan);
//			colorbar[1] |= (clrchanp->colorset << clrchanp->colorchan);
//		}
//		clrchanp++;
		SWI2cSequentialWrite(I2CDEV_0, FPGAADD1, foffset, &colorbar, sizeof(colorbar));
		/* type switch */
		foffset = FPGABACE + port * FPGABANKSIZE + (&temp.confmode - (unsigned char *)&temp);
		SWI2cSequentialWrite(I2CDEV_0, FPGAADD1, foffset, &colortype, 1);
	}
}


//void ModModeProcess(unsigned char *msg, unsigned char size)
void ModModeProcess(int port, unsigned char *msg, int *size, int op)
{
	struct modmode
	{
		unsigned char mode;
		unsigned char alloc;
	};
	struct modmode *modinfop = (struct modmode *)msg;
	unsigned int eoffset, foffset;
	fpgainfo temp;
	unsigned char mmode = 0, malloc;
	if (port)
	{
		return;
	}
	/* get original mmode and malloc */
	foffset = FPGABACE + (&temp.confmode - (unsigned char *)&temp);
	SWI2cRandomRead(I2CDEV_0, FPGAADD1, foffset, &mmode);
	foffset = FPGABACE + FPGABANKSIZE + (&temp.confmode - (unsigned char *)&temp);
	SWI2cRandomRead(I2CDEV_0, FPGAADD1, foffset, &malloc);
	if (op)
	{
		modinfop->mode = (mmode >> 3) & 0x07;
		modinfop->alloc = (malloc >> 2) & 0x0f;
		*size = sizeof(struct modmode);
	}
	else
	{
		/* alter mmode  */
		foffset = FPGABACE + (&temp.confmode - (unsigned char *)&temp);
		eoffset = FPGAINFOADD0 + (&temp.confmode - (unsigned char *)&temp);
		mmode = (~(0x07 << 3) & mmode) | (modinfop->mode << 3);
		At24_Byte_Write(I2CDEV_0, E2DEVADD0, eoffset, (unsigned char *)&mmode);
		SWI2cRandomWrite(I2CDEV_0, FPGAADD1, foffset, (unsigned char *)&mmode);
		/* alter malloc */
		foffset = FPGABACE + FPGABANKSIZE + (&temp.confmode - (unsigned char *)&temp);
		eoffset = FPGAINFOADD0 + FPGAINFOSIZE + (&temp.confmode - (unsigned char *)&temp);
		malloc = (~(0x0f << 2) & malloc) | (modinfop->alloc << 2);
		SWI2cRandomWrite(I2CDEV_0, FPGAADD1, foffset, (unsigned char *)&malloc);
		At24_Byte_Write(I2CDEV_0, E2DEVADD0, eoffset, (unsigned char *)&malloc);
	}
}


void ErrRateProcess(int port, unsigned char *msg, int *size, int op)
{
	unsigned char err[3] ;
	unsigned int foffset;
	foffset = FPGABACE + port * FPGABANKSIZE + 0x19;
	SWI2cSequentialRead(I2CDEV_0, FPGAADD1, foffset, err, sizeof(err));
	if (op)
	{
		memcpy(msg, err, sizeof(err));
		*size = sizeof(err);
	}
}

void StatusProcess(int port, unsigned char *msg, int *size, int op)
{
	struct bstatus
	{
		unsigned char boardtype;
		unsigned char rev[2];
		unsigned char optlock;
		unsigned char vxex1;
		unsigned char vxex2;
		unsigned char optvinloc1;
		unsigned char optvinloc2;
		unsigned char ethloc1;
		unsigned char ethloc2;
		unsigned char link;
		unsigned char optvoutloc1;
		unsigned char optvoutloc2;
		unsigned char reved;
	};
	struct bstatus *bstap = (struct bstatus *)msg;
	unsigned char status[8];
	unsigned int foffset;
	unsigned int ccid;
//	ccid = SWP32(MCUCODEID);
	foffset = FPGABACE + 0x1c;
	SWI2cSequentialRead(I2CDEV_0, FPGAADD1, foffset, (unsigned char *)status, 4);
	foffset = FPGABACE + FPGABANKSIZE + 0x1c;
	SWI2cSequentialRead(I2CDEV_0, FPGAADD1, foffset, (unsigned char *)status + 4, 4);
	if (op)
	{
		bstap->boardtype = 0x10;
		bstap->rev[0] = 0;
		bstap->rev[1] = 0;
		bstap->vxex1 = ((((status[3] & 0x03)) ? ((status[3] & 0x03) * 2 + 2) : 0) << 4) | (((status[3] & 0x30) == 0x30) ? 8 : (((status[3] & 0x30) >> 4) * 2));
		bstap->vxex2 = ((((status[7] & 0x03)) ? ((status[7] & 0x03) * 2 + 2) : 0) << 4) | (((status[7] & 0x30) == 0x30) ? 8 : (((status[7] & 0x30) >> 4) * 2));
		bstap->optlock = ((status[0] & 0x10) << 3) | ((status[4] & 0x10) << 2);
		bstap->optvinloc1 = HLCONV8(status[1]);
		bstap->optvinloc2 = HLCONV8(status[5]);
		bstap->ethloc1 = 0;
		bstap->ethloc2 = 0;
		bstap->link = 0;
		bstap->optvoutloc1 = HLCONV8(status[2]);
		bstap->optvoutloc2 = HLCONV8(status[6]);
		bstap->reved = 0;
		*size = sizeof(struct bstatus);
	}
}

void FpgaDefaultProcess(int port, unsigned char *msg, int *size, int op)
{
	unsigned int eoffset;
	fpgainfo macif;
	unsigned char a[2];
	memset(&macif, 0, sizeof(macif));
	eoffset = FPGAINFOADD0 + port * FPGAINFOSIZE ;
	if (op)
	{
		At24_Sequential_Read(I2CDEV_0, E2DEVADD0, eoffset, (unsigned char *)&macif, sizeof(macif));
		At24_Random_Read(I2CDEV_0, E2DEVADD0, eoffset + (&macif.confmode - (unsigned char *)&macif), a);
		memcpy(msg, (unsigned char *)&macif, sizeof(macif));
		*size = sizeof(macif);
	}
	else
	{
		memcpy(&macif, &defaultfpgainfo, sizeof(macif));
		macif.macset = 0xa5a5;
		At24_Page_Write(I2CDEV_0, E2DEVADD0, eoffset, (unsigned char *)&macif, sizeof(macif));
	}
}

typedef enum {IDINIT, IDEXIST} idstatus;
static rmtid rmtidinfo[RMTID_MAXNUM] = {{IDINIT}, {IDINIT}};
void RMTIdInfoProcess(int port, unsigned char *msg, int *size, int op)
{
	struct id
	{
		unsigned char chan;
		unsigned char level;
		unsigned char rmtid[4];
	};
	struct id *idp = (struct id *)(msg + 1);
	unsigned int  i;
	unsigned char validid = 0;
	rmtidp rmtidinfop = NULL;
	unsigned char *p = msg;
	rmtidinfop = rmtidinfo;
	if (op)
	{
		for (i = 0; i < RMTID_MAXNUM; i++)
		{
			if (rmtidinfop->isexist)
			{
				idp->chan = rmtidinfop->clientport;
				idp->level = rmtidinfop->level;
				*(unsigned int *)&idp->rmtid = SWP32(rmtidinfop->rmtid);
				idp++;
				validid++;
			}
			rmtidinfop++;
		}
		*p = validid;
		*size = validid * sizeof(struct id) + 1;
	}
}


void CardIdProcess(int port, unsigned char *msg, int *size, int op)
{
	struct cardids
	{
		unsigned char rev[12];
		unsigned int fpgaid;
		unsigned int mcuid;
	};

	struct cardids *cardidp = (struct cardids *)msg;

	unsigned int foffset;

	if (op) {
		memset(msg, 0, sizeof(struct cardids));
		cardidp->mcuid = GetCodeId(MCUCODEID);
		SWI2cSequentialRead(I2CDEV_0, FPGAADD1, 0xfb, (unsigned char *)&cardidp->fpgaid, sizeof(cardidp->fpgaid));
		*size = sizeof(struct cardids);
	}
}

void BoardInfoProcess(int port, unsigned char *msg, int *size, int op)
{
	struct boardset
	{
		unsigned char infotype;
		unsigned char context[4];
	};
	struct boardset *bdsetp = (struct boardset *)msg;
	boardinfop boardifp = &localboard;
	unsigned int eoffset;
	eoffset = BOARDINFOADDR;
	At24_Sequential_Read(I2CDEV_0, E2DEVADD0, eoffset, (unsigned char *)boardifp, sizeof(boardinfo));
	if (op)
	{
		*msg = bdsetp->infotype;
		*(unsigned int *)(msg + 1) = boardifp->remoteid;
		*size = sizeof(struct boardset);
	}
	else
	{
		boardifp->remoteid = *(unsigned int *)&bdsetp->context;
		At24_Page_Write(I2CDEV_0, E2DEVADD0, eoffset, (unsigned char *)boardifp, sizeof(boardinfo));
	}
}


void FpgaRegProcess(int port, unsigned char *msg, int *size, int op)
{
	struct fpgareg {
		unsigned char	addr;
		unsigned char	value;
	};

	struct fpgareg * fpgaregp = (struct fpgareg *)msg;
	if (op) {
		SWI2cSequentialRead(I2CDEV_0, FPGAADD1, fpgaregp->addr, &fpgaregp->value, 1);
		*size = sizeof(struct fpgareg);
	}
	else {
		SWI2cSequentialWrite(I2CDEV_0, FPGAADD1, fpgaregp->addr, &fpgaregp->value, 1);
	}
}

void PhyRegProcess(int port, unsigned char *msg, int *size, int op)
{
	struct phyreg {
		unsigned short int devaddr;
		unsigned short int regaddr;
		unsigned short int regvalue;
	};

	
	struct phyreg *  phyregp = (struct phyreg *)msg;
	if (op) {
		phyregp->devaddr = SWP16(phyregp->devaddr);
		phyregp->regaddr = SWP16(phyregp->regaddr);
		phyregp->regvalue = TlkValueRead(port, phyregp->devaddr, phyregp->regaddr);
		phyregp->devaddr = SWP16(phyregp->devaddr);
		phyregp->regaddr = SWP16(phyregp->regaddr);
		phyregp->regvalue = SWP16(phyregp->regvalue);
		*size = sizeof(struct phyreg);
	}
	else {
		phyregp->devaddr = SWP16(phyregp->devaddr);
		phyregp->regaddr = SWP16(phyregp->regaddr);
		phyregp->regvalue = SWP16(phyregp->regvalue);
		TlkValueWrite(port, phyregp->devaddr, phyregp->regaddr, phyregp->regvalue);
		
	}
}


#if (defined PHY_TEST)

typedef enum phy_op
{
    SAVE = 0,
    REG,
    DATAPATH,
    STATUSREAD
};
void PhyTestProcess(int port, unsigned char *msg, int *size, int op)
{
	struct phytest
	{
		unsigned char phychan;
		unsigned char devaddr;
		unsigned short subaddr;
		unsigned short val;
	};
	unsigned char subcmd = *(msg + 1);
	switch (subcmd)
	{
		case SAVE:
		{
			unsigned short int regaddr;
			regaddr = (*(msg + 2)) ? PHYREGADDR2 : PHYREGADDR1;
			if (op)
			{
				At24_Sequential_Read(I2CDEV_0, E2DEVADD0, regaddr, msg + 3, 6);
				*size = 6 + 3;
				*msg = *size + 1;
			}
			else
			{
				if (*size != 8)
				{
					*(msg + 3) = 0xee;
					*(msg + 4) = 0xee;
					*size = 2 + 3;
					*msg = *size + 1;
					*(msg - 3) = 1;
					break;
				}
				At24_Page_Write(I2CDEV_0, E2DEVADD0, regaddr, msg + 3, 6);
			}
			break;
		}
		case REG:
		{
			struct phytest *regp = (struct phytest *)(msg + 2);
			regp->phychan = (regp->phychan) ? PHYCHAN2 : PHYCHAN1;
			if (op)
			{
				regp->subaddr = SWP16(regp->subaddr);
				regp->val = TlkValueRead(regp->phychan, regp->devaddr, regp->subaddr);
				regp->val = SWP16(regp->val);
				regp->subaddr = SWP16(regp->subaddr);
			}
			else
			{
				regp->val = SWP16(regp->val);
				regp->subaddr = SWP16(regp->subaddr);
				TlkValueWrite(regp->phychan, regp->devaddr, regp->subaddr, regp->val);
			}
			regp->phychan = (regp->phychan) ? PHYCHAN1 : PHYCHAN2;
			*size += 1;
			break;
		}
		case DATAPATH:
		{
			struct phytest *regp = (struct phytest *)(msg + 2);
			regp->phychan = (regp->phychan) ? PHYCHAN2 : PHYCHAN1;
			regp->val = TlkValueRead(regp->phychan, 0x1e, 0x000e);
			regp->val |= 0x0008;
			TlkValueWrite(regp->phychan, 0x1e, 0x000e, regp->val);
			break;
		}
		case STATUSREAD:
		{
			struct phytest *regp = (struct phytest *)(msg + 2);
			regp->phychan = (regp->phychan) ? PHYCHAN2 : PHYCHAN1;
			Read_status(regp->phychan);
			break;
		}
	}
}
#endif


void UpgradeProcee(int port, unsigned char *msg, int *size, int op)
{
	
	UpgradeImageProsess(msg,(unsigned int*)size);

}

void GetIdMacProcess(int port, unsigned char *msg, int *size, int op)
{
	getidmacmsgp idmacmsgp = (getidmacmsgp)msg;
	idmacmsgp->scode       = 0xef;
	idmacmsgp->remotenum   = 0x01;	//
	idmacmsgp->portnum     = port;
	idmacmsgp->casnum      = 0x00;
	memcpy(idmacmsgp->devicetype,card1coding,8);
	idmacmsgp->idmacnum = 0x11;
	memcpy(idmacmsgp->idnum,v_id,4);
	memcpy(idmacmsgp->mmacnum,DevMac,6);
	*size = 23;
}

void SystemResetProcess(int port, unsigned char *msg, int *size, int op)
{
    uint8_t  Rx_Buffer_copy_a[128] = {0};
    
	switch(*msg){
		case MCURESET:
			NVIC_SystemReset();
			break;
		case FPGARESET:
		//case FPGABACKUP:	
			//FPGA_PROGRAM_START();
			//FPGA3 restart
			Delay_Ms(10);
			FPGAPROMCRL(1);
			Delay_Ms(100);
			FPGAPROMCRL(0);
			Delay_Ms(100);
			FPGAPROMCRL(1);
			Delay_Ms(3000);
			FImageinfoCheck();
			break;					
		case FPGABACKUP:
		//case FPGARESET:
			//------------------------恢复出厂设置------------------------------//
			//----------------复制golden区到primary区-------------------//
		{
			uint32_t copy_1;
			uint8_t erasesector;
			uint8_t  Rx_Buffer_copy;

			SPI3_FLASH_Init();
			// erase flash //
			for (erasesector = 0; (erasesector < 15); erasesector++) {
				SPI3_FLASH_EraseSector(PRIMARY_ADDRESS + (SPI_FLASH_SIZE * erasesector));
			}
			
			for(copy_1 = 0; copy_1 < 7680; copy_1++){
				SPI3_FLASH_ReadBuffer(Rx_Buffer_copy_a,GOLDEN_ADDRESS+copy_1,128) ;
				SPI3_FLASH_WriteBuffer(Rx_Buffer_copy_a,PRIMARY_ADDRESS+copy_1,128);
			}
			SPI_Cmd(SPI3_FLASH_SPI, DISABLE);
			SPI3_MCU_pin_to_3state();
			//----------------------------------------------------------
			//restart fpga
			Delay_Ms(10);
			FPGAPROMCRL(1);Delay_Ms(100);FPGAPROMCRL(0);Delay_Ms(100);FPGAPROMCRL(1);			
			Delay_Ms(3000);
			
			FImageinfoCheck();
			//----------------------------------------------------------
		}
			break;
		default:
			break;
	}
}

char *CpuMsgParse(newcmd_e code)
{
	switch(code)
		{
		case CODEIDSRL		:	return "Get card version info "; break;
		case MCODEUPGRADE	:	return "Upgrade fpga MsgRx"; break;
		case SYSRESET		:	return "Reset fpga "; break;
		default 			:	return "unknow code "; break ;
		}
}

void CpuMsgDump(newcmd_e code)
{ 
	st_fpga_debug(ST_FPGA_DEBUG,"%s\r\n",CpuMsgParse(code));
}

void CpuMsgExe(unsigned char *buf, unsigned int size)
{
	msgheadp msgp = (msgheadp) buf;
	boardinfop boardif = &localboard;
	serial10Gcmdp ccp = (serial10Gcmdp)(&msgp->len + 1);
	unsigned int cnt = size;
	unsigned char port;
	int msgsize = MsgLenGet(ccp->head.len) - (&ccp->subcmd - &ccp->cmd + 1) - 2;
	cmdendp endp;
	
	ccp->mask &= ALLOPMASK;
	if (ccp->mask) {
		if ((ccp->remoteid == boardif->remoteid) || (ccp->remoteid == BOARDCASTID)) {
			if((ccp->port == 0) || (MAXOPPORTNUM < ccp->port)) {
				ccp->port = MAXOPPORTNUM;
			}
			port = (ccp->port - 1);

			/* display command code message */
			CpuMsgDump(ccp->subcmd);
			switch (ccp->subcmd) {
				case MACINFO:
					MacInfoProcess(port, &ccp->subcmd + 1, &msgsize, ccp->cmd);
					break;
				case CHANMASK:
					ChanMaskProcess(port, &ccp->subcmd + 1, &msgsize, ccp->cmd);
					break;
				case MACMASK:
					MacMaskProcess(port, &ccp->subcmd + 1, &msgsize, ccp->cmd);
					break;
				case MODBOARD:
					ModModeProcess(port, &ccp->subcmd + 1, &msgsize, ccp->cmd);
					break;
				case COLORMODE:
					ColorModeProcess(port, &ccp->subcmd + 1, &msgsize, ccp->cmd);
					break;
				case COLORCHAN:
					ColorChanProcess(port, &ccp->subcmd + 1, &msgsize, ccp->cmd);
					break;
				case ERRRATE:
					ErrRateProcess(port, &ccp->subcmd + 1, &msgsize, ccp->cmd);
					break;
				case STATUS:
					StatusProcess(port, &ccp->subcmd + 1, &msgsize, ccp->cmd);
					break;
				case DEFAULTSET:
					FpgaDefaultProcess(port, &ccp->subcmd + 1, &msgsize, ccp->cmd);
					break;
				case BOARDRMTSET:
//					BoardInfoProcess(port, &ccp->subcmd + 1, &msgsize, ccp->cmd);
					break;
				case CODEIDSRL:
					CardIdProcess(port, &ccp->subcmd + 1, &msgsize, ccp->cmd);
					break;
				case MCODEUPGRADE:
                  {
                    //				upgrademsgp up;
					unsigned int swpsize;
					ccp->cmd = CMDREQ;
					//msgsize -= 6;
	//				up = (upgrademsgp)&ccp->subcmd + 1;

					swpsize = *(unsigned int *)(&ccp->subcmd + 4);
					*(unsigned int *)(&ccp->subcmd + 4) = SWP32(swpsize);

					UpgradeProcee(port, &ccp->subcmd + 1, &msgsize, ccp->cmd);
                  }
					break;
				case SYSRESET:
					SystemResetProcess(port, &ccp->subcmd + 1, &msgsize, ccp->cmd);
					break;
				case RMTIDSET:
					GetIdMacProcess(port, &ccp->subcmd + 1, &msgsize, ccp->cmd);
					break;
				default:
					break;
			}
			
			if (ccp->cmd) {
				msgp->src = REMOTE_MCU;
#if 1
				msgp->dst = LOCAL_MCU;
#else
				msgp->dst = (BOX_CPU | ((ccp->slecport & 0x0f) << 4));
#endif
				msgp->len = msgsize + sizeof(serial10Gcmd) + 2; 	// len = sizeof(msghead) + msgsize + sizeof(serial10Gcmd) + 2 - 1;
				ccp->head.len = msgp->len - sizeof(ccp->head);
				ccp->cmd = CMDREQ;
				endp = (cmdendp)(((unsigned char *)&ccp->subcmd + 1) + msgsize);
				ccp->head.len = MsgLenSet(ccp->head.len);
				endp->crc = CrcCreat((unsigned char *)&ccp->head.boxid, msgp->len - 3);
				endp->end = NMSGEND;
				Uart1Pollputc((unsigned char *)msgp, msgp->len + sizeof(msghead));
			}
		}
	}
	else{
		switch (ccp->subcmd) {
			default:
				msgp->src = BOX_CPU;
				msgp->dst = REMOTE_MCU;
				msgp->len = cnt - sizeof(msghead);
				ccp->head.len = MsgLenSet(ccp->head.len);
				Uart1Pollputc((unsigned char *)msgp, cnt);
				break;
		}
	}
}

void LocalMcuExe(unsigned char *buf, unsigned int size)
{
	msgheadp msgp = (msgheadp) buf;
	boardinfop boardif = &localboard;
	serial10Gcmdp ccp = (serial10Gcmdp)(&msgp->len + 1);
	unsigned int cnt = size;
	unsigned char port;
	unsigned char slecport;
	int msgsize = MsgLenGet(ccp->head.len) - (&ccp->subcmd - &ccp->cmd + 1) - 2;
	cmdendp endp;
	
	ccp->mask &= ALLOPMASK;
	if (ccp->mask) {
		if ((ccp->remoteid == boardif->remoteid) || (ccp->remoteid == BOARDCASTID)) {
			if((ccp->port == 0) || (MAXOPPORTNUM < ccp->port)) {
				ccp->port = MAXOPPORTNUM;
			}
			port = (ccp->port - 1);
			slecport = ccp->slecport;
			/* display command code message */
			CpuMsgDump(ccp->subcmd);
			switch (ccp->subcmd) {
				case RMTIDSET:
					    GetIdMacProcess(slecport, &ccp->subcmd + 1, &msgsize, ccp->cmd);
					break;
				default:
					break;
			}
			
			if (ccp->cmd) {
				msgp->src = REMOTE_MCU;
#if 1
				msgp->dst = LOCAL_MCU;
#else
				msgp->dst = (BOX_CPU | ((ccp->slecport & 0x0f) << 4));
#endif
				msgp->len = msgsize + sizeof(serial10Gcmd) + 2; 	// len = sizeof(msghead) + msgsize + sizeof(serial10Gcmd) + 2 - 1;
				ccp->head.len = msgp->len - sizeof(ccp->head);
				ccp->cmd = CMDREQ;
				endp = (cmdendp)(((unsigned char *)&ccp->subcmd + 1) + msgsize);
				ccp->head.len = MsgLenSet(ccp->head.len);
				endp->crc = CrcCreat((unsigned char *)&ccp->head.boxid, msgp->len - 3);
				endp->end = NMSGEND;
				Uart1Pollputc((unsigned char *)msgp, msgp->len + sizeof(msghead));
			}
		}
	}
	else{
		switch (ccp->subcmd) {
			default:
				msgp->src = BOX_CPU;
				msgp->dst = REMOTE_MCU;
				msgp->len = cnt - sizeof(msghead);
				ccp->head.len = MsgLenSet(ccp->head.len);
				Uart1Pollputc((unsigned char *)msgp, cnt);
				break;
		}
	}
}


void McuMsgExe(unsigned char *buf, unsigned int size)
{
	msgheadp msgp = (msgheadp)buf;
	serial10Gcmdp ccp;
	unsigned char i;
	rmtidp rmtidinfop = rmtidinfo;
	cmdendp endp;
	unsigned short int len;
	ccp = (serial10Gcmdp)(buf + sizeof(msghead));
	switch (ccp->subcmd)
	{
		case RMTIDUP:
			for (i = 0; i < RMTID_MAXNUM; i++)
			{
				if (rmtidinfop->isexist == IDINIT)
				{
					rmtidinfop->isexist = IDEXIST;
					rmtidinfop->clientport = (msgp->dst>> 3);
					rmtidinfop->level = ccp->head.boardid;
					rmtidinfop->rmtid = SWP32(ccp->remoteid);
					rmtidinfop->timeout = LLBLINK * 10;
					break;
				}
				else if ((rmtidinfop->clientport == (msgp->dst >> 3)) && (rmtidinfop->level == ccp->head.boardid))
				{
					rmtidinfop->rmtid = SWP32(ccp->remoteid);
					rmtidinfop->timeout = LLBLINK * 10;
					break;
				}
				rmtidinfop++;
			}
			break;
		default:
			ccp->slecport = msgp->dst >> 3;
			msgp->dst = BOX_CPU;
			msgp->src = LOCAL_MCU;

			/* crc  */
			len= MsgLenGet(ccp->head.len);
			endp = (cmdendp)(&ccp->cmd + len - 2);

			endp->crc = CrcCreat(&ccp->head.boardid, len + sizeof(ccp->head) - 3);
			
			Uart1Pollputc((unsigned char *)msgp, msgp->len + sizeof(msghead));
			break;
	}
	/*
		if((ccp->head.boxid == localboard.boxid) && (ccp->head.boardid == localboard.boardid)){
			msgp->dst = BOX_CPU;
			msgp->src = LOCAL_MCU;
			Uart1Pollputc((unsigned char *)msgp, msgp->len + sizeof(msghead));
		}
		switch(mcumsgp->cmd){
			case STATUSUPLOAD:{
				unsigned int i, remotenum;
				rmtinfo * remoteinfop = remoteinfo;
				remotenum = sizeof(remoteinfo) / sizeof(rmtinfo) ;
				for(i=0; i<remotenum; i++){
					if(remoteinfop->rmtid == mcumsgp->rmtid){
	//					memcpy(&remoteinfop->status, (unsigned char *)(&mcumsgp->rmtid + 1), sizeof(rmtinfo));
						remoteinfop++;
					}
					break;
				}
				break;
			}
		}
	*/
}

void FpgaMsgExe(unsigned char *buf, unsigned int size)
{
}

void BoardboxidGet(void)
{
	unsigned char id;
	IdGet(&id);
	localboard.boxid = (id >> 5) | 0x80;
	localboard.boardid = (id & 0x1f) | 0x80;
	
}

void NetCmdExe(void)
{
	boardinfop boardif = &localboard;
	unsigned int ret, cmdcnt;
	unsigned short int cnt = 0;
	unsigned char buff[256];
	unsigned char crc;
	msgheadp msgp;
	serial10Gcmdp ccp;
	unsigned char id;
	while ((ut1fp->head != ut1fp->tail) && (ret =  Fifo_Read(ut1fp, (unsigned char *)&cnt, sizeof(cnt))))
	{
		ret = Fifo_Read(ut1fp, buff, cnt);
		if ((buff[0] == NMSGHLD) && (buff[cnt - 1] == NMSGEND))
		{
			msgp = (msgheadp)&buff[0];
			if ((msgp->dst & 0x07) == boardif->mcutype)
			{
				ccp = (serial10Gcmdp)(&msgp->len + 1);
				cmdcnt = cnt - sizeof(msghead);
//				ccp->head.len =  MsgLenGet(ccp->head.len);
				if ((cmdcnt - sizeof(cmdhead)) != MsgLenGet(ccp->head.len))
				{
					return;
				}
				crc = CrcCreat(&ccp->head.boxid, (cmdcnt - 3));

#if 0
				buff[cnt - 2] = crc;
#else
				if (buff[cnt - 2] != crc)
				{
					return;
				}
#endif

				switch (msgp->src)
				{
					case BOX_CPU:
						CpuMsgExe((unsigned char *)msgp, cnt);
						break;
					case LOCAL_FPGA:
						break;
					case LOCAL_MCU:
						LocalMcuExe((unsigned char *)msgp, cnt);
						break;
					case REMOTE_FPGA:
						break;
					case REMOTE_MCU:
						McuMsgExe((unsigned char *)msgp, cnt);
						break;
				}
			}
		}
	}	
}


void Com1Exe(void)
{
	static unsigned int znet_cnt  = 0;
	static unsigned int ss_cnt  = 0;
	
	static unsigned char l_sdi_status_old = 0x00;
	static unsigned char l_sdi_status_new = 0x00;
	static unsigned char l_sdi_status_r = 0x00;
	static unsigned char k_signal_old = 0x80;
	static unsigned char k_signal_new = 0x80;
	static unsigned char k_signal_r = 0x00;
	static unsigned char temperature_old = 0x00;
	static unsigned char temperature_new = 0x00;
	static unsigned char temperature_r = 0x00;
	
	static unsigned char read_net_flag = RSTART;
	static unsigned char read_net_flag_t = REND;
	
	static unsigned short int adcx;
	static float temp;
	static float temperate;
	
	//serial10Gcmdp ccp;
	//serial10Gcmd ccpl;
	//serial10Gcmd *ccp=&ccpl;
	
	static netcmd netst1;
	static netcmd *netst=&netst1;
    
    FPGA_BOOL state;
	
	//cmdend endp1;
	//cmdend *endp=&endp1;
	
	//cmdgmac gmac1;
	//cmdgmac *gmac = &gmac1;
	
	//boardinfop boardifp = &localboard;
	//if(read_devid_flag == RSTART){
	//	At24_Sequential_Read(I2CDEV_0, E2DEVADD0, BOARDINFOADDR, (unsigned char *)boardifp, sizeof(boardinfo));
	//	memcpy(v_id,(unsigned char *)&(boardifp->remoteid),4);
	//	read_devid_flag = REND;
	//}
	//
    
    fpga_get_sdi_status(1, &state);
    fpga_get_opt_lock(1,  &state);
    fpga_get_k_signal(&state);

	ss_cnt++;
	if(ss_cnt == 100){znet_cnt++;ss_cnt = 0;}
	
	if(znet_cnt == 55){
		SWI2cSequentialRead(I2CDEV_0, FPGAADD1, 0xf1, (unsigned char *)&l_sdi_status_r, 1);
		l_sdi_status_r &= 0x01;
		l_sdi_status_r <<= 5;
		l_sdi_status_new &= 0xdf;
		l_sdi_status_new = l_sdi_status_new | l_sdi_status_r;
		
		SWI2cSequentialRead(I2CDEV_0, FPGAADD1, 0x00, (unsigned char *)&l_sdi_status_r, 1);
		l_sdi_status_r &= 0x80;
		l_sdi_status_new &= 0x7f;
		l_sdi_status_new = l_sdi_status_new | l_sdi_status_r;
		
		
		SWI2cSequentialRead(I2CDEV_0, FPGAADD1, 0xfa, (unsigned char *)&k_signal_r, 1);
		k_signal_r &= 0x80;
		k_signal_new &= 0x7f;
		k_signal_new = k_signal_new | k_signal_r;
		
		adcx = Get_Adc_Average(ADC_Channel_16,10);
		temp = (float)adcx * (3.3/4096);
		temperate = temp;
		temperate = (temperate - 0.76)/0.0025+25;
		temperature_r = (unsigned char)temperate;
		if(temperature_r < 43){temperature_new = 0x00;}
		else if((temperature_r >= 43) && (temperature_r < 48)){temperature_new = 0x10;}
		else if((temperature_r >= 48) && (temperature_r < 53)){temperature_new = 0x20;}
		else if((temperature_r >= 53) && (temperature_r < 58)){temperature_new = 0x30;}
		else if((temperature_r >= 58) && (temperature_r < 63)){temperature_new = 0x40;}
		else if((temperature_r >= 63) && (temperature_r < 68)){temperature_new = 0x50;}
		else if((temperature_r >= 68) && (temperature_r < 73)){temperature_new = 0x60;}
		else if((temperature_r >= 73) && (temperature_r < 78)){temperature_new = 0x70;}
		else if((temperature_r >= 78) && (temperature_r < 83)){temperature_new = 0x80;}
		else if((temperature_r >= 83) && (temperature_r < 88)){temperature_new = 0x90;}
		else if((temperature_r >= 88)){temperature_new = 0x90;}
		
		l_sdi_status_new &= 0xa0;
		k_signal_new     &= 0x80;
		if(l_sdi_status_new != l_sdi_status_old){
			l_sdi_status_old = l_sdi_status_new;
			read_net_flag = RSTART;
		}
		if(k_signal_new != k_signal_old){
			k_signal_old = k_signal_new;
			read_net_flag = RSTART;
		}
		if(temperature_new != temperature_old){
				temperature_old = temperature_new;
				read_net_flag = RSTART;
		}
		
		if(read_net_flag == RSTART){	
			//netst = (netcmdp)(&ccp->subcmd+1);
			
			netst->head = 0x80;
			//netst->card_coding = 0x01020304;
			memcpy(netst->card_coding,card1coding,8);
			//netst->card_name_length = 0x13;
			//netst->card_name = card1name;//////////////////
			//memcpy(netst->card_name,card1name,19);
			//0x47 56 33 53 2D 48 4F 4E 55 2D 51 4D 5F 56 31 2E 30 30
			netst->card_name_length = sizeof(BOARD_NAME) - 1;
			memcpy(netst->card_name, BOARD_NAME, sizeof(BOARD_NAME) - 1);
			
			netst->light_num = 0x01;
			netst->elec_num = 0x01;
			netst->datachannum = 0x01;
			netst->ethnum = 0x08;
			netst->temperature = temperature_old;
			netst->voltage = 0x55;
			netst->l_sdi_status = l_sdi_status_old;
			netst->k_signal = k_signal_old;
			read_net_flag = REND;
			read_net_flag_t = RSTART;
		}
	}
	
	
		
	
}

#define COMPUTC(buf, size)	Uart2Pollputc(buf, size)


void ComCmdExe(void)
{
	unsigned char buff[64];
	uart2cmdp utccp;
	int ret;
	unsigned short int cnt = 0;
	unsigned char  debugen;

	while(ret = HalQueueRead((int8_t *)buff, 0))
	{
		utccp = (uart2cmdp)buff;
		ret -= ((&utccp->subcmd - &utccp->head) - 1);
		switch (utccp->subcmd)
		{
#if 0
			case MACINFO:
				MacInfoProcess(utccp->port, &utccp->rev, &ret, utccp->cmd);
				break;
			case CHANMASK:
				ChanMaskProcess(utccp->port, &utccp->rev, &ret, utccp->cmd);
				break;
			case MACMASK:
//				MacMaskProcess(utccp->port, &utccp->rev, &ret, utccp->cmd);
				break;
			case MODBOARD:
				ModModeProcess(utccp->port, &utccp->rev, &ret, utccp->cmd);
				break;
			case COLORMODE:
//				ColorModeProcess(utccp->port, &utccp->subcmd + 1, &ret, utccp->cmd);
				break;
			case COLORCHAN:
//				ColorChanProcess(utccp->port, &utccp->subcmd + 1, &ret, utccp->cmd);
			case  ERRRATE:
				ErrRateProcess(utccp->port, &utccp->rev, &ret, utccp->cmd);
				break;
			case STATUS:
				StatusProcess(utccp->port, &utccp->rev, &ret, utccp->cmd);
				break;
			case DEFAULTSET:
				FpgaDefaultProcess(utccp->port, &utccp->rev, &ret, utccp->cmd);
				break;
			case RMTIDSET:
				RMTIdInfoProcess(utccp->port, &utccp->rev, &ret, utccp->cmd);
				break;

			case FPGAREG:
				FpgaRegProcess(utccp->port, &utccp->rev, &ret, utccp->cmd);
				break;
			case PHYREG:{
				unsigned char port;
				port = (utccp->port) ? PHYCHAN2 : PHYCHAN1;
				PhyRegProcess(port, &utccp->rev, &ret, utccp->cmd);
				break;
			}
#endif
			case BOARDRMTSET:
				BoardInfoProcess(utccp->port, &utccp->subcmd + 1, &ret, utccp->cmd);
				break;
			default:
				break;
		}
		if (utccp->cmd)
		{
			utccp->cmd = CMDREQ;
			*(&utccp->len + ret + 2) = CMSGEND;
			utccp->len = ret + 2;
			HalWrite((int8_t *)utccp, ret + sizeof(uart2cmd));
		}
	}
}




void FpgaRegSet(void)
{
	fpgainfo macif;
	unsigned char id;
#if (BOARDMCU == LOCAL_MCU)
	IdGet(&id);
	BoardInfoGet();
	localboard.boxid = (id >> 5) | 0x80;
	localboard.boardid = (id & 0x1f) | 0x80;
	BoardInfoSet();
	At24_Byte_Write(I2CDEV_0, E2DEVADD0, FPGAINFOADD + (&macif.boxboardid - macif.lclmac), &id);
	SWI2cRandomWrite(I2CDEV_0, FPGAADD1, LCLMACADD + (&macif.boxboardid - macif.lclmac), &id);
#elif (BOARDMCU == REMOTE_MCU)
	FpgaOsdInit();
#endif
	/* config first channel mac */
	memset(&macif, 0, sizeof(macif));
	Delay_5Us(300);
	At24_Sequential_Read(I2CDEV_0, E2DEVADD0, FPGAINFOADD, (unsigned char *)&macif, sizeof(macif));
	if (macif.macset != 0xa5a5)
	{
		memcpy(&macif, &defaultfpgainfo, sizeof(macif));
		macif.macset = 0xa5a5;
		At24_Page_Write(I2CDEV_0, E2DEVADD0, FPGAINFOADD, (unsigned char *)&macif, sizeof(macif));
	}
	SWI2cSequentialWrite(I2CDEV_0, FPGAADD1, FPGABACE, (unsigned char *)&macif, 1 + (unsigned char *)(&macif.confmode) - (unsigned char *)&macif);
	memset(&macif, 0, sizeof(macif));
	SWI2cSequentialRead(I2CDEV_0, FPGAADD1, FPGABACE, (unsigned char *)&macif, sizeof(macif));
#if (BOARDMCU == LOCAL_MCU)
	/* config second channel mac */
	memset(&macif, 0, sizeof(macif));
	At24_Sequential_Read(I2CDEV_0, E2DEVADD0, FPGAINFOADD + sizeof(macif), (unsigned char *)&macif, sizeof(macif));
	if (macif.macset != 0xa5a5)
	{
		/* set mac and ctlmode */
		memcpy(&macif, &defaultfpgainfo, sizeof(macif));
		macif.macset = 0xa5a5;
		At24_Page_Write(I2CDEV_0, E2DEVADD0, FPGAINFOADD1, (unsigned char *)&macif, sizeof(macif));
	}
	SWI2cSequentialWrite(I2CDEV_0, FPGAADD1, FPGABACE + FPGABANKSIZE, (unsigned char *)&macif, 1 + (unsigned char *)(&macif.confmode) - (unsigned char *)&macif);
//	SWI2cRandomWrite(I2CDEV_0, FPGAADD1, FPGABACE + FPGABANKSIZE + (&macif.confmode - macif.lclmac), &id);
//	id = 0;
//	SWI2cRandomRead(I2CDEV_0, FPGAADD1, FPGABACE + FPGABANKSIZE + (&macif.confmode - macif.lclmac), &id);
	memset(&macif, 0, sizeof(macif));
	SWI2cSequentialRead(I2CDEV_0, FPGAADD1, FPGABACE + FPGABANKSIZE, (unsigned char *)&macif, sizeof(macif));
#endif
}


/*
void PhyDet(unsigned int timeblink)
{
	static unsigned int otime, mode;
	static int errcnt[2];
	unsigned int ctime, status[2];
	unsigned char optstatus[2];
	unsigned int exsit;
	unsigned char isauto = 0;

	unsigned char buff[32];
	unsigned char temp;
	int speed,speed1;


	SysTickGet(&ctime);
	U32TIMEOUT(ctime, otime);
	if(ctime >= timeblink){
		SysTickGet(&otime);
		speed = SWI2cSpeedGet(I2CDEV_0);
		speed1 = SPEED_200K;
		SWI2cSpeedSet(I2CDEV_0,speed1);
		temp = 0x01;
		SWI2cCurrentWrite(I2CDEV_0, I2CSWITCHADD, &temp); //select i2c switch 1 chan
		if((SWI2cRandomRead(I2CDEV_0, OPTE2PADD01, 0x02, &optstatus[0]))== I2C_ACK_OK){
			status[0] = ChanHsStaRead(PHYCH1);
			if((status[0] & 0x5c03) != 0x5c03){
				errcnt[0]++;
				if(errcnt[0] >= 10){
					SWI2cRandomRead(I2CDEV_0, FPGAADD1, 0x1c, &isauto);
					MannualModeConf(PHYCH1, (isauto & ISATMASK1), ComCmdExe);
					errcnt[0] = 0;
				}
			}
			else{
				errcnt[0] = errcnt[0] ? (errcnt[0] - 1) : 0;
			}
		}
		else{
			errcnt[0] = 0;
		}
		temp = 0x02;
		SWI2cCurrentWrite(I2CDEV_0, I2CSWITCHADD, &temp); //select i2c switch 2 chan
		if((SWI2cRandomRead(I2CDEV_0, OPTE2PADD01, 0x02, &optstatus[1])) == I2C_ACK_OK){
			status[1] = ChanHsStaRead(PHYCH2);
			if((status[1] & 0x5c03) != 0x5c03){
				errcnt[1]++;
				if(errcnt[1] >= 3){
					SWI2cRandomRead(I2CDEV_0, FPGAADD1, 0x1c, &isauto);
					MannualModeConf(PHYCH2, (isauto & ISATMASK2), ComCmdExe);
					errcnt[1] = 0;
				}
			}
			else{
				errcnt[1] = errcnt[1] ? (errcnt[1] - 1) : 0;
			}
		}
		else{
			errcnt[1] = 0;
		}
		temp = 0;
		SWI2cCurrentWrite(I2CDEV_0, I2CSWITCHADD, &temp); //close i2c bus switch
		SWI2cSpeedSet(I2CDEV_0,speed);
//		sprintf(buff, "ChanStu(%#x),Errcnt(%d)\r\n",ret, errcnt);
//		Uart2Pollputc(buff, strlen(buff));
	}
}
*/



void  RST_PCS_XAUI(unsigned char port)
{
	unsigned char temp1[2];
	unsigned char offset;
	offset = (port) ? 0x16 : 0x16 + FPGABANKSIZE;
	SWI2cRandomRead(I2CDEV_0, FPGAADD1, offset, temp1);	//复位 PCS
	temp1[1] = temp1[0] | 0x30;
	SWI2cRandomWrite(I2CDEV_0, FPGAADD1, offset, &temp1[1]);
	SWI2cRandomRead(I2CDEV_0, FPGAADD1, offset, &temp1[1]);
	Delay_Ms(300);
	temp1[1] = temp1[0] & 0xcf;
	SWI2cRandomWrite(I2CDEV_0, FPGAADD1, offset, &temp1[1]);
	SWI2cRandomRead(I2CDEV_0, FPGAADD1, offset, &temp1[1]);
}


#if 0
void FpgaMacLogicRst(unsigned char port, unsigned char mode)
{
	unsigned char offset;
    unsigned char tmp;

	offset = offset * FPGABANKSIZE + 0x16;
	
	SWI2cRandomRead(I2CDEV_0, FPGAADD1, offset, &tmp);
	tmp = (mode) ? (tmp | 0x80) : (tmp & 0x7f);
	SWI2cRandomWrite(I2CDEV_0, FPGAADD1, offset, &tmp);
}
#endif

void SYNC_XAUI(unsigned char port)
{
	unsigned char temp1[2];
	unsigned char offset;
	offset = (port) ? 0x16 : 0x16 + FPGABANKSIZE;
	SWI2cRandomRead(I2CDEV_0, FPGAADD1, offset, temp1);   //Xaui 强制同步
	temp1[1] = temp1[0] | 0x40;
	SWI2cRandomWrite(I2CDEV_0, FPGAADD1, offset, &temp1[1]);
	Delay_Ms(500);
	temp1[1] = temp1[0] & 0xbf;
	SWI2cRandomWrite(I2CDEV_0, FPGAADD1, offset, &temp1[1]);
}


void Check_LS_ALIGN(unsigned char port)
{
	unsigned char i, temp1[10];
	unsigned char flag;
	unsigned char offset;
	unsigned short int chnstatus1;
	offset = (port) ? 0x1c : 0x1c + FPGABANKSIZE;
	while (1)
	{
		for (i = 0; i < 10; i++)
		{
			SWI2cRandomRead(I2CDEV_0, FPGAADD1, offset, &temp1[i]); //FPGA 4 通道对其
			Delay_Ms(2);
		}
		for (i = 0; i < 9; i++)
		{
			if ((temp1[i] & 0x10) != (temp1[i + 1] & 0x10))
			{
				break;
			}
		}
		if (i == 9)
		{
			flag = 1;
			break;
		}
		else
		{
			flag = 0 ;
		}
	}
	chnstatus1 = TlkValueRead(0x0000, 0x1e, 0x0f);
	chnstatus1 = TlkValueRead(0x0000, 0x1e, 0x0f);
	while (((chnstatus1 & 0x5C03) != 0x5C03) || !(temp1[1] & 0x10))
	{
		RST_PCS_XAUI(port);
		Delay_Ms(200);
		SYNC_XAUI(port);
		Delay_Ms(2000);
		while (1)
		{
			for (i = 0; i < 10; i++)
			{
				SWI2cRandomRead(I2CDEV_0, FPGAADD1, offset, &temp1[i]); //FPGA 4 通道对其
				Delay_Ms(2);
			}
			for (i = 0; i < 9; i++)
			{
				if ((temp1[i] & 0x10) != (temp1[i + 1] & 0x10))
				{
					break;
				}
			}
			if (i == 9)
			{
				flag = 1;
				break;
			}
			else
			{
				flag = 0 ;
			}
		}
	}
}


#define MAXERR	2
typedef enum
{
    SRST_PCS,
    SSYSC_XAUI,
    SRST_PATH,
    SRST_READ,
    SRST_CHK
} rststatus;

typedef struct {
	unsigned int	orxerr;
	unsigned int	crxerr;
	unsigned int	currenttime;
	unsigned int	historytime;
} checkerr;

typedef struct
{
	unsigned int	chan;
	unsigned char	checkcnt;
	unsigned char	errcnt;
	unsigned char	isexsit;
	unsigned char	optstatus;
	rststatus		chk_status;
	unsigned int	timeout;
	checkerr		cerr;
} chanrst;



void updateto(unsigned int *time, int to)
{
	*time = (*time >= to) ? *time - to : 0;
}

#if 0
void PhyDet(unsigned int *timeblink)
{
	static unsigned int otime, val;
	unsigned int ctime, to;
	static chanrst rst[2] = {{PHYCHAN1, 0, 0, 0, 0, SRST_CHK, LLBLINK}, {PHYCHAN2, 0, 0, 0, 0, SRST_CHK, LLBLINK}};
	chanrst *rstp = NULL;
	unsigned int i, offset;
	rmtidp rmtidinfop = NULL;
	
	rstp = rst;
	
	SysTickGet(&ctime);
	to = ctime;
	U32TIMEOUT(to, otime);
	otime = ctime;
	for (i = 0; i < 2; i++) {
		updateto(&rstp->timeout, to);
		if (rstp->timeout == 0) {
			switch (rstp->chk_status) {
				case SRST_CHK:
					offset = 0x1c + i * FPGABANKSIZE;
					SWI2cRandomRead(I2CDEV_0, FPGAADD1, offset, &rstp->optstatus);
					if (!(rstp->optstatus & OPSTATUSMSK)) {
						rstp->checkcnt++;
						if (!(rstp->optstatus & ALIGNMASK)) {
							rstp->errcnt++;
						}
						else {
							if (rstp->errcnt) {
								rstp->errcnt--;
							}
							rstp->isexsit = 1;
						}
						if (rstp->errcnt) {
							RST_PCS_XAUI(rstp->chan);
							rstp->chk_status = SSYSC_XAUI;
							rstp->timeout = 0;
							rstp->isexsit = 0;
						}
						else {
							unsigned int tmperr,tmptime;
							//send loacl baoard id
							rstp->timeout = LLBLINK;

							FpgaMacLogicRst(rstp->chan, 0);
							SysTickGet(&rstp->cerr.currenttime);
							ErrRateProcess((rstp->chan ? 0 : 1), (unsigned char *)&rstp->cerr.crxerr, (int *)&tmperr, CMDGET);
							rstp->cerr.crxerr = SWP32(rstp->cerr.crxerr) >> 8;
							tmperr = (rstp->cerr.crxerr < rstp->cerr.orxerr) ? \
									 (rstp->cerr.crxerr + 0x01000000 - rstp->cerr.orxerr) : \
									 (rstp->cerr.crxerr - rstp->cerr.orxerr);
							tmptime = rstp->cerr.currenttime;
							U32TIMEOUT(tmptime, rstp->cerr.historytime);
							rstp->cerr.historytime = rstp->cerr.currenttime;
							rstp->cerr.orxerr = rstp->cerr.crxerr;
							if((tmperr * 1000 / tmptime) > 256){
								rstp->chk_status = SSYSC_XAUI;
								rstp->timeout = 0;
								rstp->isexsit = 0;
							}
							
							
						}
					}
					else {
						rstp->checkcnt = 0;
						rstp->errcnt = 0;
						rstp->chk_status = SRST_CHK;
						rstp->timeout = LLBLINK;
						rstp->isexsit = 1;
					}
					break;
				case SSYSC_XAUI:
					FpgaMacLogicRst(rstp->chan, 1);
					SYNC_XAUI(rstp->chan);
					rstp->chk_status = SRST_PATH;
					rstp->timeout = LBLINK;
					break;
				case SRST_PATH:
					val = TlkValueRead(rstp->chan, 0x1e, 0x000e);
					val |= 0x0008;
					TlkValueWrite(rstp->chan, 0x1e, 0x000e, val);
					rstp->chk_status = SRST_READ;
					rstp->timeout = LBLINK;
					break;
				case SRST_READ:
					Read_status(rstp->chan);
					rstp->checkcnt = 0;
					rstp->errcnt = 0;
					rstp->chk_status = SRST_CHK;
					rstp->timeout = LBLINK * 8;
					break;
			}
		}
		rstp++;
	}
	rstp = rst;
	if ((rstp->isexsit == 1) && ((rstp + 1)->isexsit == 1)) {

	}
	else {

	}
	rmtidinfop = rmtidinfo;
	for (i = 0; i < RMTID_MAXNUM; i++) {
		updateto(&rmtidinfop->timeout, to);
		if ((rmtidinfop->isexist) && (rmtidinfop->timeout == 0)) {
			rmtidinfop->isexist = IDINIT;
		}
		rmtidinfop++;
	}
}
#endif

void An_test(void)
{
	static unsigned char test, pretest = 0xff;
	unsigned short int val;
	SWI2cRandomRead(I2CDEV_0, FPGAADD1, 0x1c, &test);
	if ((test & ISATMASK1) != (pretest & ISATMASK1))
	{
		if (test & ISATMASK1)
		{
//			val = TlkValueRead(PHYCHAN1, 0x1e, 0x0003);
			/* disanble the entx */
//			val &= ~0x0008;
//			TlkValueWrite(PHYCHAN1, 0x1e, 0x0003, val);
			TlkValueWrite(PHYCHAN1, 0x07, 0x0000, 0x2000);    //disable Auto negotiation
		}
		else
		{
//			val = TlkValueRead(PHYCHAN1, 0x1e, 0x0003);
			/* disanble the entx */
//			val |= 0x0008;
//			TlkValueWrite(PHYCHAN1, 0x1e, 0x0003, val);
			TlkValueWrite(PHYCHAN1, 0x07, 0x0000, 0x3000);    //disable Auto negotiation
		}
	}
	if ((test & ISATMASK2) != (pretest & ISATMASK2))
	{
		if (test & ISATMASK1)
		{
			val = TlkValueRead(PHYCHAN2, 0x1e, 0x0003);
			/* disanble the entx */
			val &= ~0x0008;
			TlkValueWrite(PHYCHAN2, 0x1e, 0x0003, val);
		}
		else
		{
			val = TlkValueRead(PHYCHAN2, 0x1e, 0x0003);
			/* disanble the entx */
			val |= 0x0800;
			TlkValueWrite(PHYCHAN2, 0x1e, 0x0003, val);
		}
	}
	pretest = test;
}


void FpgaStatusGet(unsigned char fpgaadd, unsigned char   *status, unsigned char off)
{
	fpgainfo fpgaif;
	unsigned int offset;
	offset = FPGAINFOADD + (unsigned char *)&fpgaif.status - (unsigned char *)&fpgaif + off;
	SWI2cSequentialRead(I2CDEV_0, fpgaadd, offset, status, sizeof(fpgaif.status));
}




//end file
