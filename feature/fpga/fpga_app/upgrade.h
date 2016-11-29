


#ifndef _UPGRADE_H
#define _UPGRADE_H


/*------------------------------------------------*
 *|        stm32f10x internal flash map          |* 
 *------------------------------------------------*
 *|   board name: Var3s_10Golt_V1.10             |*
 *|               Var3s_10Golt_R_V1.10           |*
 *------------------------------------------------*
 *|   falsh size = chip internal f_size          |* 
 *------------------------------------------------*
 *|remap1(0x8000000-0x8003000,12k) for boot code |* 
 *------------------------------------------------*
 *| info name  | start address |    info size    |*
 *------------------------------------------------*
 *| bootsize   | PARTITOPNAADDR| PARTITOPNASIZE  |*
 *------------------------------------------------*
 *|remap2(0x8003000-0x8009800,26k) for user code |*
 *------------------------------------------------*
 *| user1size  | PARTITOPNBADDR| PARTITOPNBSIZE    |*
 *------------------------------------------------*
 *|remap3(0x8009800-end,f_size-38k) for user code|*
 *------------------------------------------------*
 *| user2size  | PARTITOPNCADDR| PARTITOPNCSIZE    |*
 *------------------------------------------------*
 */

#define PAGE_SIZE			(0x400)    /* 1 Kbyte */
#define FPAGE_SIZE			(0x400)
#define FLASH_SIZE			(0x10000)  /* 64 KBytes */
#define SPI_FLASH_SIZE		(0x10000)


#define	INALFLASHADDRBASE	0x8000000

#define PARTITOPNAADDR	0x8000000
#define PARTITOPNBADDR	0x8003000
#define PARTITOPNCADDR	0x8009800
#define PARTITOPNBADDR_OFFSET	0x3000
#define PRIMARY_ADDRESS	0x010000
#define GOLDEN_ADDRESS	0x100000

#define PRIMARYSIZE	(GOLDEN_ADDRESS - PRIMARY_ADDRESS)


#define PARTITOPNASIZE	(PARTITOPNBADDR - PARTITOPNAADDR)
#define PARTITOPNBSIZE	(PARTITOPNCADDR - PARTITOPNBADDR)
#define PARTITOPNCSIZE	(INALFLASHADDRBASE + FLASH_SIZE - PARTITOPNCADDR)

#define ApplicationAddress   	PARTITOPNBADDR
#define BootAddress    			PARTITOPNAADDR

#define	UPGRADEAPP	0xa5a5a5a5

typedef enum {
	MINCONTENT = 128,
	MEDCONTENT = 256,
	LARCONTENT = 512
}contentsize;

typedef enum
{
    NONEUPDATE = 0,
    UPBYINFLASH,
    UPBYSPIFLASH
} upgradestore_e;

typedef enum
{
    NOMAL = 0x8800,
    UPDATING,
    UPDATEND,
	UPERR
} upgradestatus_e;

typedef enum
{
    NONEERR = 0,
	FILETOLARERR = 0x80000000,
	FNAMTOLAREERR,
    TIMEOUTERR,
    FILECRCERR,
    FLASHERASEERR,
    FLASHWRITEERR,
    FLASHREADERR
} err_e;

#if (defined BOOT_CODE && defined __DEBUG)
/* debug info */
#define NONEERRINFO	      "none err\n"
#define FILETOLARERRINFO  "file size to large\n"
#define FNAMTOLAREERRINFO "file name to long\n"
#define TIMEOUTERRINFO    "file update time out\n"
#define FILECRCERRINFO    "file crc error\n"
#define FLASHERASEERRINFO "internal flash erase error\n"
#define FLASHWRITEERRINFO "internal flash write error\n"
#define FLASHREADERRINFO  "internal falsh read error\n"
#endif

typedef enum {
	UPSTART = 0x01,
	UPCONTENT,
	UPEND,
	UPIFCFG,
	FUPSTART = 0x11,
	FUPCONTENT,
	FUPEND,
	FUPIFCFG
} upgradeop;


/** 
  * @brief Upgrade image info 
  */
typedef struct {
	unsigned int imgver;
	unsigned int imgsize;
	unsigned char imgname[32];		//maxsize = 32 byte;
} imageif, *imageifp;

/** 
  * @brief Upgrade info 
  */
typedef struct {
	unsigned short int store;
	unsigned short int status;
	unsigned short int cnt;
	unsigned short int errtype;
	unsigned int filesize;
	imageif image;
} upgradeif, *upgradeifp;


/** 
  * @brief Upgrade Process Msg 
  */
typedef struct {
	unsigned char opcode;
	unsigned char version[2];	//version[0] = device num version[1] = version info
	unsigned char size[4];
	unsigned char content;
}upgrademsg,*upgrademsgp;

/** 
  * @brief id mac get Msg 
  */
typedef struct {
	unsigned char scode;
	unsigned char remotenum;	//
	unsigned char portnum;
	unsigned char casnum;
	unsigned char devicetype[8];
	unsigned char idmacnum;
	unsigned char idnum[4];
	unsigned char mmacnum[6];
	//unsigned char smacnum[6];
}getidmacmsg,*getidmacmsgp;

/* Compute the FLASH upload image size */

#define FLASH_IMAGE_SIZE	(uint32_t) (FLASH_SIZE - (ApplicationAddress - 0x08000000))

/* Exported macro ------------------------------------------------------------*/
/* Common routines */
#define IS_AF(c)  ((c >= 'A') && (c <= 'F'))
#define IS_af(c)  ((c >= 'a') && (c <= 'f'))
#define IS_09(c)  ((c >= '0') && (c <= '9'))
#define ISVALIDHEX(c)  IS_AF(c) || IS_af(c) || IS_09(c)
#define ISVALIDDEC(c)  IS_09(c)
#define CONVERTDEC(c)  (c - '0')

#define CONVERTHEX_alpha(c)  (IS_AF(c) ? (c - 'A'+10) : (c - 'a'+10))
#define CONVERTHEX(c)   (IS_09(c) ? (c - '0') : CONVERTHEX_alpha(c))

void JumpToApp(void);
void JumpToBoot(void);
void FlashDisWriteProtectPage(void);
unsigned int FlashCpy(unsigned int dist,unsigned int src, unsigned int size);
void UpgradeImageProsess(unsigned char* buf, unsigned int* size);





#endif //end upgrade file

