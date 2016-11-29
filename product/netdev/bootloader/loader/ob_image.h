

#ifndef __OB_IMAGE_H
#define __OB_IMAGE_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
/* Exported macro ------------------------------------------------------------*/

#define OB_IMG_HEADER_LENGTH		64
#define IH_MAGIC					0x4F424E50		/* "OBNP", OB Network Platform */

/* Image info length */
#define	IH_NAME_LEN					16
#define	IH_DATE_LEN					16
#define	IH_VERSION_LEN				16

/* Image Data Types */
#define	IH_TYPE_INVALID				0x00			/* Invalid Image */
#define	IH_TYPE_BOOTLOADER			0x01			/* Bootloader Image */
#define IH_TYPE_APPLICATION			0x02			/* Application Image */
#define IH_TYPE_CONFIG_DATA			0x03			/* Configuration Image */

/* Image store media */
#define	IH_PROG_TYPE_UNKOWN			0x00			/* Not use */
#define IH_PROG_TYPE_FLASH			0x01			/* On-Chip Flash */
#define IH_PROG_TYPE_EEPROM			0x02			/* EEprom */

/* Image data format */ 
#define	IH_DATA_FORMAT_UNKOWN		0x00
#define	IH_DATA_FORMAT_BIN			0x01			/* Binary */
#define	IH_DATA_FORMAT_SREC			0x02			/* S-Record */


/* Exported types ------------------------------------------------------------*/

typedef struct OB_Image_Header {
	u32	Magic;							/* Image Header Magic Number */
	u32	HeaderCRC32;					/* Image Header CRC Checksum */
	u32	DataSize;						/* Image Data Size */
	u32	DataCRC32;						/* Image Data CRC Checksum	*/
	u32	LoadAddress;					/* Image Load Address	*/	
	u8	ImageType;						/* Image Type */
	u8	Reserved[11];					/* Reserved1 */
	u8	ImageName[IH_NAME_LEN];			/* Image Name */
	u8	ImageVerion[IH_VERSION_LEN];	/* Image Version */
} OB_Image_Header_t;


#define	SWAP_LONG(x) ((unsigned int)( \
		(((unsigned int)(x) & (unsigned int)0x000000ffUL) << 24) | \
		(((unsigned int)(x) & (unsigned int)0x0000ff00UL) <<  8) | \
		(((unsigned int)(x) & (unsigned int)0x00ff0000UL) >>  8) | \
		(((unsigned int)(x) & (unsigned int)0xff000000UL) >> 24) ))
#define	ntohl(a)	SWAP_LONG(a)
#define	htonl(a)	SWAP_LONG(a)

#define IHCHK_OK		(0)		/* Image Header Check OK */
#define IHCHK_ERR_MAGIC	(-1)	/* Magic check error */
#define IHCHK_ERR_HCRC	(-2)	/* Header CRC check error */
#define IHCHK_ERR_DCRC	(-3)	/* Data CRC check error */
#define IHCHK_ERR_NAME	(-4)	/* Image name check error */

/* Exported functions ------------------------------------------------------- */
unsigned long crc32(unsigned long crc, const unsigned char *buf, unsigned int len);
int OB_Check_Upgrade_Image(unsigned int address, unsigned int *datasize, unsigned int *crc32);

#endif	/* __OB_IMAGE_H */


