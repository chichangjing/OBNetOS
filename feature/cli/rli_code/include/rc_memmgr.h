/*  
 *  rc_memmgr.h
 *
 *  This is a part of the OpenControl SDK source code library. 
 *
 *  Copyright (C) 1998 Rapid Logic, Inc.
 *  All rights reserved.
 *
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

#ifndef __MEMMGR_HEADER__
#define __MEMMGR_HEADER__

/* Prototypes */
extern RLSTATUS MemMgr_Init ( void );

/* Memory Manager Commands (for rc_memmgr.h) */
#define __MEMMGR_CMND_MSK__	0x0f
#define __MEMMGR_HOLD__		0x01
#define __MEMMGR_RELEASE__	0x02
#define __MEMMGR_READ__		0x03
#define __MEMMGR_FIRST__	0x04
#define __MEMMGR_NEXT__		0x05
#define __MEMMGR_CLEAR__	0x06
#define __MEMMGR_OPTN_MSK__	0xf0
#define __MEMMGR_FIND_MEM__	0x10
#define __MEMMGR_FIND_MRK__	0x20
#define __MEMMGR_FIND_ERR__	0x40

typedef struct MemMgrDebugTable
{
    void*   pMemAddr;
    char*   pFileName;
    int     lineNum;
	ubyte4	memLevel;
	ubyte4	timeCode;
	ubyte4	bufSize;
    ubyte	entryState;

} MemMgrDebugTable;


/* Used for quick verification to insure the absence of memory leaks */
typedef struct MemmgrRec
{
	ubyte4	mMemoryUsed;
	ubyte4	mMemoryTop;
	ubyte4	mMemoryMax;

/* #ifdef  __ENABLE_MEMMGR_DEBUG__ */
	ubyte4	mTimeCode;
	int		tblnow,tbltop,tblmax;
	int		mrkcnt,errcnt;
/*	Boolean	is_fil,is_tbl,is_dbg; */

	ubyte4	tbltim;
	int		tblcnt,tblerr,tblptr;
	struct  MemMgrDebugTable table_entry;

	ubyte4	timmax;
/* #endif __ENABLE_MEMMGR_DEBUG__ */

} MemmgrRec;

#ifdef __cplusplus
extern "C" {
#endif

extern void RLI_DEBUG_MARK_ADD(sbyte *pBuffer, sbyte *pFile, int lineNum);
extern void RLI_DEBUG_MARK_DEL(sbyte *pBuffer, sbyte *pFile, int lineNum);
extern void Memmgr(int cmnd,struct MemmgrRec *pDbgRec);
extern ubyte4 MemMgr_GetMemoryUsage ( void );

#ifdef __cplusplus
}
#endif

#endif
