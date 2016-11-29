/*  
 *  rc_filemgr.c
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

$History: rc_filemgr.c $
 * 
 * *****************  Version 19  *****************
 * User: Pstuart      Date: 8/22/00    Time: 5:33p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Use OS specific malloc/free for decompression of data
 * 
 * *****************  Version 18  *****************
 * User: Leech        Date: 6/21/00    Time: 11:49a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments
 * 
 * *****************  Version 17  *****************
 * User: Dreyna       Date: 6/19/00    Time: 4:48p
 * Added fudge fix to rc_filearray for PSOS.
 * 
 * *****************  Version 16  *****************
 * User: Pstuart      Date: 5/11/00    Time: 12:32p
 * made c++ comment ansi c style
 * 
 * *****************  Version 15  *****************
 * User: Epeterson    Date: 4/25/00    Time: 5:22p
 * Include history and enable auto archiving feature from VSS


*/
#include "rc_options.h"

#ifdef __FILE_MANAGER_ENABLED__

#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_dc_defs.h"
#include "rc_inflate.h"
#include "rc_decomp.h"

/*-----------------------------------------------------------------------*/

#define __RLI_PARTITION_FULL__
#ifdef __RLI_PARTITION_FULL__

/* Setup tables for NEW multiple file arrays */
sbyte4 nPartitionCount;
extern sbyte *rc_filearray[];

#else /* !__RLI_PARTITION_FULL__ */

/* Setup tables for multiple file arrays if any */
#ifndef __RLI_FILEMGR_PARTITION_COUNT__ 
#define nPartitionCount  1
extern char rc_filesys[];
static char *rc_filearray[] =
{	kNVRamStart		/* this is usually set to rc_filesys in options.h */ 
};

#else /* nPartitionCount */

/* Setup the search table that will automatically link to the requested file arrays */
#define nPartitionCount  __RLI_FILEMGR_PARTITION_COUNT__
extern char rc_filesys1[];
#if (__RLI_FILEMGR_PARTITION_COUNT__ > 1)
extern char rc_filesys2[];
#else
#define     rc_filesys2 NULL
#endif
#if (__RLI_FILEMGR_PARTITION_COUNT__ > 2)
extern char rc_filesys3[];
#else
#define     rc_filesys3 NULL
#endif
#if( __RLI_FILEMGR_PARTITION_COUNT__ > 3)
extern char rc_filesys4[];
#else
#define     rc_filesys4 NULL
#endif
#if( __RLI_FILEMGR_PARTITION_COUNT__ > 4)
extern char rc_filesys5[];
#else
#define     rc_filesys5 NULL
#endif
#if( __RLI_FILEMGR_PARTITION_COUNT__ > 5)
extern char rc_filesys6[];
#else
#define     rc_filesys6 NULL
#endif
#if( __RLI_FILEMGR_PARTITION_COUNT__ > 6)
extern char rc_filesys7[];
#else
#define     rc_filesys7 NULL
#endif
#if( __RLI_FILEMGR_PARTITION_COUNT__ > 7)
extern char rc_filesys8[];
#else
#define     rc_filesys8 NULL
#endif
static sbyte *rc_filearray[] =
{	rc_filesys1, 
	rc_filesys2,
	rc_filesys3,
	rc_filesys4,
	rc_filesys5,
	rc_filesys6,
	rc_filesys7,
	rc_filesys8,
	NULL
};
#endif	/* nPartitionCount */

#endif /* __RLI_PARTITION_FULL__ */

/*-----------------------------------------------------------------------*/

/* If the File System is compiled as a C source file, then the File Manager
 * and the compiler need to know where to look for it. */
#ifdef __RLI_PARTITION_FULL__
static  sbyte **mFlashMemoryStartArray = NULL;
static  sbyte **mFileMgrDirectoryArray = NULL;
static  ubyte4 *mFileMgrSizeArray      = NULL;
#else
static  sbyte *mFlashMemoryStartArray[nPartitionCount];
static  sbyte *mFileMgrDirectoryArray[nPartitionCount];
static  ubyte4 mFileMgrSizeArray[     nPartitionCount];
#endif

static  sbyte4 mFileSystemVersionNum;

#define kFileSystemVersion1         1
#define kUniqueSignatureLength      16
#define kMaxFileNameLen             64

typedef struct  DirectoryEntry          /* (basename:  dir) */
{
    sbyte   fileName[kMaxFileNameLen];
    sbyte4  fileSize;
    sbyte4  fileOffset;                 /* from the base addr of the file system */
    sbyte4  fileCheckSum;

} DirectoryEntry;



/*-----------------------------------------------------------------------*/

static Boolean FILEMGR_CheckUniqueSignature(sbyte *pMessageCheck)
{
    if (0 == STRNCMP(pMessageCheck, kUniqueSignature, kUniqueSignatureLength))
        return TRUE;

    return FALSE;
}


/*-----------------------------------------------------------------------*/

extern int size_of_ocsys;

static void FILEMGR_doConstruct(sbyte* RamStart, sbyte4 nTableIndex)
{
    sbyte*  StartFlashMemory;
    sbyte*  FileMgrBaseAddr;
    Boolean FoundSig = FALSE;
    sbyte4          NumFilesInDir;
    DirectoryEntry* p_dirDesc;
    sbyte4*         pFileDirectory;

    StartFlashMemory = RamStart;
    FileMgrBaseAddr  = RamStart;

    mFileSystemVersionNum               = kFileSystemVersion1;
    mFlashMemoryStartArray[nTableIndex] = StartFlashMemory;

    while (FileMgrBaseAddr < (StartFlashMemory + kNVRamSize))
    {
        if (TRUE == FILEMGR_CheckUniqueSignature(FileMgrBaseAddr))
            if (TRUE == (FoundSig = FILEMGR_CheckUniqueSignature(&FileMgrBaseAddr[kUniqueSignatureLength])))
            {
                mFileMgrDirectoryArray[nTableIndex] = FileMgrBaseAddr;
                break;
            }

        FileMgrBaseAddr += kSectorSize;
    }

    if (TRUE != FoundSig) 
	{
        mFileMgrDirectoryArray[nTableIndex] = NULL;
	} else {
		/* Calculate true size of this array buffer for correct bounds check in FileRelease() */
	    pFileDirectory = (sbyte4 *)(&((mFileMgrDirectoryArray[nTableIndex])[2 * kUniqueSignatureLength]));
	    NumFilesInDir  = pFileDirectory[1];
		if (0<NumFilesInDir)
		{
			p_dirDesc      = (DirectoryEntry *)(&pFileDirectory[2]);
			p_dirDesc      = &p_dirDesc[NumFilesInDir-1];
			mFileMgrSizeArray[nTableIndex] = ((sbyte4) p_dirDesc->fileOffset) + p_dirDesc->fileSize;
		} else {
			p_dirDesc      = NULL;
			p_dirDesc      = NULL;
			mFileMgrSizeArray[nTableIndex] = 0;
		}
	}
}



/*-----------------------------------------------------------------------*/

static DirectoryEntry *GetFileRecord(sbyte *fileName, sbyte *mFileMgrDirectory)
{
    sbyte4*         pFileDirectory;
    sbyte4          NumFilesInDir;
    DirectoryEntry* p_dirDesc;

    if ((NULL == fileName) || (NULL == mFileMgrDirectory))
        return NULL;

    pFileDirectory = (sbyte4 *)(&(mFileMgrDirectory[2 * kUniqueSignatureLength]));

    if (mFileSystemVersionNum != *pFileDirectory)
    {
        /* as we extend the file system we'll add additional versions here. JB */

        /* not able to recognize the version, exit */
        return NULL;
    }

    NumFilesInDir = pFileDirectory[1];

    /* set p_dirDesc to the start of the directory table */
    p_dirDesc     = (DirectoryEntry *)(&pFileDirectory[2]);

    while (0 < NumFilesInDir)
    {
#ifdef __CASE_INSENSITIVE_FILENAMES__
        if (0 == STRICMP(p_dirDesc->fileName, fileName))
#else
        if (0 == STRCMP(p_dirDesc->fileName, fileName))
#endif
            return p_dirDesc;
        
        p_dirDesc++;
        NumFilesInDir--;
    }

    return NULL;

}  /* GetFileRecord */



/*-----------------------------------------------------------------------*/

static void *FILEMGR_doRetrieveFile(int *pFileLen, sbyte *fileName, sbyte *mFileMgrDirectory)
{
    DirectoryEntry* p_dirDesc;
    void*           pFile        = NULL;
    int             FileLen      = 0;
    sbyte*          pRetPtr      = NULL;

#ifdef __DECOMPRESSION_ENABLED__
    sbyte*          pData        = NULL;
    Boolean         isCompressed = FALSE;
    sbyte*          pDecompressedData;
    sbyte4          decompressedLen;
#endif /* __DECOMPRESSION_ENABLED__ */

    #define RETRIEVE_FILE_CLEANUP(arg)  {                       \
                                            pRetPtr = arg;      \
                                            goto cleanup;       \
                                        }
    *pFileLen    = 0;

    if (NULL == (p_dirDesc = GetFileRecord(fileName,mFileMgrDirectory)))
        RETRIEVE_FILE_CLEANUP(NULL);

    pFile   = (void *)(&mFileMgrDirectory[p_dirDesc->fileOffset]);
    FileLen = p_dirDesc->fileSize;

#ifdef __DECOMPRESSION_ENABLED__

    /* If the file is compressed, then use the decompression code.  Otherwise,
     * just return the data as is. */
    DECOMP_IsCompressed(pFile, &isCompressed);

    if (isCompressed)
    {
        pData = (sbyte*) OS_SPECIFIC_MALLOC(FileLen);

        if (NULL == pData)
            RETRIEVE_FILE_CLEANUP(NULL);

        MEMCPY(pData, pFile, FileLen);

        DECOMP_DecompressData(pData, FileLen, &pDecompressedData, &decompressedLen);

        *pFileLen = decompressedLen;
        RETRIEVE_FILE_CLEANUP(pDecompressedData);
    }
    else
    {
        *pFileLen = FileLen;
        RETRIEVE_FILE_CLEANUP(pFile);
    }
#else
    *pFileLen = FileLen;
    RETRIEVE_FILE_CLEANUP(pFile);

#endif /* __DECOMPRESSION_ENABLED__ */

cleanup:

#ifdef __DECOMPRESSION_ENABLED__
    if (isCompressed)
        if (NULL != pData)
            OS_SPECIFIC_FREE(pData);
#endif /* __DECOMPRESSION_ENABLED__ */

    return pRetPtr;

} /* FILEMGR_doRetrieveFile */

/*-----------------------------------------------------------------------*/

extern void FILEMGR_ReleaseFile(void *pFile)
{
#ifdef __DECOMPRESSION_ENABLED__
	int i;

	for (i=0;i<nPartitionCount;i++)
	{
		if ( ((ubyte4)mFileMgrDirectoryArray[i]                         <= (ubyte4)pFile) &&
			(((ubyte4)mFileMgrDirectoryArray[i] + mFileMgrSizeArray[i]) >  (ubyte4)pFile) )
		{	/* pFile found inside one of the file arrays */
			return;
		}
	}

    /* outside of all static memory arrays (flash memory), so free the buffer. */
    OS_SPECIFIC_FREE(pFile);

#endif /* __DECOMPRESSION_ENABLED__ */

}

/*-----------------------------------------------------------------------*/

static void *FILEMGR_doRetrieveResource(int *pFileLen, sbyte *fileName, sbyte *mFileMgrDirectory)
{
    DirectoryEntry* p_dirDesc;
    void*           pFile        = NULL;
    int             FileLen      = 0;
    sbyte*          pRetPtr      = NULL;

    #define RETRIEVE_FILE_CLEANUP(arg)  {                       \
                                            pRetPtr = arg;      \
                                            goto cleanup;       \
                                        }
    *pFileLen    = 0;

    if (NULL == (p_dirDesc = GetFileRecord(fileName,mFileMgrDirectory)))
        RETRIEVE_FILE_CLEANUP(NULL);

    pFile   = (void *)(&mFileMgrDirectory[p_dirDesc->fileOffset]);
    FileLen = p_dirDesc->fileSize;

    *pFileLen = FileLen;
    RETRIEVE_FILE_CLEANUP(pFile);

cleanup:

    return pRetPtr;

} /* FILEMGR_doRetrieveFile */



/*-----------------------------------------------------------------------*/

extern void FILEMGR_ReleaseResource(void *pFile)
{
    return;
}

/*-----------------------------------------------------------------------
 *
 * Methods for handling Multiple File Array Partitions
 *
 */

#ifdef __PSOS_OS__
extern const unsigned char rc_filesys[];
#endif

extern void FILEMGR_Construct(void)
{	int  i;

#ifdef __PSOS_OS__
	if (NULL == rc_filearray[0])
	{
		rc_filearray[0] = (void *) rc_filesys;
		rc_filearray[1] = NULL;
	}
#endif

#ifdef __RLI_PARTITION_FULL__
	/* Find the partition count */
	for (nPartitionCount = 0;rc_filearray[nPartitionCount];nPartitionCount++);
	mFlashMemoryStartArray = (sbyte **) RC_CALLOC(nPartitionCount,sizeof(sbyte *));
	mFileMgrDirectoryArray = (sbyte **) RC_CALLOC(nPartitionCount,sizeof(sbyte *));
	mFileMgrSizeArray      = (ubyte4 *) RC_CALLOC(nPartitionCount,sizeof(ubyte4 ));
#endif

	for (i=0;i<nPartitionCount;i++)
	{
		FILEMGR_doConstruct(rc_filearray[i],i);
	}

}

extern void FILEMGR_DeConstruct(void)
{	int  i;

#ifdef __RLI_PARTITION_FULL__
	/* Delete the partition arrays */
	if (mFlashMemoryStartArray) RC_FREE(mFlashMemoryStartArray);
	if (mFileMgrDirectoryArray) RC_FREE(mFileMgrDirectoryArray);
	if (mFileMgrSizeArray     ) RC_FREE(mFileMgrSizeArray     );
#endif

	for (i=0;i<nPartitionCount;i++)
	{
/*		FILEMGR_deConstruct(rc_filearray[i],i); */
	}

}

extern void *FILEMGR_RetrieveFile(int *pFileLen, sbyte *fileName)
{	int  i;
	void *ret = NULL;

	for (i=0;i<nPartitionCount;i++)
	{
		if (NULL != (ret=FILEMGR_doRetrieveFile(pFileLen,fileName,mFileMgrDirectoryArray[i])))
			return(ret);
	}
	return(ret);
}

extern void *FILEMGR_RetrieveResource(int *pFileLen, sbyte *fileName)
{	int  i;
	void *ret = NULL;

	for (i=0;i<nPartitionCount;i++)
	{
		if (NULL != (ret=FILEMGR_doRetrieveResource(pFileLen,fileName,mFileMgrDirectoryArray[i])))
			return(ret);
	}
	return(ret);
}

/*-----------------------------------------------------------------------*/

#endif /*__FILE_MANAGER_ENABLED__*/
