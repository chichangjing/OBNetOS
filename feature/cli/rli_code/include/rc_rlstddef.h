/*  
 *  rc_rlstddef.h
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
$History: rc_rlstddef.h $
 * 
 * *****************  Version 38  *****************
 * User: Pstuart      Date: 8/10/01    Time: 1:42p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * clean up debug_msg macros
 * 
 * *****************  Version 37  *****************
 * User: Pstuart      Date: 2/26/01    Time: 10:29a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added memory allocation macro, envoy stack macro
 * 
 * *****************  Version 36  *****************
 * User: Pstuart      Date: 12/01/00   Time: 12:22p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * added rapidmark detection macros
 * 
 * *****************  Version 35  *****************
 * User: Pstuart      Date: 11/17/00   Time: 7:01p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * better debugging macros, moved datatype structs to rc_convert.h
 * 
 * *****************  Version 34  *****************
 * User: Pstuart      Date: 11/10/00   Time: 2:39p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * added HAS_STRING macro
 * 
 * *****************  Version 33  *****************
 * User: Pstuart      Date: 10/31/00   Time: 2:51p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * fix gcc error
 * 
 * *****************  Version 32  *****************
 * User: Pstuart      Date: 10/29/00   Time: 12:54p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added debug print macros
 * 
 * *****************  Version 31  *****************
 * User: Pstuart      Date: 10/19/00   Time: 10:08a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * 
 * *****************  Version 30  *****************
 * User: Pstuart      Date: 10/11/00   Time: 10:24a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added  kEOF
 * 
 * *****************  Version 29  *****************
 * User: Pstuart      Date: 6/16/00    Time: 1:22p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * 
 * *****************  Version 28  *****************
 * User: Pstuart      Date: 6/16/00    Time: 9:43a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * 
 * *****************  Version 27  *****************
 * User: Pstuart      Date: 6/15/00    Time: 3:19p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * 
 * *****************  Version 26  *****************
 * User: Pstuart      Date: 6/12/00    Time: 1:57p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * 
 * *****************  Version 25  *****************
 * User: Pstuart      Date: 6/07/00    Time: 5:49p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Casted params for Handlers
 * 
 * *****************  Version 24  *****************
 * User: Pstuart      Date: 5/18/00    Time: 4:00p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * 
 * *****************  Version 23  *****************
 * User: Pstuart      Date: 5/11/00    Time: 2:35p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added typedefs needed for rc_printf
 * 
 * *****************  Version 22  *****************
 * User: Dreyna       Date: 5/09/00    Time: 4:22p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Moved Access typedef to rc_options.h
 * 
 * *****************  Version 21  *****************
 * User: Pstuart      Date: 5/08/00    Time: 4:52p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Made Access sbyte4
 * 
 * *****************  Version 20  *****************
 * User: Pstuart      Date: 5/04/00    Time: 3:22p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added extra void* for DTTypeInfo custom help
 * 
 * *****************  Version 19  *****************
 * User: Dreyna       Date: 5/03/00    Time: 5:53p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * New help handler.
 * 
 * *****************  Version 18  *****************
 * User: Dreyna       Date: 5/01/00    Time: 3:20p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Incremental Custom Type Update: Default value into ParamInfo record,
 * ValidString compression.
 * 
 * *****************  Version 16  *****************
 * User: Dreyna       Date: 4/27/00    Time: 1:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * 
 * *****************  Version 15  *****************
 * User: Epeterson    Date: 4/25/00    Time: 2:13p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Include history and enable auto archiving feature from VSS



$/Rapid Logic/Code Line/rli_code/include/rc_rlstddef.h

14   Dreyna      4/17/00   6:18p   Checked in $/Rapid Logic/Code Line/rli_code/include    
13   Dreyna      4/17/00   5:57p   Checked in $/Rapid Logic/Code Line/rli_code/include    
12   Schew       4/13/00   6:25p   Checked in $/Rapid Logic/Code Line/rli_code/include    
11   Pstuart     4/11/00   3:37p   Checked in $/Rapid Logic/Code Line/rli_code/include    
10   Pstuart     4/11/00   2:53p   Checked in $/Rapid Logic/Code Line/rli_code/include    
9    Schew       3/22/00   7:32p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Pstuart     3/22/00  10:53a   Labeled 'PreCodeRename'                                
     David       3/07/00   8:06p   Labeled 'RC 3.01 Final Release'                        
     Builder     3/02/00   5:18p   Labeled 'RC 301 Build20000302'                         
     Builder     2/29/00   6:48p   Labeled 'Build301 20000229'                            
     Builder     2/28/00   6:35p   Labeled 'RC301 Build20000228'                          
     Builder     2/24/00   4:01p   Labeled 'RC301 Build20000224'                          
8    Pstuart     2/17/00   3:22p   Checked in $/Rapid Logic/Code Line/rli_code/include    
7    Pstuart     2/09/00   2:12p   Checked in $/Rapid Logic/Code Line/rli_code/include    
6    Pstuart     1/05/00   1:47p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     David      12/20/99  10:55a   Labeled 'RC 3.0 Final Release'                         
     Builder    12/15/99   6:36p   Labeled 'RC30 Build19991215'                           
     Builder    12/14/99   7:08p   Labeled 'RC30 Build19991214'                           
     Builder    12/10/99   5:55p   Labeled 'RC30 Build19991210'                           
     Builder    11/17/99   4:32p   Labeled 'RC30 Build19991117 Release'                   
     Builder    11/16/99   5:37p   Labeled 'RC30 Build19991116'                           
     Builder    11/15/99   5:59p   Labeled 'RC30 Build19991115'                           
     Builder    11/12/99   5:27p   Labeled 'RC30 Build19991112'                           
     Builder    11/11/99   7:00p   Labeled 'RC30 Build19991111'                           
     Builder    11/10/99   7:12p   Labeled 'RC30 Build19991110'                           
5    Leech      11/09/99   4:30p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Builder    11/08/99   6:09p   Labeled 'RC30 Build19991108'                           
     Builder    11/05/99   6:33p   Labeled 'RC30 Build19991105'                           
     Builder    11/04/99   5:04p   Labeled 'RC30 Build19991104'                           
     Builder    11/03/99   6:23p   Labeled 'RC3.0 Build 19991103'                         
     Builder    11/01/99   5:51p   Labeled 'RC 3.0 Build 19991101'                        
     Builder    10/29/99   5:30p   Labeled 'RC30 Build 19991029'                          
     David       9/28/99   5:42p   Labeled 'RC 3.0 beta 2 - 9-28-99'                      
4    Paul        9/17/99   2:19p   Checked in $/Rapid Logic/Code Line/rli_code/include    
3    Kedron      9/17/99  12:16p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Builder     8/10/99   2:41p   Labeled 'RC 3.0 alpha 4 limited release'               
     David       8/04/99   1:24p   Labeled 'RC 2.4 release'                               
     Builder     7/29/99   5:56p   Labeled 'RCA/RCW/Mibway 2.4 Final'                     
     David       7/20/99   2:04p   Labeled 'RC 2.4 beta 6 training '                      
2    Kedron      5/27/99   8:54a   Checked in $/Rapid Logic/Code Line/rli_code/include    
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
     Leech       8/07/98   4:28p   Labeled '2.01'                                         
1    Leech       7/27/98   6:25p   Created rlstddef.h                                     





  */

#ifndef __RLSTDDEF_HEADER__
#define __RLSTDDEF_HEADER__

/* Rapid Logic Standard Definitions */
 
/* Standard Types */
typedef sbyte   Path;
typedef ubyte4  Length;
typedef ubyte4  Counter;
typedef int     CompareType;
typedef sbyte   DataType;
typedef ubyte2  UniqId;
typedef ubyte4  EnumType;
typedef sbyte2  EditType;

#ifndef __BOOLEAN_DEFINED__
typedef int     Boolean;
#endif 

typedef int     RLSTATUS;

/* Standard Definitions */
#ifndef TRUE
#define TRUE    1
#endif 

#ifndef FALSE   
#define FALSE   0
#endif

#ifndef OK
#define OK      0
#endif

#ifndef NULL
#define NULL    ((void*)0)
#endif

/* for the CompareType type */

#ifndef EQUAL
#define EQUAL           0
#endif

#ifndef GREATER_THAN
#define GREATER_THAN    1
#endif

#ifndef LESS_THAN
#define LESS_THAN       -1
#endif

#define kMAX_UCHAR      ((unsigned char)((char)(-1)))
#define kMIN_UCHAR      0
#define kMAX_CHAR       (kMAX_UCHAR >> 1)
#define kMIN_CHAR       (~kMAX_CHAR)

#define kMAX_UNSIGNED   ((unsigned int)((int)(-1)))
#define kMIN_UNSIGNED   0
#define kMAX_INT        (kMAX_UNSIGNED >> 1)
#define kMIN_INT        (~kMAX_INT)

#define kMAX_USHORT     ((unsigned short)((short)(-1)))
#define kMIN_USHORT     0
#define kMAX_SHORT      (kMAX_USHORT >> 1)
#define kMIN_SHORT      (~kMAX_SHORT)

#define kMAX_ULONG      ((unsigned long)((long)(-1)))
#define kMIN_ULONG      0
#define kMAX_LONG       (kMAX_ULONG >> 1)
#define kMIN_LONG       (~kMAX_LONG)

#define kErrorChange    -1
#define kNoChange       0
#define kChange         1

#define kMaxTimeString  32

#ifndef kCRLF
#define kCRLF       "\x0D\x0A"
#define kCRLFSize   2
#endif

#ifndef kCR
#define kCR     ((char)(0x0d))
#endif

#ifndef kLF
#define kLF     ((char)(0x0a))
#endif

#ifndef kBS
#define kBS     ((char)(0x08))
#endif

#ifndef kDEL
#define kDEL    ((char)(0x7F))
#endif

#ifndef kTAB
#define kTAB    ((char)(0x09))
#endif

#ifndef kHELP
#define kHELP   ((char)(0x3F))
#endif

#ifndef kESC
#define kESC    ((char)(0x1b))
#endif

#ifndef kCHAR_NULL
#define kCHAR_NULL ((char)(0x00))
#endif

#ifndef kEOF
#define kEOF -1
#endif

/*  MACROS  */

#if !defined (__LITTLE_ENDIAN_SYSTEM__) || defined(__DISABLE_LITTLE_ENDIAN_CONVERSION__)

#define HTON2(x) (x)
#define NTOH2(x) (x)
#define HTON4(x) (x)
#define NTOH4(x) (x)
    
#else

#define HTON2(x) ((((x) << 8) & 0xFF00) | (((x) >> 8) & 0x00FF)) 
#define NTOH2(x) HTON2(x)
#define HTON4(x) (   (( (x) << 24 ) & 0xFF000000) | (( (x) << 8 ) & 0x00FF0000) \
                   | (( (x) >> 8 ) & 0x0000FF00) | (( (x) >> 24 ) & 0x000000FF) )
#define NTOH4(x) HTON4(x)

#endif

/* Free and null pointer to allocated memory */
#define FREEMEM(x) {if (NULL != x) {RC_FREE(x); x = NULL;}}
#define MEMORY_ALLOC_M(x) ((x *) RC_CALLOC(sizeof(x), 1))

/* misc handy tools */
#define NULL_STRING(x)          ((NULL == x) || ('\0' == *x))
#define HAS_STRING(x)           ((NULL != x) && ('\0' != *x))
#define ARRAY_SIZE(x)           (sizeof(x)/sizeof(x[0]))
#define DIGIT_TO_CHAR(Digit)    ((sbyte)(Digit - '0'))

#define IS_RAPIDMARK_START(x)  \
((kMagicMarkupStartChar0 == *x) && (kMagicMarkupStartChar1 == *(x + 1)))

#define IS_RAPIDMARK_END(x)  \
((kMagicMarkupEndChar0 == *x) && (kMagicMarkupEndChar1 == *(x + 1)))

#if (   defined(__EPILOGUE_ENVOY_SNMP_STACK__)                  ||  \
        defined(__EPILOGUE_ENVOY_MASTER_SUBAGENT_SNMP_STACK__)  ||  \
        defined(__WINDNET_SNMP_BINARY__)                        ||  \
        defined(__WINDNET_SNMP_SOURCE__)    )
#define ENVOY_STACK_K
#endif      


/* debugging aids */
#ifndef __DEBUG_MACRO__
#define __DEBUG_MACRO__

#ifdef __USE_STDERR__
#define DEBUG_FILE stderr
#else
#define DEBUG_FILE stdout
#endif

#ifdef __RCB_DEBUG__
#  define DEBUG_MSG_0(msg)          fprintf(DEBUG_FILE, msg)
#  define DEBUG_MSG_1(msg, x)       fprintf(DEBUG_FILE, msg, x)
#  define DEBUG_MSG_2(msg, x, y)    fprintf(DEBUG_FILE, msg, x, y)
#  define DEBUG_MSG_3(msg, x, y, z) fprintf(DEBUG_FILE, msg, x, y, z)
#else
#  define DEBUG_MSG_0(msg)
#  define DEBUG_MSG_1(msg, x)
#  define DEBUG_MSG_2(msg, x, y)
#  define DEBUG_MSG_3(msg, x, y, z)
#endif /* __RCC_DEBUG__ */

#endif /* __DEBUG_MACRO__ */

#endif /* __RLSTDDEF_HEADER__ */
