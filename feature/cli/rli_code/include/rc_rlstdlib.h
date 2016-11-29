/*
 *  rc_rlstdlib.h
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
$History: rc_rlstdlib.h $
 * 
 * *****************  Version 27  *****************
 * User: Leech        Date: 4/16/01    Time: 6:37p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * fixed _cplusplus curly brace misplacement
 * 
 * *****************  Version 26  *****************
 * User: Pstuart      Date: 12/11/00   Time: 11:30a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * added c++ wrappers for prototypes
 * 
 * *****************  Version 25  *****************
 * User: Schew        Date: 10/24/00   Time: 5:43p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * __ENABLE_DANGLING_PTR_TESTING__
 * 
 * *****************  Version 24  *****************
 * User: Pstuart      Date: 8/04/00    Time: 10:17a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * disable mem debug w/ native malloc
 * 
 * *****************  Version 23  *****************
 * User: Pstuart      Date: 7/25/00    Time: 3:21p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Rename MIN/MAX to RC_MIN/RC_MAX
 * 
 * *****************  Version 22  *****************
 * User: Pstuart      Date: 6/07/00    Time: 5:50p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * 
 * *****************  Version 21  *****************
 * User: Pstuart      Date: 5/26/00    Time: 3:13p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added ansi equivalent for STRNCAT
 * 
 * *****************  Version 20  *****************
 * User: Pstuart      Date: 5/23/00    Time: 10:19a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added RC_Replace
 * 
 * *****************  Version 19  *****************
 * User: Pstuart      Date: 5/18/00    Time: 4:12p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added ISALPHA()
 * 
 * *****************  Version 17  *****************
 * User: Epeterson    Date: 4/25/00    Time: 2:13p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Include history and enable auto archiving feature from VSS



$/Rapid Logic/Code Line/rli_code/include/rc_rlstdlib.h

16   Pstuart     4/19/00  12:40p   Checked in $/Rapid Logic/Code Line/rli_code/include    
15   Schew       4/14/00   3:10p   Checked in $/Rapid Logic/Code Line/rli_code/include    
14   Schew       4/14/00  11:16a   Checked in $/Rapid Logic/Code Line/rli_code/include    
13   Schew       4/13/00   6:25p   Checked in $/Rapid Logic/Code Line/rli_code/include    
12   Pstuart     4/05/00   3:23p   Checked in $/Rapid Logic/Code Line/rli_code/include    
11   Schew       3/22/00   7:32p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Pstuart     3/22/00  10:53a   Labeled 'PreCodeRename'                                
     David       3/07/00   8:06p   Labeled 'RC 3.01 Final Release'                        
     Builder     3/02/00   5:18p   Labeled 'RC 301 Build20000302'                         
     Builder     2/29/00   6:48p   Labeled 'Build301 20000229'                            
     Builder     2/28/00   6:35p   Labeled 'RC301 Build20000228'                          
     Builder     2/24/00   4:01p   Labeled 'RC301 Build20000224'                          
     David      12/20/99  10:55a   Labeled 'RC 3.0 Final Release'                         
     Builder    12/15/99   6:36p   Labeled 'RC30 Build19991215'                           
     Builder    12/14/99   7:08p   Labeled 'RC30 Build19991214'                           
10   Pstuart    12/14/99   6:02p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Builder    12/10/99   5:55p   Labeled 'RC30 Build19991210'                           
     Builder    11/17/99   4:32p   Labeled 'RC30 Build19991117 Release'                   
     Builder    11/16/99   5:37p   Labeled 'RC30 Build19991116'                           
     Builder    11/15/99   5:59p   Labeled 'RC30 Build19991115'                           
     Builder    11/12/99   5:27p   Labeled 'RC30 Build19991112'                           
     Builder    11/11/99   7:00p   Labeled 'RC30 Build19991111'                           
     Builder    11/10/99   7:12p   Labeled 'RC30 Build19991110'                           
     Builder    11/08/99   6:09p   Labeled 'RC30 Build19991108'                           
     Builder    11/05/99   6:33p   Labeled 'RC30 Build19991105'                           
     Builder    11/04/99   5:04p   Labeled 'RC30 Build19991104'                           
     Builder    11/03/99   6:23p   Labeled 'RC3.0 Build 19991103'                         
     Builder    11/01/99   5:51p   Labeled 'RC 3.0 Build 19991101'                        
     Builder    10/29/99   5:30p   Labeled 'RC30 Build 19991029'                          
     David       9/28/99   5:42p   Labeled 'RC 3.0 beta 2 - 9-28-99'                      
9    Paul        9/17/99   3:34p   Checked in $/Rapid Logic/Code Line/rli_code/include    
8    Paul        9/17/99   2:20p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Builder     8/10/99   2:41p   Labeled 'RC 3.0 alpha 4 limited release'               
     David       8/04/99   1:24p   Labeled 'RC 2.4 release'                               
     Builder     7/29/99   5:56p   Labeled 'RCA/RCW/Mibway 2.4 Final'                     
     David       7/20/99   2:04p   Labeled 'RC 2.4 beta 6 training '                      
7    Kedron      5/27/99   8:54a   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Leech       4/29/99  11:10a   Labeled 'WC/JC 2.31 Release - 4-29-99'                 
     Leech       4/27/99   2:10p   Labeled 'WC/JC 2.31 beta 7- 4-27-99'                   
     Leech       4/26/99   2:42p   Labeled 'WC/JC 2.31 beta6 4-26-99'                     
     David       4/14/99   3:53p   Labeled 'WC/JC 2.31 beta4 '                            
     David       4/07/99   6:06p   Labeled 'WC/JC2.31 beta2 - 4-7-99'                     
     David       4/06/99   3:38p   Labeled 'WC2.31 beta 1 - 4-6-99'                       
     Mredison    2/19/99   6:42p   Labeled 'WebControl 2.3 Final Gold'                    
     David       2/17/99  10:14a   Labeled 'JC2.3 Final Release - 2-17-99'                
     David       2/16/99   6:19p   Labeled 'WC 2.3 Final Release - 2-16-99'               
     David       2/15/99   5:56p   Labeled 'WC2.3 beta11 - 2-15-99'                       
     David       2/12/99   6:06p   Labeled 'JC 2.3 beta9 - 2-12-99'                       
     David       2/12/99   6:06p   Labeled 'WC 2.3 beta 9 - 2-12-99'                      
     David       2/11/99   4:51p   Labeled 'JC 2.3 beta8 - 2-11-99'                       
     David       2/11/99   4:51p   Labeled 'WC 2.3 beta 8 - 2-11-99'                      
     David       2/09/99  11:30a   Labeled 'WC2.3 beta5 - 2-9-99'                         
     David       2/08/99   5:28p   Labeled 'JC2.3 beta5 - 2-8-99'                         
     David       2/05/99   6:55p   Labeled 'WC2.3 beta4 - 2-5-99'                         
     David       2/05/99   6:55p   Labeled 'JC2.3 beta4 - 2-5-99'                         
     David       2/03/99   7:23p   Labeled 'WC 2.3 beta3 - 2-3-99'                        
     David       2/03/99   7:23p   Labeled 'JC2.3 beta3 - 2-3-99'                         
     David       2/01/99   7:00p   Labeled 'JC2.3 beta 2 - 2/1/99'                        
     David       1/29/99   6:13p   Labeled 'JC2.3beta1'                                   
     David       1/15/99   3:16p   Labeled 'jc 2.2 rc4'                                   
     Henry       1/08/99   4:20p   Labeled 'jc 2.2 rc3'                                   
     Henry      12/18/98   5:12p   Labeled 'jc 2.2 rc2a'                                  
     Henry      12/18/98   4:34p   Labeled 'jc 2.2 rc2'                                   
     David      12/10/98   6:29p   Labeled 'JC 12/10'                                     
     Henry      12/07/98   5:27p   Labeled 'JC 12/7'                                      
     Henry      12/04/98   3:35p   Labeled 'Build 12/4'                                   
6    Jab        12/01/98   3:21p   Checked in $/Rapid Logic/Code Line/rli_code/include    
5    Jamesb     10/30/98   3:44p   Checked in $/Rapid Logic/Code Line/rli_code/include    
4    David      10/14/98   5:27p   Checked in $/Rapid Logic/Code Line/rli_code/include    
3    Mredison    9/17/98  11:44a   Checked in $/Rapid Logic/Code Line/rli_code/include    
2    Mredison    9/16/98   5:11p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Leech       8/07/98   4:28p   Labeled '2.01'                                         
1    Leech       7/27/98   6:25p   Created rlstdlib.h                                     





  */




#ifndef __RLSTDLIB_HEADER__
#define __RLSTDLIB_HEADER__

/* prototypes */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __OS_MALLOC_PROVIDED__
#undef __ENABLE_MEMMGR_DEBUG__
#endif 

#if ((!defined(__ENABLE_MEMMGR_DEBUG__)) && (!defined(__ENABLE_DANGLING_PTR_TESTING__)))

extern void     *RC_MALLOC ( Length memSize                             );
extern void     *RC_CALLOC ( ubyte4 num, Length size                       );
extern void     RC_FREE ( void *pBuffer                                 );

#else

#ifdef __ENABLE_DANGLING_PTR_TESTING__
extern void     RC_FREE ( void *pBuffer                                 );
extern void     *RC_MALLOC ( Length memSize                             );

#define RC_MALLOC(x)	RLI_DEBUG_MALLOC(x,__FILE__,__LINE__)
extern void     *RLI_DEBUG_MALLOC ( Length memSize, sbyte *pFile, int lineNum          );

#else

#define RC_FREE(x)		RLI_DEBUG_FREE(x,__FILE__,__LINE__)
#define RC_MALLOC(x)	RLI_DEBUG_MALLOC(x,__FILE__,__LINE__)

extern void     RLI_DEBUG_FREE    ( void *pBuffer, sbyte *pFile, int lineNum           );
extern void     *RLI_DEBUG_MALLOC ( Length memSize, sbyte *pFile, int lineNum          );
#endif

#define RC_CALLOC(x,y)  RLI_DEBUG_CALLOC(x,y,__FILE__,__LINE__)

extern void     *RLI_DEBUG_CALLOC ( ubyte4 num, Length size, sbyte *pFile, int lineNum );

#endif

/* String functions. */
extern sbyte    *STRTOK_REENTRANT
                        ( sbyte *pStr, sbyte *pDelimiter, sbyte **ppPrevStr);
extern sbyte4   STRICMP ( sbyte *pStr1, sbyte *pStr2                    );
extern sbyte4   STRNICMP( sbyte *pStr1, sbyte *pStr2, sbyte4 Len        );
extern sbyte*   STRICHR ( const sbyte *pStr, sbyte4 c                   );

/* String Conversion routines */
extern Boolean  ISLOWER ( sbyte4 c                                      );
extern Boolean  ISDIGIT ( sbyte4 c                                      );
extern Boolean  ISSPACE ( sbyte4 c                                      );
extern Boolean  ISWSPACE( sbyte4 c                                      );
extern sbyte4   ATOI    ( const sbyte *pStr                             );
extern void     UPCASE  ( sbyte *pStr                                   );
extern Boolean  ISALPHA ( sbyte4 x                                      );
extern Boolean  ISALPHANUMERIC(sbyte4 c                                 );
extern RLSTATUS RC_Replace(sbyte *pInput, sbyte *pOutput, sbyte4 outLen,
                           sbyte *pFind, sbyte *pReplace, Boolean word  );

#define TOUPPER(x)  ( (('a' <= x) && ('z' >= x)) ? (x - 'a' + 'A') : x )
#define TOLOWER(x)  ( (('A' <= x) && ('Z' >= x)) ? (x - 'A' + 'a') : x )

/* Time routines */
extern void     ASCTIME ( ubyte4 TimeInSec, sbyte *pTimeBuffer          );

/* General Tests */
#define RC_MIN(x, y)   ( (x < y) ? (x) : (y) )
#define RC_MAX(x, y)   ( (x > y) ? (x) : (y) )


#ifndef __USE_LOCAL_ANSI_LIB__


/* Memory Routines. */
#ifndef MEMSET
extern void*    MEMSET  ( void *pDest, sbyte value, Length size         );
#endif

extern void*    MEMCPY  ( void *pDest, const void *pSrc, Length size    );
extern sbyte4   MEMCMP  ( const void *pBuf1, const void *pBuf2,
                          Length Len );

/* String functions. */
extern sbyte4   STRCMP  ( sbyte *pStr1, sbyte *pStr2                    );
extern sbyte4   STRNCMP ( sbyte *pStr1, sbyte *pStr2, sbyte4 Len        );

#ifndef STRLEN
extern sbyte4   STRLEN  ( sbyte *pStr                                   );
#endif

extern sbyte*   STRCPY  ( sbyte *pDest, const sbyte *pSrc               );
extern sbyte*   STRNCPY ( sbyte *pDest, const sbyte *pSrc, sbyte4 num   );
extern sbyte*   STRCAT  ( sbyte *pDest, const sbyte *pSrc               );
extern sbyte*   STRNCAT ( sbyte *pDest, const sbyte *pSrc, sbyte4 bufLen);
extern sbyte*   STRCHR  ( const sbyte *pStr, sbyte4 c                   );
extern sbyte*   STRRCHR ( const sbyte *pStr, sbyte4 c                   );
extern sbyte4   STRSPN  ( sbyte *pStr, sbyte *pSampleChars              );
extern sbyte4   STRCSPN ( sbyte *pStr, sbyte *pReject                   );
extern sbyte*   STRPBRK ( sbyte *pStr, sbyte *pSampleStr                );

#else   /* __USE_LOCAL_ANSI_LIB__ */


#include <stdlib.h>
#include <string.h>

/* Memory Routines. */
#ifndef MEMSET
#define MEMSET      memset
#endif

#define MEMCPY      memcpy
#define MEMCMP      memcmp

/* String functions. */
#define STRCMP      strcmp
#define STRNCMP     strncmp

#ifndef STRLEN
#define STRLEN      strlen
#endif

#define STRCPY      strcpy
#define STRNCPY     strncpy
#define STRCAT      strcat
#define STRNCAT     strncat
#define STRCHR      strchr
#define STRRCHR     strrchr
#define STRSPN      strspn
#define STRCSPN     strcspn
#define STRPBRK     strpbrk

/* String Conversion routines */
#define ATOI        atoi

#endif /* __USE_LOCAL_ANSI_LIB__ */

#ifdef __cplusplus
}
#endif

#endif /* __RLSTDLIB_HEADER__ */
