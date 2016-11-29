/*  
 *  rc_ansifs.c
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
/*

$History: rc_ansifs.c $
 * 
 * *****************  Version 22  *****************
 * User: Pstuart      Date: 3/02/01    Time: 1:26p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Append directory separator for working directories missing that at end
 * 
 * *****************  Version 21  *****************
 * User: Pstuart      Date: 1/11/01    Time: 3:05p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * fix gcc warning
 * 
 * *****************  Version 20  *****************
 * User: James        Date: 1/03/01    Time: 5:24p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Removed unused variable. JAB
 * 
 * *****************  Version 19  *****************
 * User: Pstuart      Date: 11/16/00   Time: 5:14p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * new support for ls w/ file size, and directory info
 * 
 * *****************  Version 18  *****************
 * User: Schew        Date: 11/10/00   Time: 1:59p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * take out 'a'- cause compiler error
 * 
 * *****************  Version 17  *****************
 * User: Pstuart      Date: 10/31/00   Time: 2:51p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Consolidate file read routines, add support for separate working
 * directories per session
 * 
 * *****************  Version 16  *****************
 * User: Pstuart      Date: 9/26/00    Time: 11:56a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Change: Empty files return NULL w/ 0 length,  bad files return NULL w/
 * -1 length
 * 
 * *****************  Version 15  *****************
 * User: Pstuart      Date: 9/16/00    Time: 8:52p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * 
 * *****************  Version 14  *****************
 * User: Pstuart      Date: 9/16/00    Time: 7:30p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Added support for file system manipulation (cd, ls, pwd).
 * 
 * *****************  Version 13  *****************
 * User: Schew        Date: 8/02/00    Time: 3:42p
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * RC_FREE to OS_SPECIFIC_FREE
 * 
 * *****************  Version 11  *****************
 * User: Leech        Date: 6/21/00    Time: 11:49a
 * Updated in $/Rapid Logic/Code Line/rli_code/rli_os
 * Cleanup of the history logging comments


 */

#include "rc_options.h"

#ifdef __ANSI_FILE_MANAGER_ENABLED__

#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_environ.h"
#include "rc_dc_defs.h"
#include "rc_inflate.h"
#include "rc_decomp.h"
#include "rc_ansifs.h"
#include <stdio.h>

static sbyte                *mDefaultPath = NULL;
static OS_SPECIFIC_MUTEX    mFileMutex;

#ifndef RL_STATIC
#define RL_STATIC static
#endif

/*-----------------------------------------------------------------------*/

#ifdef __DECOMPRESSION_ENABLED__

RL_STATIC void
ANSIFS_Lock(void)
{
   OS_SPECIFIC_MUTEX_WAIT( mFileMutex );
}

/*-----------------------------------------------------------------------*/

RL_STATIC void
ANSIFS_Unlock(void)
{
   OS_SPECIFIC_MUTEX_RELEASE( mFileMutex );
}

#endif /* __DECOMPRESSION_ENABLED__ */

/*-----------------------------------------------------------------------*/

RL_STATIC void
ANSIFS_DetermineFileLength( FILE *pFile, Length *pLength)
{
    sbyte4  start, end;

    /* Is there an easier way of doing this? */
    fseek(pFile, 0, SEEK_SET);
    start = ftell(pFile);

    fseek(pFile, 0, SEEK_END);
    end   = ftell(pFile);

    fseek(pFile, 0, SEEK_SET);

    *pLength = (Length)end - (Length)start;
}

/*-----------------------------------------------------------------------*/

RL_STATIC void*
ANSIFS_OpenFile(sbyte *pFileName, sbyte *pPathName, 
                int *pLength, Boolean compression)
{
    sbyte   *pFullPath   = NULL;
    sbyte   *pData       = NULL;
    sbyte   *pReturn     = NULL; 
    FILE    *pFile       = NULL;
    sbyte4  pathLen      = 0;
    sbyte4  fullPathLen  = 0;
    sbyte4  bytesRead    = 0;
    Length  fileLen      = 0;
#ifdef __DECOMPRESSION_ENABLED__
    Boolean isCompressed = FALSE;
    sbyte   *pDecompressedData;
    sbyte4   decompressedLen;
#endif /* __DECOMPRESSION_ENABLED__ */

    *pLength = -1; /* to differentiate a null for empty file */

    pathLen     = STRLEN(pPathName);
    fullPathLen = pathLen + STRLEN(pFileName) + 3;
    pFullPath   = (sbyte*) OS_SPECIFIC_MALLOC(fullPathLen);
    if (NULL == pFullPath)
    {
        DEBUG_MSG_2("error malloc -- %d bytes, for %s\n", fullPathLen, pFileName);
        return NULL;
    }

    STRCPY(pFullPath, pPathName);

    /* does the base path have proper path separator? */
    if (0 < pathLen)
    {
        --pathLen;

#ifdef __WIN32_OS__
        if ('\\' != pPathName[pathLen])
            STRCAT(pFullPath, "\\");
#else
        if ('/' != pPathName[pathLen])
            STRCAT(pFullPath, "/");
#endif
    }

    STRCAT(pFullPath, pFileName);

    pFile = fopen(pFullPath, "rb");
    if (NULL == pFile)
    {
        DEBUG_MSG_1("error fopen -- %s", pFullPath);
        OS_SPECIFIC_FREE(pFullPath);
        return NULL;
    }

    ANSIFS_DetermineFileLength( pFile, &fileLen );
    if (0 >= fileLen)
    {
        *pLength = 0;
        OS_SPECIFIC_FREE(pFullPath);
        return NULL;
    }

    pData = (sbyte*)OS_SPECIFIC_MALLOC(fileLen);
    if (NULL == pData)
    {
        DEBUG_MSG_1("error malloc -- %d bytes, for buffer\n", fileLen);
        OS_SPECIFIC_FREE(pFullPath);
        return NULL;
    }

    bytesRead = fread(pData, sizeof(sbyte), fileLen, pFile);
    *pLength = fileLen;
    pReturn  = pData;

#ifdef __DECOMPRESSION_ENABLED__

    if (compression)
    {
        /* If the file is compressed, then use the decompression code.
           Otherwise, just return the data as is. */
        DECOMP_IsCompressed(pData, &isCompressed);
        if ( isCompressed )
        {
            ANSIFS_Lock();
            DECOMP_DecompressData(pData, bytesRead, 
                                  &pDecompressedData, &decompressedLen);
            ANSIFS_Unlock();
            OS_SPECIFIC_FREE(pData);

            *pLength = decompressedLen;
            pReturn  = pDecompressedData;
        }
    }

#endif /* __DECOMPRESSION_ENABLED__ */

    OS_SPECIFIC_FREE(pFullPath);
    fclose(pFile);
 
    return pReturn;
}

/*-----------------------------------------------------------------------*/
extern void*    
ANSIFS_ReadFile(environment *pEnv, int *pLen, sbyte *pId)
{
#ifdef __ENABLE_SESSION_DIRECTORIES__
    sbyte   *pBasePath    = pEnv->cwd;
#else
    sbyte   *pBasePath    = mDefaultPath;
#endif

    return ANSIFS_OpenFile(pId, pBasePath, pLen, TRUE);
}

/*-----------------------------------------------------------------------*/

extern void*    
ANSIFS_RetrieveFile(int *pLen, sbyte *pId)
{
    return ANSIFS_OpenFile(pId, mDefaultPath, pLen, TRUE);
}


/*-----------------------------------------------------------------------*/

extern void
ANSIFS_ReleaseFile(void *pObject)
{
    if (NULL != pObject)
        OS_SPECIFIC_FREE(pObject);
}

/*-----------------------------------------------------------------------*/

extern void*    
ANSIFS_RetrieveResource(int *pLen, sbyte *pId)
{
    return ANSIFS_OpenFile(pId, mDefaultPath, pLen, FALSE);
}

/*-----------------------------------------------------------------------*/

extern void
ANSIFS_ReleaseResource(void *pObject)
{
    OS_SPECIFIC_FREE(pObject);
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS
ANSIFS_Init(sbyte *pPath)
{
    RLSTATUS status = OK;

    /* already initialized? */
    if (NULL != mDefaultPath)
        return ERROR_GENERAL;

    mDefaultPath = OS_SPECIFIC_MALLOC( STRLEN(pPath) + 1 );
    if ( NULL == mDefaultPath )
        return ERROR_MEMMGR_NO_MEMORY;

    STRCPY( mDefaultPath, pPath );

    status = OS_SPECIFIC_MUTEX_CREATE( &mFileMutex );
    return status;
}

#ifdef __ENABLE_SESSION_DIRECTORIES__

/*-----------------------------------------------------------------------*/

RL_STATIC Boolean ANSIFS_ChangePath(sbyte *pOldPath, sbyte *pNewPath)
{
    sbyte       *pParent;

    /* relative change */
    while (('.' == *pNewPath) && ('.' == *(pNewPath + 1)))
    {
        /* move up old path */
        pParent  = STRRCHR(pOldPath, kFileSeparator);

        pNewPath = STRCHR(pNewPath, kFileSeparator);
/*
        if (NULL == pNewPath)
            return FALSE;
*/
        if (NULL == pParent)
            break;

        *(++pParent) = 0;

        if (NULL == pNewPath)
            break;
    }
    return TRUE;
}

/*-----------------------------------------------------------------------*/

extern RLSTATUS
ANSIFS_Chdir(environment *pEnv, sbyte *pPath)
{
    RLSTATUS     status  = OK;
    sbyte        tempPath[kDIRECTORY_BUFFER_SIZE];
    sbyte4       length;

#ifdef __WIN32_OS__
    DWORD        attributes;
#else
    DIR         *handle;
#endif

    if (NULL_STRING(pPath))
        return ERROR_GENERAL_INVALID_PATH;

    if (kDIRECTORY_BUFFER_SIZE < STRLEN(pPath))
        return ERROR_GENERAL_BUFFER_OVERRUN;

    STRNCPY(tempPath, pEnv->cwd, kDIRECTORY_BUFFER_SIZE);

    switch (*pPath)
    {
    case '.':
        /* relative path */
        ANSIFS_ChangePath(tempPath, pPath);
        break;
    case kFileSeparator:
        /* absolute path */
        STRNCPY(tempPath, pPath, kDIRECTORY_BUFFER_SIZE);
        break;
    default:
#ifdef __WIN32_OS__
        /* grrrr... stupid DOS */
        if (':' == *(pPath + 1))
            STRNCPY(tempPath, pPath, kDIRECTORY_BUFFER_SIZE);
        else
#endif
        {
            length = STRLEN(tempPath);
            if (kFileSeparator != tempPath[length - 1])
            {
                tempPath[length++] = kFileSeparator;
                tempPath[length  ] = 0;
            }
            STRNCAT(tempPath, pPath, kDIRECTORY_BUFFER_SIZE - length);
        }
        break;
    }

    /* make sure path is valid and have access */
#ifdef __WIN32_OS__
    if ( -1 == (attributes = GetFileAttributes(tempPath)))
        return ERROR_GENERAL_INVALID_PATH;

    if (  0 == (FILE_ATTRIBUTE_DIRECTORY & attributes))
        return ERROR_GENERAL_INVALID_PATH;
#else
    if (NULL == (handle = opendir(tempPath)))
        return ERROR_GENERAL_INVALID_PATH;

    closedir(handle);
#endif

    STRNCPY(pEnv->cwd, tempPath, kDIRECTORY_BUFFER_SIZE);

    return status;
}

/*-----------------------------------------------------------------------*/

#ifdef __WIN32_OS__
extern RLSTATUS ANSIFS_OpenDir(environment *pEnv, sbyte *pFileName)
{
    sbyte       *pFullPath;
    sbyte4       length;
    DIR_HANDLE  *pDir   = pEnv->pDirHandle;
    RLSTATUS     status = OK;

    if (NULL == pDir)
    {
        pDir = (DIR_HANDLE *) RC_MALLOC(sizeof(DIR_HANDLE));

        if (NULL == pDir)
            return ERROR_MEMMGR_NO_MEMORY;
    }

    if (NULL == pFileName)
        pFileName = "*";

    if (NULL == (pFullPath = RC_MALLOC(kDIRECTORY_BUFFER_SIZE)))
        return ERROR_MEMMGR_NO_MEMORY;

    length = STRLEN(pEnv->cwd);
    STRCPY(pFullPath, pEnv->cwd);
    if (kFileSeparator != pFullPath[length - 1])
    {
        pFullPath[length++] = kFileSeparator;
        pFullPath[length]     = 0;
    }

    STRNCAT(pFullPath, pFileName, kDIRECTORY_BUFFER_SIZE - length);

    /* Find first file in current directory */
    if (-1L == (pDir->handle = _findfirst(pFullPath, &pDir->fileInfo)))
        status = ERROR_GENERAL_FILE_NOT_FOUND;

    pDir->found      = FALSE;
    pEnv->pDirHandle = pDir;

    RC_FREE(pFullPath);

    return status;
}

#else /* ! __WIN32_OS__ */

extern RLSTATUS 
ANSIFS_OpenDir(environment *pEnv, sbyte *pFileName)
{
    sbyte4       length;
    sbyte       *pFullPath = NULL;
    sbyte       *pTruncate = NULL;
    DIR_HANDLE  *pDir      = pEnv->pDirHandle;

    DEBUG_MSG_1("Open directory for filename: %s\n", pFileName);

    if (NULL == (pDir = (DIR_HANDLE *) RC_MALLOC(sizeof(DIR_HANDLE))))
        return ERROR_MEMMGR_NO_MEMORY;

    if (NULL == (pFullPath = RC_MALLOC(kDIRECTORY_BUFFER_SIZE)))
    {
        RC_FREE(pDir);
        return ERROR_MEMMGR_NO_MEMORY;
    }

    length = STRLEN(pEnv->cwd);
    STRCPY(pFullPath, pEnv->cwd);
    if (kFileSeparator != pFullPath[length - 1])
    {
        pFullPath[length++] = kFileSeparator;
        pFullPath[length]     = 0;
    }

    DEBUG_MSG_1("Current working directory: %s\n", pFullPath);
    
    if (! NULL_STRING(pFileName))
    {
        DEBUG_MSG_1("Appending filename: %s\n", pFileName);
        STRNCAT(pFullPath, pFileName, kDIRECTORY_BUFFER_SIZE - length);
        pTruncate = STRRCHR(pFullPath, kFileSeparator);
        if (NULL == pTruncate)
            pTruncate = STRRCHR(pFullPath, '.');
    }

    DEBUG_MSG_0("Appended filename\n");

    if (NULL != pTruncate)
    {
        pTruncate++;
        DEBUG_MSG_1("Filemask: %s\n", pTruncate);
        STRNCPY(pDir->fileMask, pTruncate, sizeof(pDir->fileMask));
        *pTruncate = 0;
	STRNCPY(pDir->fileDir, pFileName, sizeof(pDir->fileDir));
    }
    else
    {
        DEBUG_MSG_0("No filename given\n");   
        if (NULL_STRING(pFileName))
	    pDir->fileMask[0] = 0;
	else
            STRNCPY(pDir->fileMask, pFileName, sizeof(pDir->fileMask));
    }    
    pDir->nameLen = STRLEN(pDir->fileMask);
      
    DEBUG_MSG_1("Adjusted directory: %s\n", pFullPath);
    DEBUG_MSG_1("Filemask: %s\n", pDir->fileMask);

    if (NULL == (pDir->handle = opendir (pFullPath)))
    {
        RC_FREE(pDir);
	    RC_FREE(pFullPath);
        return ERROR_GENERAL_FILE_NOT_FOUND;
    }

    RC_FREE(pFullPath);
    
    pEnv->pDirHandle = pDir;

    return OK;
}
#endif /* ! __WIN32_OS__ */

/*-----------------------------------------------------------------------*/

#ifdef __WIN32_OS__
extern sbyte* ANSIFS_NextDir(environment *pEnv, sbyte4 *size, ubyte4 *flags)
{
    DIR_HANDLE  *pDir;

    *size  = 0;
    *flags = 0;

    if ((NULL == pEnv) || (NULL == pEnv->pDirHandle))
        return NULL;

    pDir = pEnv->pDirHandle;

    /* First file in current directory? */
    if (FALSE == pDir->found)
        pDir->found = TRUE;
    else
    {
        if (0 > _findnext(pDir->handle, &pDir->fileInfo))
            return NULL;
    }

    if (pDir->fileInfo.attrib & _A_SUBDIR)
        *flags |= kANSIFS_DIRECTORY;

    if (pDir->fileInfo.attrib & _A_RDONLY)
        *flags |= kANSIFS_READ_ONLY;

    *size = pDir->fileInfo.size;
    return pDir->fileInfo.name;
}

#else /* ! __WIN32_OS__ */

extern sbyte* ANSIFS_NextDir(environment *pEnv, sbyte4 *size, ubyte4 *flags)
{
    sbyte           fileSeparator = kFileSeparator;
    sbyte	   *pFullName;
    struct  dirent *pDirInfo;
    DIR_HANDLE     *pDir;
    sbyte          *pName;
    struct stat     fileStats;

    *size  = 0;
    *flags = 0;

    if ((NULL == pEnv) || (NULL == pEnv->pDirHandle))
        return NULL;

    if (NULL == (pFullName = RC_MALLOC(kDIRECTORY_BUFFER_SIZE)))
        return NULL;

    *pFullName = 0;
    pDir = pEnv->pDirHandle;

    if (NULL == (pDirInfo = readdir(pDir->handle)))
    {
        DEBUG_MSG_0("Readdir failed\n");
	RC_FREE(pFullName);
        return NULL;
    }
	
    pName = pDirInfo->d_name;
    if (0 == STRCMP(pName, ".") || (0 == STRCMP(pName, "..")))
        return pName;
/* 
    if ( (!NULL_STRING(pName)) && (0 != STRNCMP(pName, pDir->fileMask, pDir->nameLen)))
       return NULL;
*/
    if (!NULL_STRING(pEnv->cwd))
       STRCPY(pFullName, pEnv->cwd);
    
    if (!NULL_STRING(pDir->fileDir))
       STRCAT(pFullName, pDir->fileDir);
    
    if (!NULL_STRING(pFullName))
        STRNCAT(pFullName, &fileSeparator, 1);
	
    if (!NULL_STRING(pDirInfo->d_name))
       STRCAT(pFullName, pDirInfo->d_name);

    DEBUG_MSG_1("Stat for: %s\n", pFullName);
       
    /* expand error handling -- get err info */
    if (0 != stat(pFullName, &fileStats))
    {
        DEBUG_MSG_0("Stat failed\n");
	RC_FREE(pFullName);
        return NULL;
    }
    
    *size = fileStats.st_size;
    DEBUG_MSG_1("File size: %d\n", *size);
    
    if (S_ISDIR(fileStats.st_mode))
        *flags |= kANSIFS_DIRECTORY;
	
    RC_FREE(pFullName);
    return pDirInfo->d_name;

}

#endif /* __WIN32_OS__ */

/*-----------------------------------------------------------------------*/

extern void ANSIFS_CloseDir(environment *pEnv)
{
    DIR_HANDLE  *pDir  = pEnv->pDirHandle;

    CLOSEDIR(pDir->handle);
    RC_FREE(pDir);
    pEnv->pDirHandle = NULL;
}

#else  /* __ENABLE_SESSION_DIRECTORIES__ */

extern RLSTATUS ANSIFS_Chdir(environment *pEnv, sbyte *pPath) {return OK;}
extern void     ANSIFS_CloseDir(environment *pEnv){}
extern RLSTATUS ANSIFS_OpenDir(environment *pEnv, sbyte *pFileName) {return OK;}
extern sbyte*   ANSIFS_NextDir(environment *pEnv, sbyte4 *size, ubyte4 *flags)
{
    *size  = 0;
    *flags = 0;
    return NULL;
}


#endif /* __ENABLE_SESSION_DIRECTORIES__ */

#endif /* __ANSI_FILE_MANAGER_ENABLED__ */

