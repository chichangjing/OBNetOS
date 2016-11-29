/*  
 *  rc_ansifs.h
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

#ifdef __ANSI_FILE_MANAGER_ENABLED__

#ifndef __ANSIFS_HEADER__
#define __ANSIFS_HEADER__

#ifdef __WIN32_OS__

#include <direct.h>
#include <io.h>
#define CLOSEDIR    _findclose    

#else /* __WIN32_OS__ */

#ifdef __POSIX_OS__
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif /* __POSIX_OS__ */

#ifdef __VXWORKS_OS__
#include <dirent.h>
#include <stat.h>
#endif /* __VXWORKS_OS__ */

#define CLOSEDIR    closedir

#endif /* ! __WIN32_OS__ */


#ifdef __WIN32_OS__
#define kFileSeparator  '\\'
#else
#define kFileSeparator  '/'
#endif


#ifdef __WIN32_OS__
typedef struct DIR_HANDLE 
{
    Boolean found;
    long    handle;
    struct  _finddata_t fileInfo;
} DIR_HANDLE;
#else
typedef struct DIR_HANDLE
{
    DIR     *handle;
    sbyte4   nameLen;
    sbyte    fileMask[64];
    sbyte    fileDir[64];
} DIR_HANDLE;
#endif /* __WIN32_OS__ */

/* file attribute flags -- expand */
#define kANSIFS_DIRECTORY   0x0001
#define kANSIFS_READ_ONLY   0x0002


#ifdef __cplusplus
extern "C" {
#endif

extern RLSTATUS ANSIFS_Init             (sbyte  *pPath          );
extern void*    ANSIFS_ReadFile         (environment *pEnv, int *pLen, sbyte *pId);
extern void*    ANSIFS_RetrieveFile     (int    *pLen, sbyte *Id);
extern void     ANSIFS_ReleaseFile      (void   *pObject        );
extern void*    ANSIFS_RetrieveResource (int    *pLen, sbyte *Id);
extern void     ANSIFS_ReleaseResource  (void   *pObject        );

extern RLSTATUS ANSIFS_Chdir    (environment *pEnv, sbyte *pPath);
extern RLSTATUS ANSIFS_OpenDir  (environment *pEnv, sbyte *pFile);
extern sbyte*   ANSIFS_NextDir  (environment *pEnv, sbyte4 *size, ubyte4 *flags);
extern void     ANSIFS_CloseDir (environment *pEnv              );

#ifdef __cplusplus
}
#endif


#endif /* ! __ANSIFS_HEADER__               */
#endif /*   __ANSI_FILE_MANAGER_ENABLED__   */

