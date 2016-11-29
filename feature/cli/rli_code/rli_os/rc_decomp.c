/*  
 *	rc_decomp.c
 *
 *  This is a part of the OpenControl SDK source code library. 
 *
 *
 *  This code is based heavily on funzip.c, which was put in the public domain by 
 *  Mark Adler.  Following the zip file conventions, this is the work that was
 *  done to the file.
 *
    vers     date          who           what
    ----   ---------  --------------  ------------------------------------
	WC2.0  31 Mar 98  K. Wolcott	  mated the code with the OCBP, made
									  code match with RL conventions.
 */

/*----------------------------------------------------------------------
 *
 * NAME CHANGE NOTICE:
 *
 * On May 11th, 1999, Rapid Logic changed its corporate naming scheme.
 * The changes are as follows:
 *
 *      OLD NAME                        NEW NAME
 *
 *      OpenControl                     RapidControl
 *      WebControl                      RapidControl for Web
 *      JavaControl                     RapidControl for Applets
 *      MIBway                          MIBway for RapidControl
 *
 *      OpenControl Backplane (OCB)     RapidControl Backplane (RCB)
 *      OpenControl Protocol (OCP)      RapidControl Protocol (RCP)
 *      MagicMarkup                     RapidMark
 *
 * The source code portion of our product family -- of which this file 
 * is a member -- will fully reflect this new naming scheme in an upcoming
 * release.
 *
 *
 * RapidControl, RapidControl for Web, RapidControl Backplane,
 * RapidControl for Applets, MIBway, RapidControl Protocol, and
 * RapidMark are trademarks of Rapid Logic, Inc.  All rights reserved.
 *
 */

/* Included herein was the original header of funzip.c */

/* Start of original header ------------------------- */

/* funzip.c -- put in the public domain by Mark Adler */

#define VERSION "3.92 of 31 May 1997"
/*

$History: rc_decomp.c $
 * 
 * *****************  Version 14  *****************
 * User: Leech        Date: 6/22/01    Time: 5:45p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Renamed inflate to rc_inflate to avoid collision with native VxWorks
 * lib
 * 
 * *****************  Version 13  *****************
 * User: Pstuart      Date: 1/11/01    Time: 3:05p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * fix gcc warning
 * 
 * *****************  Version 12  *****************
 * User: Pbrar        Date: 8/07/00    Time: 2:46p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * OS_SPECIFIC_CALLOC to  OS_SPECIFIC_MALLOC 
 * 
 * *****************  Version 11  *****************
 * User: Schew        Date: 8/02/00    Time: 3:11p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * RC_MALLOC to OS_SPECIFIC_MALLOC
 * 
 * *****************  Version 10  *****************
 * User: Leech        Date: 6/21/00    Time: 11:49a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments
 * 
 * *****************  Version 9  *****************
 * User: Schew        Date: 4/28/00    Time: 10:23a
 * Change CALLOC to RC_CALLOC
 * 
 * *****************  Version 8  *****************
 * User: Epeterson    Date: 4/25/00    Time: 5:22p
 * Include history and enable auto archiving feature from VSS


*/

/* You can do whatever you like with this source file, though I would
   prefer that if you modify it and redistribute it that you include
   comments to that effect with your name and the date.  Thank you.

   History:
   vers     date          who           what
   ----   ---------  --------------  ------------------------------------
   1.0    13 Aug 92  M. Adler        really simple unzip filter.
   1.1    13 Aug 92  M. Adler        cleaned up somewhat, give help if
                                     stdin not redirected, warn if more
                                     zip file entries after the first.
   1.2    15 Aug 92  M. Adler        added check of lengths for stored
                                     entries, added more help.
   1.3    16 Aug 92  M. Adler        removed redundant #define's, added
                                     decryption.
   1.4    27 Aug 92  G. Roelofs      added exit(0).
   1.5     1 Sep 92  K. U. Rommel    changed read/write modes for OS/2.
   1.6     6 Sep 92  G. Roelofs      modified to use dummy crypt.c and
                                     crypt.h instead of -DCRYPT.
   1.7    23 Sep 92  G. Roelofs      changed to use DOS_OS2; included
                                     crypt.c under MS-DOS.
   1.8     9 Oct 92  M. Adler        improved inflation error msgs.
   1.9    17 Oct 92  G. Roelofs      changed ULONG/UWORD/byte to ulg/ush/uch;
                                     renamed inflate_entry() to inflate();
                                     adapted to use new, in-place zdecode.
   2.0    22 Oct 92  M. Adler        allow filename argument, prompt for
                                     passwords and don't echo, still allow
                                     command-line password entry, but as an
                                     option.
   2.1    23 Oct 92  J-l. Gailly     fixed crypt/store bug,
                     G. Roelofs      removed crypt.c under MS-DOS, fixed
                                     decryption check to compare single byte.
   2.2    28 Oct 92  G. Roelofs      removed declaration of key.
   2.3    14 Dec 92  M. Adler        replaced fseek (fails on stdin for SCO
                                     Unix V.3.2.4).  added quietflg for
                                     rc_inflate.c.
   3.0    11 May 93  M. Adler        added gzip support
   3.1     9 Jul 93  K. U. Rommel    fixed OS/2 pipe bug (PIPE_ERROR)
   3.2     4 Sep 93  G. Roelofs      moved crc_32_tab[] to tables.h; used FOPx
                                     from unzip.h; nuked OUTB macro and outbuf;
                                     replaced flush(); inlined FlushOutput();
                                     renamed decrypt to encrypted
   3.3    29 Sep 93  G. Roelofs      replaced ReadByte() with NEXTBYTE macro;
                                     revised (restored?) flush(); added FUNZIP
   3.4    21 Oct 93  G. Roelofs      renamed quietflg to qflag; changed outcnt,
                     H. Gessau       second updcrc() arg and flush() arg to ulg;
                                     added inflate_free(); added "g =" to null
                                     getc(in) to avoid compiler warnings
   3.5    31 Oct 93  H. Gessau       changed DOS_OS2 to DOS_NT_OS2
   3.6     6 Dec 93  H. Gessau       added "near" to mask_bits[]
   3.7     9 Dec 93  G. Roelofs      added extent typecasts to fwrite() checks
   3.8    28 Jan 94  GRR/JlG         initialized g variable in main() for gcc
   3.81   22 Feb 94  M. Hanning-Lee  corrected usage message
   3.82   27 Feb 94  G. Roelofs      added some typecasts to avoid warnings
   3.83   22 Jul 94  G. Roelofs      changed fprintf to macro for DLLs
    -      2 Aug 94  -               public release with UnZip 5.11
    -     28 Aug 94  -               public release with UnZip 5.12
   3.84    1 Oct 94  K. U. Rommel    changes for Metaware High C
   3.85   29 Oct 94  G. Roelofs      changed fprintf macro to Info
   3.86    7 May 95  K. Davis        RISCOS patches;
                     P. Kienitz      Amiga patches
   3.87   12 Aug 95  G. Roelofs      inflate_free(), DESTROYGLOBALS fixes
   3.88    4 Sep 95  C. Spieler      reordered macro to work around MSC 5.1 bug
   3.89   22 Nov 95  PK/CS           ifdef'd out updcrc() for ASM_CRC
   3.9    17 Dec 95  G. Roelofs      modified for USE_ZLIB (new fillinbuf())
    -     30 Apr 96  -               public release with UnZip 5.2
   3.91   17 Aug 96  G. Roelofs      main() -> return int (Peter Seebach)
   3.92   13 Apr 97  G. Roelofs      minor cosmetic fixes to messages
    -     22 Apr 97  -               public release with UnZip 5.3
    -     31 May 97  -               public release with UnZip 5.31
 */


/*

   All funzip does is take a zipfile from stdin and decompress the
   first entry to stdout.  The entry has to be either deflated or
   stored.  If the entry is encrypted, then the decryption password
   must be supplied on the command line as the first argument.

   funzip needs to be linked with rc_inflate.o and crypt.o compiled from
   the unzip source.  If decryption is desired, the full version of
   crypt.c (and crypt.h) from zcrypt21.zip or later must be used.

 */

/* End of original header ------------------------- */


/* Headers to work with the OpenControl Back Plane */
#include "rc_options.h"
#include "rc_rlstddef.h"
#include "rc_errors.h"
#include "rc_rlstdlib.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"

/* Decompression Headers */
#include "rc_dc_defs.h"
#include "rc_inflate.h"
#include "rc_decomp.h"
#include "rc_crc32.h"

#ifdef __DECOMPRESSION_ENABLED__

#define kLocationLenOffset	22			/* offset of PKZIP Length from 
										 * beginning of data */
#define kGzipMagicNum		"\x01f\x08b"
#define kPkzipMagicNum		"\x050\x4b\x003\x004"
#define kStored				0
#define kDeflated			8

/* PKZIP header definitions */
#define ZIPMAG 0x4b50           /* two-byte zip lead-in */
#define LOCREM 0x0403           /* remaining two bytes in zip signature */
#define LOCSIG 0x04034b50L      /* full signature */
#define LOCFLG 4                /* offset of bit flag */
#define  CRPFLG 1               /*  bit for encrypted entry */
#define  EXTFLG 8               /*  bit for extended local header */
#define LOCHOW 6                /* offset of compression method */
#define LOCTIM 8                /* file mod time (for decryption) */
#define LOCCRC 12               /* offset of crc */
#define LOCSIZ 16               /* offset of compressed size */
#define LOCLEN 20               /* offset of uncompressed length */
#define LOCFIL 24               /* offset of file name field length */
#define LOCEXT 26               /* offset of extra field length */
#define LOCHDR 28               /* size of local header, including LOCREM */
#define EXTHDR 16               /* size of extended local header, inc sig */

/* GZIP header definitions */
#define GZPMAG 0x8b1f           /* two-byte gzip lead-in */
#define GZPHOW 0                /* offset of method number */
#define GZPFLG 1                /* offset of gzip flags */
#define  GZPMUL 2               /* bit for multiple-part gzip file */
#define  GZPISX 4               /* bit for extra field present */
#define  GZPISF 8               /* bit for filename present */
#define  GZPISC 16              /* bit for comment present */
#define  GZPISE 32              /* bit for encryption */
#define GZPTIM 2                /* offset of Unix file modification time */
#define GZPEXF 6                /* offset of extra flags */
#define GZPCOS 7                /* offset of operating system compressed on */
#define GZPHDR 8                /* length of minimal gzip header */

/* Macros for getting two-byte and four-byte header values */
#define SH(p) ((ush)(uch)((p)[0]) | ((ush)(uch)((p)[1]) << 8))
#define LG(p) ((ulg)(SH(p)) | ((ulg)(SH((p)+2)) << 16))



/*-----------------------------------------------------------------------*/

extern sbyte
DECOMP_GetNextByte(Inflate_Control_Block	*pICB)
{
	sbyte	val;

	val = pICB->inbuf[pICB->in_idx];
	pICB->in_idx++;

	return val;
}



/*-----------------------------------------------------------------------*/

static void
DECOMP_WriteDataBlock(Inflate_Control_Block *pICB, sbyte *pDataBlock)
{
	MEMCPY(&(pICB->outbuf[pICB->out_idx]), pDataBlock, (extent)pICB->outcnt);
	pICB->out_idx += pICB->outcnt;
}



/*-----------------------------------------------------------------------*/

static void
DECOMP_ReadDataBlock(sbyte *pBuffer, Inflate_Control_Block *pICB, ubyte4 numBytes)
{
	MEMCPY(pBuffer, &(pICB->inbuf[pICB->in_idx]), numBytes);
	pICB->in_idx += numBytes;
}



/*-----------------------------------------------------------------------*/

static RLSTATUS
DECOMP_Unzip(Inflate_Control_Block *pICB)
{
  ush n;
  uch h[LOCHDR];                /* first local header (GZPHDR < LOCHDR) */
  int g = 0;                    /* true if gzip format */
 
   /* read local header, check validity, and skip name and extra fields */
  n = DECOMP_GetNextByte(pICB);  n |= DECOMP_GetNextByte(pICB) << 8;
  if (n == ZIPMAG)
  {
	DECOMP_ReadDataBlock((char*)h, pICB, LOCHDR);
    if (SH(h) != LOCREM)
		return ERROR_DECOMP_BAD_PKZIP_FILE;

    if (SH(h + LOCHOW) != kStored && SH(h + LOCHOW) != kDeflated)
		return ERROR_DECOMP_BAD_FIRST_ENTRY;

    for (n = SH(h + LOCFIL); n--; ) g = DECOMP_GetNextByte(pICB);
    for (n = SH(h + LOCEXT); n--; ) g = DECOMP_GetNextByte(pICB);
    g = 0;
  }
  else if (n == GZPMAG)
  {
	DECOMP_ReadDataBlock((char*)h, pICB, GZPHDR);
    if (h[GZPHOW] != kDeflated)
		return ERROR_DECOMP_GZIP_FILE_NOT_DEFLATED;

    if (h[GZPFLG] & GZPMUL)
		return ERROR_DECOMP_MULTIPART_GZIP_FILES;

    if (h[GZPFLG] & GZPISX)
    {
      n = DECOMP_GetNextByte(pICB);  n |= DECOMP_GetNextByte(pICB) << 8;
      while (n--) g = DECOMP_GetNextByte(pICB);
    }
    if (h[GZPFLG] & GZPISF)
      while ((g = DECOMP_GetNextByte(pICB)) != 0) ;
    if (h[GZPFLG] & GZPISC)
      while ((g = DECOMP_GetNextByte(pICB)) != 0) ;
    g = 1;
  }
  else
    return ERROR_DECOMP_INVALID_FILE_FORMAT;

  /* prepare output buffer and crc */
  pICB->outptr	= pICB->slide;
  pICB->outcnt	= 0L;
  pICB->out_idx	= 0L;
  pICB->crc32val	= kCrcInitialValue;

  /* decompress */
  if (g || h[LOCHOW])
  {                             /* deflated entry */
    int r;

    if ((r = rc_inflate(pICB)) != 0)
    {
      if (r == 3)
        return ERROR_MEMMGR_NO_MEMORY;
      else
		return ERROR_DECOMP_FORMAT_VIOLATION;
    }
	inflate_free(pICB);
  }
  else
  {                             /* stored entry */
    register ulg n;

    n = LG(h + LOCLEN);

    if (n != LG(h + LOCSIZ)) {
      return ERROR_DECOMP_LENGTH_MISMATCH;
    }
    while (n--) {
      ush c = DECOMP_GetNextByte(pICB);
      *pICB->outptr++ = (uch)c;
      if (++pICB->outcnt == WSIZE)    /* do FlushOutput() */
      {
        pICB->crc32val = crc32(pICB->crc32val, pICB->slide, (extent)pICB->outcnt);
        
		DECOMP_WriteDataBlock(pICB, (char *)pICB->slide);
        pICB->outptr = pICB->slide;
        pICB->outcnt = 0L;
      }
    }
  }
  if (pICB->outcnt)   /* flush one last time; no need to reset pICB->outptr/outcnt */
  {
    pICB->crc32val = crc32(pICB->crc32val, pICB->slide, (extent)pICB->outcnt);
 	DECOMP_WriteDataBlock(pICB, (char *)pICB->slide);
  }

  /* if extended header, get it */
  if (g)
  {
	DECOMP_ReadDataBlock((char *)h + LOCCRC, pICB, 8);
  }
  else if ((h[LOCFLG] & EXTFLG) )
	{
		DECOMP_ReadDataBlock((char *)h + LOCCRC - 4, pICB, EXTHDR);
	}

  /* validate decompression */
  if (LG(h + LOCCRC) != pICB->crc32val)
	  return ERROR_DECOMP_CRC_MISMATCH;

  if (LG((g ? (h + LOCSIZ) : (h + LOCLEN))) != pICB->out_idx)
	  return ERROR_DECOMP_DATA_LENGTH;

  return OK;
}



/*-----------------------------------------------------------------------*/

static void	
DECOMP_GetOrigLen(sbyte *pData, sbyte4 dataLen, sbyte4 *pDecompressedLen)
{
	ubyte	*pDataLenLocation;
	ubyte4	decompressedLen;

	if ( 0 == MEMCMP(pData, kGzipMagicNum, 2))
	{
		pDataLenLocation = ((ubyte*)pData + dataLen) - 4;
		decompressedLen =  (pDataLenLocation[3] << 24) 
						 | (pDataLenLocation[2] << 16)
						 | (pDataLenLocation[1] << 8 )
						 | (pDataLenLocation[0] );

		*pDecompressedLen = (sbyte4)decompressedLen;
	}
	else if ( 0 == MEMCMP(pData, kPkzipMagicNum, 4))
	{
		pDataLenLocation = ((ubyte*)pData + kLocationLenOffset);
		decompressedLen =  (pDataLenLocation[3] << 24) 
						 | (pDataLenLocation[2] << 16)
						 | (pDataLenLocation[1] << 8 )
						 | (pDataLenLocation[0] );

		*pDecompressedLen = (sbyte4)decompressedLen;
	}
	else
		*pDecompressedLen = 0;
}



/*-----------------------------------------------------------------------*/

extern void	
DECOMP_IsCompressed(sbyte *pData, Boolean *pIsCompressed)
{
	if (	( 0 == MEMCMP(pData, kGzipMagicNum, 2 ))
		||	( 0 == MEMCMP(pData, kPkzipMagicNum, 4)) )
		*pIsCompressed = TRUE;
	else
		*pIsCompressed = FALSE;
}


	
/*-----------------------------------------------------------------------*/

extern RLSTATUS	
DECOMP_DecompressData	(	sbyte *pData, sbyte4 dataLen, 
							sbyte **ppDecompressedData, sbyte4 *pDecompressedLen)
{
	sbyte4					origFileLen;
	sbyte				   *pDecompressedData;
	RLSTATUS				status;
	Inflate_Control_Block  *pICB;
	size_t					slideSize;

	*ppDecompressedData = NULL;
	*pDecompressedLen	= 0;

	/* Get a buffer for the expanded data */
	DECOMP_GetOrigLen(pData, dataLen, &origFileLen);
	/* N.B. OS_SPECIFIC_MALLOC is used here because the Retrieval function expects
	 * file space to be allocated outside of the OpenControl memory space */
	pDecompressedData = OS_SPECIFIC_MALLOC(origFileLen);
	if (NULL == pDecompressedData)
		return SYS_ERROR_NO_MEMORY;

    MEMSET(pDecompressedData, 0, origFileLen);

	/* Get a "Inflate_Control_Block" struct, which is used to monitor the expansion */
	pICB = (Inflate_Control_Block *) OS_SPECIFIC_MALLOC(sizeof(Inflate_Control_Block));
	if (NULL == pICB)
	{
		OS_SPECIFIC_FREE(pDecompressedData);
		return ERROR_MEMMGR_NO_MEMORY;
	}
    MEMSET(pICB,0,sizeof(Inflate_Control_Block));

	/* Initialize buffers */
	pICB->inbuf		= (ubyte*)pData;
	pICB->outbuf	= (ubyte*)pDecompressedData;

	/* Create scratch space */
	if ( WSIZE > origFileLen )
		slideSize = origFileLen;
	else
		slideSize = 8193 * (sizeof(short) + sizeof(char) + sizeof(char));
	
	pICB->slide = OS_SPECIFIC_MALLOC(slideSize);

	if (NULL == pICB->slide)
	{
		OS_SPECIFIC_FREE(pDecompressedData);
		OS_SPECIFIC_FREE(pICB);
		return ERROR_MEMMGR_NO_MEMORY;
	}

	MEMSET(pICB->slide, 0, slideSize);
	status = DECOMP_Unzip(pICB);

 	/* return the data everyone is looking for, and clean up */
	*ppDecompressedData = pDecompressedData;
	*pDecompressedLen	= origFileLen;

	OS_SPECIFIC_FREE(pICB->slide);
	OS_SPECIFIC_FREE(pICB);
	return status;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS	
DECOMP_Init	( void )
{
	RLSTATUS status;

	status = INFLATE_Init();
	if ( 0 > status )
		return status;

	status = CRC32_Init();

	return status;
}

#endif /* __DECOMPRESSION_ENABLED__ */
