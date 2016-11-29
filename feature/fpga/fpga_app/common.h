

#ifndef _COMMON_H
#define _COMMON_H

#include "stm32f2xx.h"
#include "stm32f2xx_flash.h"
#include "i2c_ee.h"
#include "tlk10232.h"
#include "upgrade.h"


/* loacl board info */
#ifndef BOARDMCU
#define	BOARDMCU REMOTE_MCU
#endif


#define BOARDCASTID	0xe5e5e5e5


/** 
  * @brief I2C's device Address
  */
#define	SI5324ADD1		0xd0
#define	SI5324ADD2		0xd2
#define	SI5324ADD3		0xd4
#define	SI5324ADD4		0xd6
#define	SI5324ADD5		0xd8


#define	FPGAADD1		0x80
#define FPGAADD2		0x82		//u67
#define FPGAADD3		0x84		//u69
#define	LCLMACADD		0x00
#define	SRCMACADD		0x06
#define	RMTMACADD		0x0c

#define	I2CSWITCHADD	0xe0
#define	OPTE2PADD01		0xa0
#define	OPTE2PADD02		0xa2


/** 
  * @brief LED Link Delay Time Size
  */
#define	FBLINK	100
#define	MBLINK	500
#define	LBLINK	1000
#define	LLBLINK	2000

/*Fpga reg address*/
typedef enum
{
	OPTINFOADD0 = 0x00,
	OPTINFOADD1,
	OPTINFOADD2,
	OPTINFOADD3,

	FCODEIDADD0 = 0xfb,
	FCODEIDADD1,
	FCODEIDADD2,
	FCODEIDADD3,
	SWIDADD
} fregadd;



/** 
  * @brief Msg From or to Type
  */
typedef enum
{
    BOX_CPU = 0,
    LOCAL_FPGA,
    LOCAL_MCU,
    REMOTE_FPGA,
    REMOTE_MCU,
    LOLRMT_FPGA,
    LOLRMT_MCU
} msgtype;

/** 
  * @brief Cmd mask for Local or remote
  */
typedef enum
{
    LOCALOPMASK = 0x00,
    REMOTEOPMASK = 0x01,
    LOLRMTOPMASK = 0x02,
    ALLOPMASK = 0x03
} dstmask;


/** 
  * @brief Old Cmd Type
  */
typedef enum
{
    GET_MATRIXSTA = 0x01,
    GET_LOCALSTA,
    GET_REMOTESTA,
    GET_MATRIXPOW,
    GET_MATRIXCFG,
    GET_MATRIXCLK,
    GET_MATRIXPORT,
    GET_MATRIXRMTID,

    SET_LCLOSD	= 0x11,
    SET_RMTOSD,
    SET_ALLOSD,

    GET_MATRIXLPFAIL = 0x14,

    GET_CLASS = 0x18,
    SET_CLASS = 0x19,

    BIGSCEEN_CLASS = 0x20,
    RMTUPGRADE_CLASS = 0x21,

} oldcmd_e;

/** 
  * @brief New Cmd Type
  */
typedef enum
{
/*!< operat code 1 >*/
	CMDSET	= 0,
	CMDGET,
	CMDREQ,
	CMDRMTSET,
	CMDRMTGET,
	CMDRMTREQ,
/*!< operat code 1 >*/
	MACINFO = 0,
	CHANMASK,
	MACMASK,
	MODBOARD,
	COLORMODE,
	COLORCHAN,
	ERRRATE,
	STATUS,
	DEFAULTSET,
	BOARDRMTSET,
	CMDREV,
	OSDOP,
	RMTIDSET,
	
#if (defined PHY_TEST)
	OPTLINKTEST,
#endif
	RMTIDUP = 0x0f,
	MCODEUPGRADE,
	CODEIDSRL,

	FPGAREG = 0x12,
	PHYREG,
	ERRMAX,
	IMGINIT,
	SYSRESET

} newcmd_e;

/** 
  * @brief Cpu To Fpga or Mcu Msg Head
  */
typedef struct
{
	unsigned char	cmdhead;
	unsigned char	boxid;
	unsigned char	boardid;
	unsigned char sflag;
	unsigned short int len;
} cmdhead, *cmdheadp;

/** 
  * @brief Msg End
  */
typedef struct cmdend
{
	unsigned char crc;
	unsigned char end;
} cmdend, *cmdendp;

/** 
  * @brief Uart2's Msg head
  */
typedef struct
{
	unsigned char head;
	unsigned char cmd;
	unsigned char port;
	unsigned char len;
	unsigned char subcmd;
	unsigned char rev;
} uart2cmd, *uart2cmdp;

/** 
  * @brief Cpu To Fpga or Mcu Msg Handle
  */
typedef struct
{
	cmdhead	head;
	unsigned char cmd;
	unsigned char mask;					//0 local, 12 remote
	unsigned int  remoteid;
	unsigned char slecport;
	unsigned char port;
	unsigned char chan;
	unsigned char subcmd;
} serial10Gcmd, *serial10Gcmdp;

/** 
  * @brief Cpu To Fpga or Mcu Msg Handle
  */
typedef struct
{
	unsigned char head;
	unsigned char card_coding[8];				
	unsigned char  card_name_length;
	unsigned char card_name[19];
	unsigned char light_num;
	unsigned char elec_num;
	unsigned char datachannum;
	unsigned char ethnum;
	unsigned char temperature;
	unsigned char voltage;
	unsigned char l_sdi_status;
	unsigned char k_signal;
} netcmd, *netcmdp;

/** 
  * @brief Fpga From or To Mcu Msg Head
  */
typedef struct MSG
{
	unsigned char	head;
	unsigned char	dst;
	unsigned char	src;
	unsigned char	len;
//	unsigned char *cmdp;
} msghead, *msgheadp;

/*Reset sub command */
typedef enum {
	MCURESET = 0x00,
	FPGARESET,
	BOARDRESET,
	BOARDRESTART,
	MCUBACKUP,
	FPGABACKUP,
	BOARDBACKUP
} sysresetop;





/*------------------------------------------------*
 *|               eeprom map                     |* 
 *------------------------------------------------*
 *|   board name: Var3s_10Golt_V1.10             |*
 *|               Var3s_10Golt_R_V1.10           |*
 *------------------------------------------------*
 *|   e2prom size = AT24C64(64Kbit)              |* 
 *------------------------------------------------*
 *| remap1(0x00 - 0x200,512byte) for system info |* 
 *------------------------------------------------*
 *| info name  | start address |    info size    |*
 *------------------------------------------------*
 *| board info | BOARDINFOADDR | BOARDINFOSIZE   |*
 *------------------------------------------------*
 *| fpga info1 | FPGAINFOADD0  | FPGAINFOSIZE    |*
 *------------------------------------------------*
 *| fpga info2 | FPGAINFOADD2  | FPGAINFOSIZE    |*
 *------------------------------------------------*
 *| fpga info2 | UPGRADEINFOADD| E4UPGRADESIZE   |*
 *|            |     (0x1c0)   |                 |*
 *------------------------------------------------*
 *| remap1(0x200 - end,512byte) for osd info     |*
 *------------------------------------------------*
 *| osd info   | OSDSIZE       | FPGAINFOSIZE    |*
 *------------------------------------------------*
 */


/* eeprom remap store for system info */
#define EEPROMSTART 0x0000
#define	EEPROMSIZE	(AT24C64 + 1)
/* e2prom user size = 512 byte */
#define	SYSIFSTRSIZE 0x200

/* eeprom area  */
#define	BOARDINFOADDR		EEPROMSTART
typedef struct {
	unsigned int	devid;			//4byte
	unsigned char	boxid;			//1byte
	unsigned char	boardid;		//1byte
	unsigned int	remoteid;		//4byte
	unsigned char	mcutype;		//1byte
	unsigned char	fpgatype;		//1byte
	unsigned char	boardname[28];	//28byte
} boardinfo, *boardinfop;
#define	BOARDINFOSIZE	sizeof(boardinfo)		//total 40byte

/*Fpga imagename head*/
#define FIMAGENAME "VAR3S.X.XX.XXX.XX"


#if (defined SERIES_10G)

typedef struct {
	unsigned char		version;
	unsigned char		errcnt3;
	unsigned char		errcnt2;
	unsigned char		errcnt1;
	unsigned char		fpgadial;
	unsigned char		recvlock;
	unsigned char		sendlock;
	unsigned char		subversion;
} statusreg;

typedef struct {
	unsigned char		lclmac[6];
	unsigned char		srcmac[6];
	unsigned char		rmtmac[6];
	unsigned char		chansw1;
	unsigned char		chansw2;
	unsigned char		confmode;
	unsigned char		boxboardid;
	unsigned char		colorbar1;
	unsigned char		colorbar2;
	statusreg		status;
	unsigned short int	macset;
} fpgainfo, *fpgainfop;

#define FPGAINFOSIZE		sizeof(fpgainfo)


#define ISATMASK1	0x01
#define ISATMASK2	0x02
#define ALIGNMASK	0x10
#define OPSTATUSMSK	0x20

#define	FPGAINFOADD		(BOARDINFOADDR + BOARDINFOSIZE)
#define FPGAINFOADD0	FPGAINFOADD
#define FPGAINFOADD1	(FPGAINFOADD + FPGAINFOSIZE)
#define FPGABACE		0x00
#define FPGABANKSIZE	0x20
#define	PHYCHAN1		PHYCH2
#define	PHYCHAN2		PHYCH1
#endif


#define E4UPGRADESIZE	0x20
#define UPGRADEINFOADD	0x1C0
#define UPGRADEINFOADD3	0x1E80

#define	UPGRADEINFOSIZE	sizeof(uginfo)
#define ISSTORESIZE(strsize,realsize) ((strsize) >= (realsize))



#if (defined VAR3S_10GOLT_V1_10 || (defined VAR3S_10GOLT_R_V1_10))
#define MAXOPPORTNUM 2
#endif


#if (defined PHY_TEST)
#define	PHYREGADDR1		FPGAINFOADD1 + sizeof(fpgainfo)
#define	PHYREGADDR2		(PHYREGADDR1 + 6)
#endif


#ifndef RMTID_MAXNUM
#define	RMTID_MAXNUM		2
#endif

typedef struct RMTIDINFO
{
	unsigned char isexist;
	unsigned char level;
	unsigned char clientport;
	unsigned char rev;
	unsigned int	rmtid;
	unsigned int	timeout;
	struct RMTIDINFO *next;
} rmtid, *rmtidp;

/** 
  * @brief Chip Device info
  */
typedef struct {
	unsigned short int d_flashsize;		/*!< flash size = xx Kbyte >*/
	unsigned short int d_id0;
	unsigned short int d_id1;
	unsigned int d_id2;
	unsigned int d_id3;
}device_t,*device_p;

/* 网络字节序与主机字节序转换 */
#define SWP32(a)		((unsigned int)((a >> 24) | ((a & 0x00ff0000) >> 8) | \
                        ((a & 0x0000ff00) << 8) | (a << 24)))
#define SWP16(a)		((unsigned short int)(a >> 8) | (a << 8))




#define CONV32(a)	    ((unsigned int) (((a & 0x7f000000) >> 3) | ((a & 0x007f0000 ) >> 2) |\
					    ((a & 0x00007f00) >>  1) | (a & 0x0000007f)))
#define CONV16_14(a)	((unsigned short int)(((a & 0x7f00) >>  1) | (a & 0x007f)))
#define CONV14_16(a)	((unsigned short int)(((a & 0x3f80) << 1) | (a & 0x007f)))

#define HLCONV8(a)	    ((unsigned char)(a>>7) | (a<<7) | ((a&0x40)>>5) |((a&0x02)<<5) | \
					    ((a&0x20)>>3) | ((a&0x04)<<3) | ((a&0x10)>>1) |((a&0x08)<<1))


#define GetCodeId(id)	(unsigned int)((id << 24) | ((id & 0x0000ff00) << 8) | \
                        ((id & 0x00ff0000) >> 8) | (id >> 24))
#define GetFpgaId(id)	(unsigned int)((id << 24) | ((id & 0xf0000000) >> 24)|\
						((id & 0x0f000000) >> 12) | ((id & 0x00f00000) >> 20) |\
						((id & 0x000f0000) << 4 ) | ((id & 0x0000f000) >> 4 ) |\
						((id & 0x00000f00) << 8))







typedef int (*taskfun)(int argc, char **argv);

typedef struct LIST_HEAD
{
	struct LIST_HEAD *next, *prev;
} list_head, *list_head_p;

typedef struct INIT_FUNC
{
	list_head	list;
	taskfun		task;
} initfunc, *initfuncp;


/* Exported types ------------------------------------------------------------*/
typedef  void (*pFunction)(void);

//#define	CTRLC	0x03

/* Constants used by Serial Command Line Mode */
#define CMD_STRING_SIZE		128


extern int bootabort;



void UartR1TimeoutSet(void);
void Ut1Recv(unsigned char ch);
void UartR2TimeoutSet(void);
void Ut2Recv(unsigned char ch);
void UartR4TimeoutSet(void);
void Ut4Recv(unsigned char ch);


void  RST_PCS_XAUI(unsigned char port);
void SYNC_XAUI(unsigned char port);
void Check_LS_ALIGN(unsigned char port);



void FpgaRegSet(void);
void NetCmdExe();
void Com1Exe();
void ComCmdExe(void);
void PhyDet(unsigned int *timeblink);
void DeviceInfoInit(void);
char* BoardInfoGet(void);
void BoardboxidGet(void);
void FImageinfoCheck(void);

#endif /* _COMMON_H */
