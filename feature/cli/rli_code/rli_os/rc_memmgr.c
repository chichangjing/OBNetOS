/*  
 *  rc_memmgr.c
 *
 *  This is a part of the OpenControl SDK source code library. 
 *
 *  Copyright (C) 2000 Rapid Logic, Inc.
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
/*

$History: rc_memmgr.c $
 * 
 * *****************  Version 34  *****************
 * User: Schew        Date: 10/24/00   Time: 5:42p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * __ENABLE_DANGLING_PTR_TESTING__
 * 
 * *****************  Version 33  *****************
 * User: James        Date: 10/24/00   Time: 3:55p
 * Updated in $/Rapid Logic/Mediation/rli_code/rli_os
 * 
 * *****************  Version 32  *****************
 * User: Kedron       Date: 10/04/00   Time: 3:31p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * made pOutput file a module level variable.  also changed it's name to
 * mpOutput
 * 
 * *****************  Version 31  *****************
 * User: Pstuart      Date: 8/16/00    Time: 2:57p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Apparently, MemMgrDebugTable is not a debug table but is a normal part
 * of the RL memory mgr.
 * 
 * *****************  Version 30  *****************
 * User: Pstuart      Date: 8/04/00    Time: 11:19a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * fix more gcc warnings
 * 
 * *****************  Version 29  *****************
 * User: Pstuart      Date: 8/04/00    Time: 10:28a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * fix gcc warnings
 * 
 * *****************  Version 28  *****************
 * User: Schew        Date: 7/25/00    Time: 5:12p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Disable __ENABLE_MEMMGR_DEBUG__ when using Host System memory
 * 
 * *****************  Version 27  *****************
 * User: Leech        Date: 6/21/00    Time: 11:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments
 * 
 * *****************  Version 26  *****************
 * User: Dreyna       Date: 6/20/00    Time: 2:25p
 * Global for the simulate failure variables.
 * 
 * *****************  Version 25  *****************
 * User: Dreyna       Date: 6/14/00    Time: 7:09p
 * Added memory failure simulation routine to gebug mode.
 * 
 * *****************  Version 24  *****************
 * User: Pstuart      Date: 5/08/00    Time: 5:24p
 * fixed the fix for envoysam
 * 
 * *****************  Version 23  *****************
 * User: Pstuart      Date: 5/08/00    Time: 4:14p
 * Fix for memmgr debug w/ envoysam.lib
 * 
 * *****************  Version 21  *****************
 * User: Pstuart      Date: 5/08/00    Time: 11:18a
 * MEMMGR_DEBUG vs. ENVOY_SAM
 * 
 * *****************  Version 20  *****************
 * User: Epeterson    Date: 4/25/00    Time: 5:22p
 * Include history and enable auto archiving feature from VSS


*/
#include "rc_options.h"

/* can only debug our own code */
#ifdef __OS_MALLOC_PROVIDED__
#undef __ENABLE_MEMMGR_DEBUG__
#endif

#ifdef  __ENABLE_MEMMGR_DEBUG__
#include <stdio.h>
#ifdef __WIN32_OS__
#include <io.h>
#endif /* __WIN32_OS__ */
#endif /* __ENABLE_MEMMGR_DEBUG__ */

#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"

/* Allow RC_FREE and RC_MALLOC for ENVOYSAM.LIB */
#ifdef  __ENABLE_MEMMGR_DEBUG__
#undef  __ENABLE_MEMMGR_DEBUG__
#include "rc_rlstdlib.h"
#define  __ENABLE_MEMMGR_DEBUG__
#else
#include "rc_rlstdlib.h"
#endif /* __ENABLE_MEMMGR_DEBUG__ */

#include "rc_memmgr.h"
#include "rc_msghdlr.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"



/*-----------------------------------------------------------------------*/

/* Debug Table Entry Status */
#define kEmptyEntry             0x01
#define kActiveEntry            0x02
#define kHoldEntry				0x10
#define kMarkEntry				0x20
#define kErrorFreeEntry         0x40
#define kErrorGetEntry			0x80
#define kErrorEntry				(kErrorGetEntry|kErrorFreeEntry)



/*-----------------------------------------------------------------------*/

#ifdef __OS_MALLOC_PROVIDED__
#ifdef __ENABLE_DANGLING_PTR_TESTING__

#define kMaxDanglingEntries	10000
#define kDanglingClearByte  0xcd

#define kAllocActive		0x11
#define kAllocInactive		0x34

sbyte4  mNumDanglingEntries = 0;

static OS_SPECIFIC_MUTEX    mDanglingMutex;

typedef struct
{
	sbyte*	pMemBlock;
	sbyte4	memBlockSize;
	sbyte4  memBlockState;

    sbyte*  pFilename;
    sbyte4  lineNum;

} danglingPtrDescr;

danglingPtrDescr danglingWatchList[kMaxDanglingEntries];

#endif /* __ENABLE_DANGLING_PTR_TESTING__ */
#endif /* __OS_MALLOC_PROVIDED__ */



/*===========Memory Debug Setup==========================================*/
/*===  The memory manager debug code can be enabled to track memory   ===*/
/*===    usage, possible leaks, failed ALLOCs, and unmatched RC_FREEs.===*/
/*===  If the data is tabulated in the local table, reports can be    ===*/
/*===    generated by a call to MEMMGR_DisplayDebug(), or it can be   ===*/
/*===    access via VD_MEMMGR. A full table that includes all of the  ===*/
/*===    blades and tutorials will take a max of 1400 entries. The    ===*/
/*===    SQUEEZE option will remove entries below 'base' markers, data===*/
/*===    that is basically static (like the backplane database). In   ===*/
/*===    this case, only 200 table entries will probably be needed.   ===*/
/*===  If the data is saved to a file, you can use Excel and the prog ===*/
/*===    memfilter.exe to sort and remove alloc/free pairs, leaving   ===*/
/*===    memory leaks. The file option is slow; you may have OCP      ===*/
/*===    rapidmarks timeout and return blanks on occasion.            ===*/
/*===  The method RLI_DEBUG_MARK_ADD() allows you to add baseline     ===*/
/*===    marks to view where the 'static' memory is distibuted, and   ===*/
/*===    also 'ceilings' (so that VD_MEMMGR doesn't show its own      ===*/
/*===    ALLOCs).                                                     ===*/
/*===  The table is completely re-entrant. Memmgr() captures a snap-  ===*/
/*===    shot of the memory management state for display, while       ===*/
/*===    allowing the table to still fully act during the display     ===*/
/*===    time. Any table overflows will be noted, but will not stop   ===*/
/*===    operation of the machine.                                    ===*/
/*===  To enable this feature, add the line:                          ===*/
/*===    "#define __ENABLE_MEMMGR_DEBUG_FILE__" to  rc_options.h via the ===*/
/*===    future extensions dialog box.                                ===*/
/*=======================================================================*/
#ifdef  __ENABLE_MEMMGR_DEBUG__

/*======================== Debug File Control ===========================*/
#ifdef __MEMMGR_DEBUG_FILE__
static FILE  *mpOutput;

#define  MemusageFile __MEMMGR_DEBUG_FILE__
#endif
/*=======================================================================*/

/* Debug Squeeze Table Control */
#define __ENABLE_MEMMGR_DEBUG_SQUEEZE__


/* Debug Table Size */
#ifndef __ENABLE_MEMMGR_DEBUG_SQUEEZE__
/* If Not Squeezed, let kMaxMemMgrLeakTable=1400 if full blade tutorial */
#define kMaxMemMgrLeakTable    1400
#else
/* If     Squeezed, let kMaxMemMgrLeakTable= 200 */
#define kMaxMemMgrLeakTable     200
#endif

#else	/* !__ENABLE_MEMMGR_DEBUG__ */
#define kMaxMemMgrLeakTable     1
#endif

/*======================================================*/

#ifndef __OS_MALLOC_PROVIDED__

static MemMgrDebugTable m_mallocTable[kMaxMemMgrLeakTable];     /* used to find memory leaks */

#define kInitValue      0xFF
#define kMagicMarker    0xD0B22368
 
typedef struct MemBlock
{
    struct MemBlock *pNextMemBlk;
    ubyte4           memBlkLen;
    
} MemBlock;
                
static ubyte4               mMaxAddr;
static ubyte4               mEndAddr;
static MemBlock            *pmMemFreeListHead;
static OS_SPECIFIC_MUTEX    mMutex;

static struct MemmgrRec mMemmgr;

#endif /* __OS_MALLOC_PROVIDED__ */



/*-----------------------------------------------------------------------*/

#ifndef __OS_MALLOC_PROVIDED__
 
static void
MemMgr_LockFreeList ( void )
{
	OS_SPECIFIC_MUTEX_WAIT( mMutex );
}

static void
MemMgr_UnlockFreeList ( void )
{
	OS_SPECIFIC_MUTEX_RELEASE( mMutex );
}

#endif /* __OS_MALLOC_PROVIDED__ */

/*-----------------------------------------------------------------------*/

#ifdef __ENABLE_MEMMGR_DEBUG__
extern void MEMMGR_InitDebug()
{
    int index;

    for (index = 0; index < kMaxMemMgrLeakTable; index++)
        m_mallocTable[index].entryState = kEmptyEntry;

	mMemmgr.tblnow = 0;
	mMemmgr.tbltop = 0;
	mMemmgr.tblmax = kMaxMemMgrLeakTable;

	mMemmgr.errcnt = 0;
	mMemmgr.mrkcnt = 0;

	mMemmgr.timmax = 9999999;
	mMemmgr.tbltim = -1;

	mMemmgr.mTimeCode = 0;

	/* Start with a clean slate each session, for now */
#ifdef __ENABLE_MEMMGR_DEBUG_FILE__
	if (TRUE) remove(MemusageFile);
#endif

}

/* This routine are used to simulate Alloc Failures */
/* Breakpoint here, set the new limits, and catch the failures */
static ubyte4 nSimFail_MaxMemory = 0;
static ubyte4 nSimFail_FailDelay = 0;
static ubyte4 nSimFail_DelayCnt  = 0;
extern int MEMMGR_SimFail()
{

	/* If fail delay countdown, fail */
	if (nSimFail_FailDelay && (nSimFail_DelayCnt++ > nSimFail_FailDelay))
	{
		nSimFail_DelayCnt = 0;
		return(TRUE);
	}

	/* If over memory limit, fail */
	if (nSimFail_MaxMemory && (mMemmgr.mMemoryUsed > nSimFail_MaxMemory))
	{
		nSimFail_DelayCnt = 0;
		return(TRUE);
	}

	return(FALSE);
}

/*-----------------------------------------------------------------------*/

extern void MEMMGR_DisplayDebug()
{
	int index;

	printf("\n\n");
	printf("MEMORY ALLOCS:\n");
	printf("=============\n");
    
	for (index = 0; index < kMaxMemMgrLeakTable; index++)
		if (kActiveEntry & m_mallocTable[index].entryState)
		{
			printf("addr= %08x\tfile = %12s\tline = %d\n", (unsigned int) m_mallocTable[index].pMemAddr, m_mallocTable[index].pFileName, m_mallocTable[index].lineNum);
		}
	printf("\n");

	printf("MEMORY MARKS:\n");
	printf("===============\n");
	for (index = 0; index < kMaxMemMgrLeakTable; index++)
		if (kMarkEntry & m_mallocTable[index].entryState)
		{
			printf("addr= %08x\tfile = %12s\tline = %d\n", (unsigned int) m_mallocTable[index].pMemAddr, m_mallocTable[index].pFileName, m_mallocTable[index].lineNum);
		}
	printf("\n");

	printf("MEMORY ERRORS:\n");
	printf("=============\n");
	for (index = 0; index < kMaxMemMgrLeakTable; index++)
		if (kErrorEntry & m_mallocTable[index].entryState)
		{
			printf("addr= %08x\tfile = %12s\tline = %d\n", (unsigned int) m_mallocTable[index].pMemAddr, m_mallocTable[index].pFileName, m_mallocTable[index].lineNum);
		}
	printf("\n");
}

/*-----------------------------------------------------------------------*/

static void AddMallocEntry(void *pMemAddr, char *pFileName, int lineNum, int size, int state)
{	
    int index;
    sbyte *pFile;

	mMemmgr.mTimeCode++;
	pFile = STRRCHR(pFileName, '\\');
	if        ( pFile == NULL) pFile = pFileName;
	  else if (*pFile == '\\') pFile++;

    if (NULL == pMemAddr)
        return;

#ifdef __ENABLE_MEMMGR_DEBUG_FILE__
	if (NULL == (mpOutput = fopen(MemusageFile ,"a"))) {
		printf("Failed to open Memusage.txt");
	} else {
		fprintf(mpOutput,"ALLOC:\t%08x\tFile:%s      \tLineNum:\t%d\ttime:\t%d\tsize:\t%d\tMemtop:\t%d\n", 
			(unsigned int) pMemAddr, pFile, lineNum, mMemmgr.mTimeCode, size,mMemmgr.mMemoryUsed);
		fclose(mpOutput);
	}
#endif

	mMemmgr.tblnow++;
	if (mMemmgr.tbltop < mMemmgr.tblnow)
		mMemmgr.tbltop = mMemmgr.tblnow;
	
    for (index = 0; index < kMaxMemMgrLeakTable; index++)
        if (kEmptyEntry == m_mallocTable[index].entryState)
        {
            m_mallocTable[index].entryState = state;
            m_mallocTable[index].pMemAddr   = pMemAddr;
            m_mallocTable[index].pFileName  = pFile;
            m_mallocTable[index].lineNum    = lineNum;
            m_mallocTable[index].bufSize    = size;

            m_mallocTable[index].memLevel  = mMemmgr.mMemoryUsed;
            m_mallocTable[index].timeCode  = mMemmgr.mTimeCode  ;

            return;
        }
}

/*-----------------------------------------------------------------------*/

static void RemoveMallocEntry(void *pMemAddr, char *pFileName, int lineNum)
{
    int index;
    sbyte *pFile;

    pFile = STRRCHR(pFileName, '\\');
	if        ( pFile == NULL) pFile = pFileName;
	  else if (*pFile == '\\') pFile++;

#ifdef __ENABLE_MEMMGR_DEBUG_FILE__
	if (NULL == (mpOutput = fopen(MemusageFile ,"a"))) {
		printf("Failed to open Memusage.txt");
	} else {
		fprintf(mpOutput,"RC_FREE:\t%08x\tFile:%s      \tLineNum:\t%d\ttime:\t%d\tsize:\t%d\tMemtop:\t%d\n", 
			(unsigned int) pMemAddr, pFile, lineNum, mMemmgr.mTimeCode, 0,mMemmgr.mMemoryUsed);
		fclose(mpOutput);
	}
#endif

	mMemmgr.tblnow--;

    for (index = 0; index < kMaxMemMgrLeakTable; index++)
        if ((pMemAddr    == m_mallocTable[index].pMemAddr  ) &&
            (kActiveEntry & m_mallocTable[index].entryState) )
        {
			/* Release, but don't change hold flag */
            m_mallocTable[index].entryState &= ~kActiveEntry;
            m_mallocTable[index].entryState |=  kEmptyEntry;

            return;
        }

#ifdef __ENABLE_MEMMGR_DEBUG_SQUEEZE__	
	/* Not found; perhaps 'Squeeze out'below baselines? */
	/* Recalculate mMemmgr.tblnow & tbltop (in case small table overflowed) */
	mMemmgr.tblnow = 0;
	mMemmgr.tbltop = 0;
	for (index = 0; index < kMaxMemMgrLeakTable; index++)
		if (~kEmptyEntry & m_mallocTable[index].entryState) 
		{
			mMemmgr.tblnow++;
			mMemmgr.tbltop++;
		}
	return;
#endif /* __ENABLE_MEMMGR_DEBUG_SQUEEZE__ */

	/* Add error state RC_FREE NOT FOUND */
	AddMallocEntry(pMemAddr, pFileName, lineNum, 0, kErrorFreeEntry);
}


/*-----------------------------------------------------------------------*/

/* This is for adding messages and high-water marks in the allocation */
extern void RLI_DEBUG_MARK_ADD(sbyte *pBuffer, sbyte *pFile, int lineNum)
{   int index;

    MemMgr_LockFreeList();
	mMemmgr.mTimeCode++;

	/* Is this a new baseline? */
    if (STRNCMP(pBuffer,"BASE",4)==0) 
	{
#ifdef __ENABLE_MEMMGR_DEBUG_SQUEEZE__	
		/* Squeeze out 'static' allocs below this baseline */
		for (index = 0; index < kMaxMemMgrLeakTable; index++)
			if (kActiveEntry & m_mallocTable[index].entryState) 
			{
				m_mallocTable[index].entryState = kEmptyEntry;
			}
		/* Recalculate mMemmgr.tblnow & tbltop (in case small table overflowed) */
		mMemmgr.tblnow = 0;
		mMemmgr.tbltop = 0;
		for (index = 0; index < kMaxMemMgrLeakTable; index++)
			if (~kEmptyEntry & m_mallocTable[index].entryState) 
			{
				mMemmgr.tblnow++;
				mMemmgr.tbltop++;
			}
#endif /* __ENABLE_MEMMGR_DEBUG_SQUEEZE__ */

		/* set this as current timecode top */
		mMemmgr.tbltim = mMemmgr.mTimeCode; 
	}
	
	AddMallocEntry(pBuffer, pBuffer /*pFile*/, lineNum, 0, kMarkEntry);

	/* Is this a Alloc display ceiling? */
	if (STRNCMP(pBuffer,"CEIL",4)==0) 
	{
		mMemmgr.timmax = mMemmgr.mTimeCode;
	}

	MemMgr_UnlockFreeList();
}

extern void RLI_DEBUG_MARK_DEL(sbyte *pBuffer, sbyte *pFile, int lineNum)
{   int index;

    MemMgr_LockFreeList();
	mMemmgr.mTimeCode++;

    for (index = 0; index < kMaxMemMgrLeakTable; index++) 
	{
		if ((m_mallocTable[index].entryState & kMarkEntry    ) &&
		    (STRCMP(m_mallocTable[index].pMemAddr,pBuffer)==0) )
		{
			m_mallocTable[index].entryState = kEmptyEntry;
			mMemmgr.tblnow--;
			MemMgr_UnlockFreeList();
			return;
		}
	}

	MemMgr_UnlockFreeList();
}

#endif /* __ENABLE_MEMMGR_DEBUG__ */

/*-----------------------------------------------------------------------*/

#ifndef __OS_MALLOC_PROVIDED__

extern void Memmgr(int cmnd,struct MemmgrRec *pDbgRec)
{
	int		index;
    ubyte	key;

	switch (cmnd & __MEMMGR_CMND_MSK__)
	{
		case __MEMMGR_HOLD__ :
		    MemMgr_LockFreeList();
			/* copy contents of the MemRec */
			if (0 < pDbgRec->tbltim) mMemmgr.tbltim = pDbgRec->tbltim;
			*pDbgRec        = mMemmgr;
			pDbgRec->tblcnt =  0;
			pDbgRec->mrkcnt =  0;
			pDbgRec->errcnt =  0;
			for (index = 0; index < kMaxMemMgrLeakTable; index++)
			{
				if (kHoldEntry       &  m_mallocTable[index].entryState)
					m_mallocTable[index].entryState = kEmptyEntry;
				if ((kActiveEntry    &  m_mallocTable[index].entryState) &&
				    (pDbgRec->tbltim <= m_mallocTable[index].timeCode  ) && 
				    (pDbgRec->timmax >  m_mallocTable[index].timeCode  ) )
				{
					m_mallocTable[index].entryState |= kHoldEntry;
					pDbgRec->tblcnt++;
				}
				if ( kErrorEntry     &  m_mallocTable[index].entryState)
				{
					pDbgRec->errcnt++;
				}
				if ( kMarkEntry      &  m_mallocTable[index].entryState)
				{
					pDbgRec->mrkcnt++;
				}
			}
			MemMgr_UnlockFreeList();
			break;
		case __MEMMGR_READ__ :
			/* copy contents of the MemRec */
		    MemMgr_LockFreeList();
			*pDbgRec = mMemmgr;
			MemMgr_UnlockFreeList();
			break;
		case __MEMMGR_FIRST__ :
			/* Find First Held Allocation record */
			pDbgRec->tblptr = 0;
			/* continue ... */
		case __MEMMGR_NEXT__ :
			key = 0;
			if (cmnd & __MEMMGR_FIND_MEM__) key |= kHoldEntry;
			if (cmnd & __MEMMGR_FIND_MRK__) key |= kMarkEntry;
			if (cmnd & __MEMMGR_FIND_ERR__) key |= kErrorEntry;

			/* Find Next Held Allocation record */
			while ((pDbgRec->tblptr < kMaxMemMgrLeakTable                 )  &&
				   ((key & m_mallocTable[pDbgRec->tblptr].entryState) == 0) )
			{
				pDbgRec->tblptr++;
			}

			if (pDbgRec->tblptr < kMaxMemMgrLeakTable)
			{
				pDbgRec->table_entry = m_mallocTable[pDbgRec->tblptr];
				pDbgRec->tblptr++;
			} else {
				pDbgRec->table_entry.pFileName = "EOF";
				pDbgRec->table_entry.pMemAddr  = 0;
				pDbgRec->table_entry.lineNum   = 0;
				pDbgRec->table_entry.memLevel  = 0;
				pDbgRec->table_entry.timeCode  = 0;
				pDbgRec->table_entry.bufSize   = 0;
			}
				
			break;
		case __MEMMGR_RELEASE__	:
		    MemMgr_LockFreeList();
			for (index = 0; index < kMaxMemMgrLeakTable; index++) {
				if (kHoldEntry & m_mallocTable[index].entryState)
					m_mallocTable[index].entryState &= ~kHoldEntry;
				if ((kMarkEntry & m_mallocTable[index].entryState      ) &&
					(STRNCMP(m_mallocTable[index].pMemAddr,"CEIL",4)==0) )
				{
					m_mallocTable[index].entryState  =  kEmptyEntry;
					mMemmgr.tblnow--;
					mMemmgr.timmax = 999999;
				}
			}
			MemMgr_UnlockFreeList();
			break;
		case __MEMMGR_CLEAR__	:
		    MemMgr_LockFreeList();
			for (index = 0; index < kMaxMemMgrLeakTable; index++)
				if (kErrorEntry & m_mallocTable[index].entryState)
				{
					m_mallocTable[index].entryState = kEmptyEntry;
					mMemmgr.tblnow--;
				}
			MemMgr_UnlockFreeList();
			break;
	}

}

#endif /* __OS_MALLOC_PROVIDED__ */

/*-----------------------------------------------------------------------*/

#ifdef __OS_MALLOC_PROVIDED__



/*-----------------------------------------------------------------------*/

#ifdef __ENABLE_DANGLING_PTR_TESTING__

static void LockDanglingWatchList()
{
	OS_SPECIFIC_MUTEX_WAIT(mDanglingMutex);
}

static void UnlockDanglingWatchList()
{
	OS_SPECIFIC_MUTEX_RELEASE(mDanglingMutex);
}

static void AddToMemoryDanglingList(void *pAddBlock, sbyte4 addBlockSize, sbyte *pFile, sbyte4 lineNum)
{
	LockDanglingWatchList();

	if (kMaxDanglingEntries > mNumDanglingEntries)
	{
		danglingWatchList[mNumDanglingEntries].pMemBlock     = pAddBlock;
		danglingWatchList[mNumDanglingEntries].memBlockSize  = addBlockSize;
		danglingWatchList[mNumDanglingEntries].memBlockState = kAllocActive;
		danglingWatchList[mNumDanglingEntries].pFilename     = pFile;
		danglingWatchList[mNumDanglingEntries].lineNum       = lineNum;

		mNumDanglingEntries++;
	}

	UnlockDanglingWatchList();

	return;
}

static void WatchMemoryDanglingList(void *pWatchBlock)
{
	sbyte4 index;
    sbyte* pMem;

	LockDanglingWatchList();

	for (index = 0; index < mNumDanglingEntries; index++)
	{
        pMem = danglingWatchList[index].pMemBlock;

		if (pMem == pWatchBlock)
		{
			if (kAllocInactive == danglingWatchList[index].memBlockState)
			{
				printf("WatchMemoryDanglingList: Double Free Bug!\n");		/*!!!!!!!! place break point here. you've found a bug! JAB */
			}
			else
			{
				danglingWatchList[index].memBlockState = kAllocInactive;

				/* clear out memory block */
				memset(pWatchBlock, kDanglingClearByte, danglingWatchList[index].memBlockSize);
			}

			break;
		}
	}

	UnlockDanglingWatchList();

	return;
}

extern void MEMMGR_CheckMemoryWatch()
{
	int loop, index, size;
    sbyte* pMem;
    sbyte val1, val2;

    val1 = (sbyte) kDanglingClearByte;

	LockDanglingWatchList();

	for (loop = 0; loop < mNumDanglingEntries; loop++)
	{
        pMem = danglingWatchList[loop].pMemBlock;
        size = danglingWatchList[loop].memBlockSize;

		if (kAllocInactive == danglingWatchList[loop].memBlockState)
    		for (index = 0; index < size; index++)
            {
                val2 = pMem[index];

			    if (val1 != val2)
			    {
				    /*!!!!!!!!!!! place break point here. congratulations, you've found a dangling pointer bug! JAB */
				    printf("\nDETECTED DANGLING POINTER: addr = (%08x), size = (%d), file = (%s), line = (%d).\n", danglingWatchList[loop].pMemBlock, danglingWatchList[loop].memBlockSize, danglingWatchList[loop].pFilename, danglingWatchList[loop].lineNum);
				    break;
			    }
            }
	}

	UnlockDanglingWatchList();
}

void MEMMGR_FlushDanglingWatchList()
{
	int loop;

	MEMMGR_CheckMemoryWatch();

	LockDanglingWatchList();

	for (loop = 0; loop < mNumDanglingEntries; loop++)
		OS_SPECIFIC_FREE(danglingWatchList->pMemBlock);

	mNumDanglingEntries = 0;

	UnlockDanglingWatchList();
}

#endif /* __ENABLE_DANGLING_PTR_TESTING__ */



/*-----------------------------------------------------------------------*/

#if  !defined(__ENABLE_MEMMGR_DEBUG__) || defined(__MIBWAY_TUTORIAL__)

#ifdef __ENABLE_DANGLING_PTR_TESTING__
#undef RC_MALLOC
#endif

void *RC_MALLOC(Length memSize)
{
    return OS_SPECIFIC_MALLOC( memSize );
}
#endif

#if ((defined(__ENABLE_MEMMGR_DEBUG__)) || (defined(__ENABLE_DANGLING_PTR_TESTING__)))
extern void *RLI_DEBUG_MALLOC(Length memSize, sbyte *pFile, int lineNum)
{
#ifdef __ENABLE_DANGLING_PTR_TESTING__
	void *pMemBlock = OS_SPECIFIC_MALLOC( memSize );

	AddToMemoryDanglingList(pMemBlock, memSize, pFile, lineNum);

	return pMemBlock;

#else

    void *pMem = OS_SPECIFIC_MALLOC( memSize );
    AddMallocEntry(pMem, pFile, lineNum, memSize, kActiveEntry);
    return pMem;

#endif
}
#endif

/*-----------------------------------------------------------------------*/

#if ((!defined(__ENABLE_MEMMGR_DEBUG__)) && (!defined(__ENABLE_DANGLING_PTR_TESTING__)))
extern void *RC_CALLOC(ubyte4 num, Length size)
#else
extern void *RLI_DEBUG_CALLOC(ubyte4 num, Length size, sbyte *pFile, int lineNum)
#endif
{
    Length  memSize;
    void    *pMem;

    memSize = num * size;
    pMem    = OS_SPECIFIC_MALLOC( memSize );

#ifdef __ENABLE_DANGLING_PTR_TESTING__
	AddToMemoryDanglingList(pMem, memSize, pFile, lineNum);
#endif

    if ( NULL == pMem )
        return pMem;

#ifdef  __ENABLE_MEMMGR_DEBUG__
    AddMallocEntry(pMem, pFile, lineNum, memSize, kActiveEntry);
#endif

    MEMSET( pMem, 0, memSize );

    return pMem;
}
 
/*-----------------------------------------------------------------------*/

#if  !defined(__ENABLE_MEMMGR_DEBUG__) || defined(__MIBWAY_TUTORIAL__)
extern void RC_FREE(void *pBuffer)
{   
#ifdef __ENABLE_DANGLING_PTR_TESTING__
	WatchMemoryDanglingList(pBuffer);
#else
    OS_SPECIFIC_FREE( pBuffer );
#endif
}

#endif

#ifdef  __ENABLE_MEMMGR_DEBUG__
extern void RLI_DEBUG_FREE(void *pBuffer, sbyte *pFile, int lineNum)
{   
    RemoveMallocEntry(pBuffer, pFile, lineNum);

    OS_SPECIFIC_FREE( pBuffer );
}
  
#endif

#endif /* __OS_MALLOC_PROVIDED__ */

/*-----------------------------------------------------------------------*/

#ifndef __OS_MALLOC_PROVIDED__

extern ubyte4
MemMgr_GetMemoryUsage ( void )
{
    return(mMemmgr.mMemoryUsed);
}

#endif

/*-----------------------------------------------------------------------*/

#ifndef __OS_MALLOC_PROVIDED__

static void MemMgr_RoundDown8(ubyte4 *p_memSize)
{
    *p_memSize = (*p_memSize) & 0xfffffff8;
}

#endif

/*-----------------------------------------------------------------------*/

extern RLSTATUS
MemMgr_Init ( void )
{
    RLSTATUS      status = OK;

#ifndef __OS_MALLOC_PROVIDED__
    ubyte4      memSize = kInitialMemoryAllocation;
    MemBlock    *pMemPtr;
    MemBlock    *pInitialBlk;

    /* memory has to be allocated in pieces of 8 */
    MemMgr_RoundDown8(&memSize);

    if ( memSize < sizeof(MemBlock) )
        return ERROR_MEMMGR_BAD_MEMSIZE;

    if (NULL == (pMemPtr = OS_SPECIFIC_MALLOC(memSize)))
        return ERROR_MEMMGR_INITIALIZATION;

    /* Initialize the free list, including the sentinel 
     * and the first block
     */
    pmMemFreeListHead               = (MemBlock*)pMemPtr;
    pmMemFreeListHead->memBlkLen    = 0; /*pmMemFreeListHead is a sentinel.*/
    pmMemFreeListHead->pNextMemBlk  = (MemBlock*)pMemPtr + 1;

    pInitialBlk                     = (MemBlock*)pMemPtr + 1;
    pInitialBlk->pNextMemBlk        = NULL;
    pInitialBlk->memBlkLen          = memSize - sizeof(MemBlock);

    /* Initialize the other relevant markers */
    mMaxAddr = (ubyte4)pMemPtr + memSize;
    mEndAddr = (ubyte4)pInitialBlk;

    /* protection for free list in a multithreaded environment */
    status = OS_SPECIFIC_MUTEX_CREATE( &mMutex );

    mMemmgr.mMemoryUsed = 0;
    mMemmgr.mMemoryTop  = 0;
    mMemmgr.mMemoryMax  = kInitialMemoryAllocation;
#endif /* __OS_MALLOC_PROVIDED__ */

#ifdef  __ENABLE_MEMMGR_DEBUG__
    MEMMGR_InitDebug();
#endif

#ifdef __ENABLE_DANGLING_PTR_TESTING__
    OS_SPECIFIC_MUTEX_CREATE( &mDanglingMutex );
#endif

    return status;

} /* MemMgr_Init */

#ifndef __OS_MALLOC_PROVIDED__
 
/*-----------------------------------------------------------------------*/

static void
MemMgr_PrepareBlock ( void *pMemBlk, ubyte4 memSize, void **ppMemPtr )
{
    ubyte4      *pLenMark, *pMagicMark, *pReturnPtr;

    pLenMark    = (ubyte4*)pMemBlk;
    *pLenMark   = memSize;

    pMagicMark  = (ubyte4*)( (sbyte*)pMemBlk + memSize - sizeof(ubyte4) );
    *pMagicMark = kMagicMarker;

    pReturnPtr  = pLenMark + 1;
    *ppMemPtr   = pReturnPtr;
    
}

/*-----------------------------------------------------------------------*/

static RLSTATUS
MemMgr_SanityCheck ( void *pMem )
{
    if ((NULL == pMem                                   ) || 
        ((ubyte4)pMem < (mEndAddr +    sizeof(ubyte4))  ) ||
        ((ubyte4)pMem > (mMaxAddr - (2*sizeof(ubyte4))) )    )
    {
        return ERROR_MEMMGR_BAD_FREE;
    }

    return OK;
}

/*-----------------------------------------------------------------------*/

static RLSTATUS
MemMgr_RetrieveMemStart ( void *pMem, void **ppMemStart )
{
    *ppMemStart = (ubyte4*)pMem - 1;

    return OK;
}

/*-----------------------------------------------------------------------*/

static RLSTATUS
MemMgr_RetrieveLength ( void *pMemPtr, ubyte4 *pMemSize )
{
    *pMemSize = *( (ubyte4*)pMemPtr );
    
    return OK;
}

/*-----------------------------------------------------------------------*/

static RLSTATUS
MemMgr_CheckMemory ( void *pMemStart, ubyte4 memSize )
{
    ubyte4  *pMagicMarker;

    if (( 0 == memSize                  ) || 
        ( memSize > (mMaxAddr - mEndAddr))   )
    {
        return ERROR_MEMMGR_INVALID_LENGTH;
    }

    pMagicMarker = (ubyte4*)( (sbyte*)pMemStart + memSize - sizeof( ubyte4 ) );

    if ( kMagicMarker != *pMagicMarker )
        return ERROR_MEMMGR_MEMORY_CORRUPTION;

    return OK;
}
 
/*-----------------------------------------------------------------------*/

static void
MemMgr_CalcFullMemSize ( ubyte4 reqMemSize, ubyte4 *pFullMemSize )
{
    ubyte4 nextEvenDword;

    /* Don't forget space for both the length and the magic marker */
    nextEvenDword  = reqMemSize + (2 * sizeof(ubyte4));

    /* !!!!! The code that follows, below, rounds to the nearest multiple
     * of eight.  If you eliminate the code below, you need to
     * modify the line marked below, in the function MemMgr_GetMemory().
     */
    /* round up result to next even DWord (64-bits) */
    nextEvenDword  = (7 + nextEvenDword) & 0xFFFFFFF8;

    *pFullMemSize = nextEvenDword;
}

/*-----------------------------------------------------------------------*/

static RLSTATUS
MemMgr_GetMemory ( ubyte4 reqMemSize, void **ppMemPtr )
{
    MemBlock *pCurrBlk, *pPrevBlk, *pLeftOver;
    ubyte4   fullMemSize;

#ifdef __ENABLE_MEMMGR_DEBUG__
	if (MEMMGR_SimFail()) 
        return ERROR_MEMMGR_NO_MEMORY; 
#endif

    if ( reqMemSize == 0 ) 
        return ERROR_MEMMGR_BAD_MEMSIZE;

    if ( pmMemFreeListHead == NULL )
        return ERROR_MEMMGR_GENERAL;

    if ( pmMemFreeListHead->pNextMemBlk == NULL )
        return ERROR_MEMMGR_NO_MEMORY; 

    MemMgr_CalcFullMemSize( reqMemSize, &fullMemSize );

    mMemmgr.mMemoryUsed += fullMemSize;
    pPrevBlk    = pmMemFreeListHead;
    pCurrBlk    = pmMemFreeListHead->pNextMemBlk;

	/* Track top of used memory */
    if (mMemmgr.mMemoryTop < mMemmgr.mMemoryUsed)
        mMemmgr.mMemoryTop = mMemmgr.mMemoryUsed;

    while ( pCurrBlk != NULL )
    {
        /* !!!!! The line of code below should read
         * if (pCurrBlk->memBlkLen > (fullMemSize + sizeof(MemBlock)))
         *
         * Otherwise, modifying the code beyond the memory allocation 
         * (which you do when you initialize the pLeftOver pointer)
         * could lead to you stomping on the next memory blk.
         *
         * This modification will happen for OC 3.0.  The only reason it's
         * not in OC 2.31 is because the roundup line in MemMgr_CalcFullMemSize()
         * obviates this problem because all memory locations are in 
         * quantums of 8 bytes, so no stomping can occur (the next memory block
         * is guaranteed to be at least 8 bytes away from the end of the current
         * block, and won't be stomped on by the updating of the pointers, since the
         * pointers themselves are part of an 8 byte structure 
         * -- i.e. sizeof(MemBlock) = 8).
         *
         * Because the stompings are guaranteed not to occur -- i.e. the code works 
         * -- and because modifying this line will require a full-blown QA of 
         * the Memory Manager (again), along with unit testing, I decided to 
         * forgo with the change at this time. KW 4/22/99  
         */
        if ( pCurrBlk->memBlkLen > fullMemSize )
        {
            pLeftOver = (MemBlock*)( (ubyte4)pCurrBlk + fullMemSize );
            pPrevBlk->pNextMemBlk  = pLeftOver;

            pLeftOver->pNextMemBlk = pCurrBlk->pNextMemBlk;
            pLeftOver->memBlkLen   = pCurrBlk->memBlkLen - fullMemSize;
            
            MemMgr_PrepareBlock( pCurrBlk, fullMemSize, ppMemPtr );

            return OK;
        }
        else if ( pCurrBlk->memBlkLen == fullMemSize )
        {
            pPrevBlk->pNextMemBlk = pCurrBlk->pNextMemBlk;

            MemMgr_PrepareBlock( pCurrBlk, fullMemSize, ppMemPtr );

            return OK;
        }

        pPrevBlk = pCurrBlk;
        pCurrBlk = pCurrBlk->pNextMemBlk;
    }

    mMemmgr.mMemoryUsed -= fullMemSize;
    return ERROR_MEMMGR_NO_MEMORY;

} /* MemMgr_GetMemory */

/*-----------------------------------------------------------------------*/

static RLSTATUS
MemMgr_FreeMemory ( void *pMem )
{
    MemBlock    *pCurrBlk, *pPrevBlk, *pNewlyFreedBlock;
    ubyte4      topOfBlock, blockEnd, memSize;
    void        *pMemStart;
    RLSTATUS      status;

    if (NULL == pmMemFreeListHead)
        return ERROR_MEMMGR_GENERAL;

    status = MemMgr_SanityCheck(pMem);
    if ( OK > status )
        return status;

    status = MemMgr_RetrieveMemStart( pMem, &pMemStart );
    if ( OK > status )
        return status;

    status = MemMgr_RetrieveLength( pMemStart, &memSize );  
    if ( OK > status )
        return status;

    status = MemMgr_CheckMemory( pMemStart, memSize );
    if ( OK > status )
        return status;
    
    pPrevBlk = pmMemFreeListHead; 
    pCurrBlk = pmMemFreeListHead->pNextMemBlk;

    while ((NULL != pCurrBlk                    ) &&
           ((ubyte4)pCurrBlk < (ubyte4)pMemStart)    )
	{
		pPrevBlk = pCurrBlk;
		pCurrBlk = pCurrBlk->pNextMemBlk;
	}

	topOfBlock = (ubyte4)pPrevBlk + pPrevBlk->memBlkLen;

	if (( topOfBlock > (ubyte4)pMemStart) &&
        ( pPrevBlk  != pmMemFreeListHead)    )
	{
		return ERROR_MEMMGR_BAD_POINTER;
	}

	blockEnd = (ubyte4)pMemStart + memSize;

	if ((NULL != pCurrBlk           ) &&
        (blockEnd > (ubyte4)pCurrBlk)    )
	{
		return ERROR_MEMMGR_BAD_POINTER;
	}

	pNewlyFreedBlock = (MemBlock*)pMemStart;

	if (    ( pPrevBlk   != pmMemFreeListHead           )
        &&  ( topOfBlock == (ubyte4)pNewlyFreedBlock    )   )
	{
		pPrevBlk->memBlkLen += memSize;
	}
	else
	{
		pNewlyFreedBlock->memBlkLen     = memSize;
		pNewlyFreedBlock->pNextMemBlk   = pCurrBlk;
		pPrevBlk->pNextMemBlk           = pNewlyFreedBlock;
		pPrevBlk                        = pNewlyFreedBlock;
	}

	topOfBlock = (ubyte4)pPrevBlk + pPrevBlk->memBlkLen;

	if ( topOfBlock == (ubyte4)pCurrBlk )
	{
		pPrevBlk->memBlkLen += pCurrBlk->memBlkLen;
		pPrevBlk->pNextMemBlk = pCurrBlk->pNextMemBlk;
	}

	mMemmgr.mMemoryUsed -= memSize;
	return OK;

} /* MemMgr_FreeMemory */ 

/*-----------------------------------------------------------------------*/

/* Allow RC_FREE and RC_MALLOC for ENVOYSAM.LIB */
#if  !defined(__ENABLE_MEMMGR_DEBUG__) || defined(__MIBWAY_TUTORIAL__)
#define REAL_MALLOC RC_MALLOC
#undef RC_MALLOC
extern void *RC_MALLOC(Length memSize)
{
    RLSTATUS status;
    void   *pMem;
    char   *pMsg;

	MemMgr_LockFreeList();
	status = MemMgr_GetMemory( memSize, &pMem );
	MemMgr_UnlockFreeList();

	if ( OK > status )
	{
		MsgHdlr_RetrieveOpenControlMessage( ERROR_MEMMGR_NO_MEMORY, &pMsg );
		OS_SPECIFIC_LOG_ERROR(kUnrecoverableError, pMsg);
		return NULL;
	}

	return pMem;
}
#define RC_MALLOC REAL_MALLOC
#endif

#ifdef  __ENABLE_MEMMGR_DEBUG__
extern void *RLI_DEBUG_MALLOC(Length memSize, sbyte *pFile, int lineNum)
{
    RLSTATUS status;
    void   *pMem;
    char   *pMsg;

	MemMgr_LockFreeList();
	status = MemMgr_GetMemory( memSize, &pMem );
#ifdef  __ENABLE_MEMMGR_DEBUG__
	if ( OK > status ) AddMallocEntry(pMem, pFile, lineNum, memSize, kErrorGetEntry);
		else           AddMallocEntry(pMem, pFile, lineNum, memSize, kActiveEntry);
#endif
	MemMgr_UnlockFreeList();

	if ( OK > status )
	{
		MsgHdlr_RetrieveOpenControlMessage( ERROR_MEMMGR_NO_MEMORY, &pMsg );
		OS_SPECIFIC_LOG_ERROR(kUnrecoverableError, pMsg);
		return NULL;
	}

	return pMem;
}
#endif

/*-----------------------------------------------------------------------*/

#ifndef  __ENABLE_MEMMGR_DEBUG__
extern void *RC_CALLOC(ubyte4 num, Length size)
#else
extern void *RLI_DEBUG_CALLOC(ubyte4 num, Length size, sbyte *pFile, int lineNum)
#endif
{
	RLSTATUS  status;
	ubyte4  memSize;
	void    *pMem;
	char    *pMsg;

	memSize = num * size;

	MemMgr_LockFreeList();
	status = MemMgr_GetMemory( memSize, &pMem );
#ifdef  __ENABLE_MEMMGR_DEBUG__
	if ( OK > status ) AddMallocEntry(pMem, pFile, lineNum, memSize, kErrorGetEntry);
		else           AddMallocEntry(pMem, pFile, lineNum, memSize, kActiveEntry);
#endif
	MemMgr_UnlockFreeList();

	if ( OK > status )
	{
		MsgHdlr_RetrieveOpenControlMessage(ERROR_MEMMGR_NO_MEMORY, &pMsg);
		OS_SPECIFIC_LOG_ERROR(kUnrecoverableError, pMsg);
		return NULL;
	}

	MEMSET( pMem, 0, memSize );

	return pMem;
}

/*-----------------------------------------------------------------------*/

/* Allow RC_FREE and RC_MALLOC for ENVOYSAM.LIB */
#if  !defined(__ENABLE_MEMMGR_DEBUG__) || defined(__MIBWAY_TUTORIAL__)
#define REAL_FREE RC_FREE
#undef RC_FREE
extern void RC_FREE ( void *pBuffer )
{   
    RLSTATUS status;
    char    *pMsg;

    MemMgr_LockFreeList();
    status = MemMgr_FreeMemory( pBuffer );
    MemMgr_UnlockFreeList();

    if ( OK > status )
    {
        MsgHdlr_RetrieveOpenControlMessage(ERROR_MEMMGR_BAD_FREE, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kUnrecoverableError, pMsg);
    }
}
#define RC_FREE REAL_FREE
#endif

#ifdef  __ENABLE_MEMMGR_DEBUG__
extern void RLI_DEBUG_FREE(void *pBuffer, sbyte *pFile, int lineNum)
{   
    RLSTATUS status;
    char    *pMsg;

    MemMgr_LockFreeList();
    RemoveMallocEntry(pBuffer, pFile, lineNum);
    status = MemMgr_FreeMemory( pBuffer );
    MemMgr_UnlockFreeList();

    if ( OK > status )
    {
        MsgHdlr_RetrieveOpenControlMessage(ERROR_MEMMGR_BAD_FREE, &pMsg);
        OS_SPECIFIC_LOG_ERROR(kUnrecoverableError, pMsg);
    }
}
#endif

#endif /* !__OS_MALLOC_PROVIDED__ */
