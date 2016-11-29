/* rc_inflate.h for UnZip -- put in the public domain by Mark Adler
   version c14f, 23 November 1995 */


/* You can do whatever you like with this source file, though I would
   prefer that if you modify it and redistribute it that you include
   comments to that effect with your name and the date.  Thank you.

   History:
   vers    date          who           what
   ----  ---------  --------------  ------------------------------------
    c14  12 Mar 93  M. Adler        made rc_inflate.c standalone with the
                                    introduction of rc_inflate.h.
    c14d 28 Aug 93  G. Roelofs      replaced flush/FlushOutput with new version
    c14e 29 Sep 93  G. Roelofs      moved everything into unzip.h; added crypt.h
    c14f 23 Nov 95  G. Roelofs      added UNZIP_INTERNAL to accommodate newly
                                    split unzip.h
	WC2.0 31 Mar 98 K. Wolcott		incorporated the global structure from 
									globals.h, and put appropriate prototypes in.
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



#ifndef __INFLATE_HEADER__
#define __INFLATE_HEADER__

#ifdef __DECOMPRESSION_ENABLED__

/***************/
/* definitions */
/***************/

#define WSIZE 0x8000  /* window size--must be a power of two, and at least */
	                  /* 32K for zip's deflate method */


#define wsize WSIZE       /* wsize is a constant */


/* Huffman code lookup table entry--this entry is four bytes for machines
   that have 16-bit pointers (e.g. PC's in the small or medium model).
   Valid extra bits are 0..13.  e == 15 is EOB (end of block), e == 16
   means that v is a literal, 16 < e < 32 means that v is a pointer to
   the next table, which codes e - 16 bits, and lastly e == 99 indicates
   an unused code.  If a code with e == 99 is looked up, this implies an
   error in the data. */

struct huft {
    uch e;                /* number of extra bits or operation */
    uch b;                /* number of bits in this code or subcode */
    union {
        ush n;            /* literal, length base, or distance base */
        struct huft *t;   /* pointer to next level of table */
    } v;
};

/* This structure is based on the Globals struct found in the unzip source.
 * However, it is renamed to reflect it's true usage, monitoring the inflation */
typedef struct Inflate_Control_Block {
    uch			*inbuf;			
    ulg			in_idx;			

    ulg			crc32val;		

    uch			*slide;			

    int			qflag;          

 
    uch			*outbuf;
    ulg			out_idx;

    uch			*outptr;
    ulg			outcnt;			

    unsigned	hufts;			
    struct huft *fixed_tl;		
    struct huft *fixed_td;	

    int			fixed_bl;
	int			fixed_bd;
	
    unsigned	wp;				
    ulg			bb;				
    unsigned	bk;				

} Inflate_Control_Block;  


#ifdef __cplusplus
extern "C" {
#endif

extern	RLSTATUS	INFLATE_Init	(void				);
extern	int			rc_inflate		(Inflate_Control_Block *pICB	);
extern	int			inflate_free	(Inflate_Control_Block *pICB	);

#ifdef __cplusplus
}
#endif

#endif /* __DECOMPRESSION_ENABLED__ */

#endif /* !__INFLATE_HEADER__ */
