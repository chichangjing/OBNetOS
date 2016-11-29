/*
 *  rc_environ.h
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



$History: rc_environ.h $
 * 
 * *****************  Version 41  *****************
 * User: Pstuart      Date: 3/27/01    Time: 2:13p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * 
 * *****************  Version 40  *****************
 * User: Pstuart      Date: 3/27/01    Time: 11:29a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * added access macros for rapidmark macro data
 * 
 * *****************  Version 39  *****************
 * User: Pstuart      Date: 2/26/01    Time: 10:50a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * use new envoy macro
 * 
 * *****************  Version 38  *****************
 * User: Pstuart      Date: 2/23/01    Time: 3:28p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * More environment access macros; move cli specific defs to rcc
 * 
 * *****************  Version 37  *****************
 * User: Pstuart      Date: 12/14/00   Time: 11:41a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * moved lineout struct / macros back to rcc_structs.h
 * 
 * *****************  Version 36  *****************
 * User: Pstuart      Date: 12/11/00   Time: 11:30a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * added c++ wrappers for prototypes
 * 
 * *****************  Version 35  *****************
 * User: Pstuart      Date: 12/01/00   Time: 12:22p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * expanded LineOut struct
 * 
 * *****************  Version 34  *****************
 * User: Pstuart      Date: 11/17/00   Time: 6:58p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * move LineOut struct here
 * 
 * *****************  Version 33  *****************
 * User: Pstuart      Date: 11/10/00   Time: 2:39p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * add support for session directories to environment
 * 
 * *****************  Version 32  *****************
 * User: Pstuart      Date: 10/31/00   Time: 11:54a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added support for separate directories per session
 * 
 * *****************  Version 31  *****************
 * User: Pstuart      Date: 10/19/00   Time: 10:08a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * 
 * *****************  Version 30  *****************
 * User: Pstuart      Date: 8/02/00    Time: 10:11a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added access macro for snmp env
 * 
 * *****************  Version 29  *****************
 * User: Pstuart      Date: 6/07/00    Time: 6:44p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * 
 * *****************  Version 28  *****************
 * User: Pstuart      Date: 6/07/00    Time: 6:38p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * 
 * *****************  Version 27  *****************
 * User: Schew        Date: 5/26/00    Time: 2:16p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Adding support for WindNET SNMP.
 * 
 * *****************  Version 26  *****************
 * User: Pstuart      Date: 5/18/00    Time: 4:13p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Changes for move to RCC v2
 * 
 * *****************  Version 25  *****************
 * User: Epeterson    Date: 4/25/00    Time: 2:13p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Include history and enable auto archiving feature from VSS


$/Rapid Logic/Code Line/rli_code/include/rc_environ.h

24   Pstuart     4/11/00   2:53p   Checked in $/Rapid Logic/Code Line/rli_code/include    
23   Dreyna      3/30/00   6:26p   Checked in $/Rapid Logic/Code Line/rli_code/include    
22   Schew       3/22/00   7:32p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Pstuart     3/22/00  10:53a   Labeled 'PreCodeRename'                                
     David       3/07/00   8:06p   Labeled 'RC 3.01 Final Release'                        
     Builder     3/02/00   5:18p   Labeled 'RC 301 Build20000302'                         
     Builder     2/29/00   6:48p   Labeled 'Build301 20000229'                            
     Builder     2/28/00   6:35p   Labeled 'RC301 Build20000228'                          
     Builder     2/24/00   4:01p   Labeled 'RC301 Build20000224'                          
21   Dreyna      1/04/00   6:43p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     David      12/20/99  10:55a   Labeled 'RC 3.0 Final Release'                         
     Builder    12/15/99   6:36p   Labeled 'RC30 Build19991215'                           
     Builder    12/14/99   7:08p   Labeled 'RC30 Build19991214'                           
     Builder    12/10/99   5:55p   Labeled 'RC30 Build19991210'                           
20   Dreyna     12/07/99  11:44a   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Builder    11/17/99   4:32p   Labeled 'RC30 Build19991117 Release'                   
     Builder    11/16/99   5:37p   Labeled 'RC30 Build19991116'                           
19   Dreyna     11/16/99   2:21p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Builder    11/15/99   5:59p   Labeled 'RC30 Build19991115'                           
     Builder    11/12/99   5:27p   Labeled 'RC30 Build19991112'                           
18   Dreyna     11/12/99  12:27p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Builder    11/11/99   7:00p   Labeled 'RC30 Build19991111'                           
     Builder    11/10/99   7:12p   Labeled 'RC30 Build19991110'                           
     Builder    11/08/99   6:09p   Labeled 'RC30 Build19991108'                           
17   Dreyna     11/08/99  11:46a   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Builder    11/05/99   6:33p   Labeled 'RC30 Build19991105'                           
     Builder    11/04/99   5:04p   Labeled 'RC30 Build19991104'                           
     Builder    11/03/99   6:23p   Labeled 'RC3.0 Build 19991103'                         
16   Dreyna     11/03/99   3:33p   Checked in $/Rapid Logic/Code Line/rli_code/include    
15   Henry      11/02/99   1:13p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Builder    11/01/99   5:51p   Labeled 'RC 3.0 Build 19991101'                        
     Builder    10/29/99   5:30p   Labeled 'RC30 Build 19991029'                          
14   Kedron     10/23/99   5:40p   Checked in $/Rapid Logic/Code Line/rli_code/include    
13   Jab         9/29/99   4:30p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     David       9/28/99   5:42p   Labeled 'RC 3.0 beta 2 - 9-28-99'                      
     Builder     8/10/99   2:41p   Labeled 'RC 3.0 alpha 4 limited release'               
     David       8/04/99   1:24p   Labeled 'RC 2.4 release'                               
     Builder     7/29/99   5:56p   Labeled 'RCA/RCW/Mibway 2.4 Final'                     
     David       7/20/99   2:04p   Labeled 'RC 2.4 beta 6 training '                      
12   Kedron      5/27/99   8:54a   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Leech       4/29/99  11:10a   Labeled 'WC/JC 2.31 Release - 4-29-99'                 
11   Jab         4/27/99   3:11p   Checked in $/Rapid Logic/Code Line/rli_code/include    
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
10   Henry      12/18/98   9:52a   Checked in $/Rapid Logic/Code Line/rli_code/include    
9    Henry      12/17/98  10:36a   Checked in $/Rapid Logic/Code Line/rli_code/include    
8    Henry      12/17/98  10:27a   Checked in $/Rapid Logic/Code Line/rli_code/include    
     David      12/10/98   6:29p   Labeled 'JC 12/10'                                     
     Henry      12/07/98   5:27p   Labeled 'JC 12/7'                                      
     Henry      12/04/98   3:35p   Labeled 'Build 12/4'                                   
7    Jab        11/01/98   4:15p   Checked in $/Rapid Logic/Code Line/rli_code/include    
6    Jamesb     10/30/98   3:38p   Checked in $/Rapid Logic/Code Line/rli_code/include    
5    Henry      10/28/98  12:54p   Checked in $/Rapid Logic/Code Line/rli_code/include    
4    Jamesb     10/14/98  11:01p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Leech       8/07/98   4:28p   Labeled '2.01'                                         
3    Jamesb      8/05/98   9:08a   Checked in $/Rapid Logic/Code Line/rli_code/include    
2    Kedron      7/29/98   8:50a   Checked in $/Rapid Logic/Code Line/rli_code/include    
1    Leech       7/27/98   6:25p   Created environ.h                                      


*/

#ifndef __ENVIRONMENT_HEADER__
#define __ENVIRONMENT_HEADER__

#include "rc_mmarkup.h"

#ifdef __ENABLE_SESSION_DIRECTORIES__
#ifndef kDIRECTORY_BUFFER_SIZE
#define kDIRECTORY_BUFFER_SIZE 128
#endif
#endif

#define kMaxEnvSize             19
#define __ENABLE_CUSTOM_STRUCT__

/* constants */
#define kenv_REQUEST_METHOD     0
#define kenv_DOCUMENT_NAME      1
#define kenv_ACCEPT             2
#define kenv_AUTHORIZATION      3
#define kenv_COOKIE             4
#define kenv_CONTENT_LENGTH     5
#define kenv_CONTENT_TYPE       6
#define kenv_CONNECTION         7
#define kenv_HOST               8
#define kenv_LANGUAGE           9
#define kenv_PRAGMA             10
#define kenv_REFERER            11
#define kenv_USER_AGENT         12
#define kenv_QUERY_STRING       13
#define kenv_HTTP_VERSION       14
/* used for WSA */
#define kenv_REMOTE_ADDR		15
#define kenv_REMOTE_PORT		16
#define kenv_SERVER_SOFTWARE	17
#define kenv_SCRIPT_NAME		18

/* Transmission Flow Constants */
#define kFlowOff    0
#define kFlowOn     1

enum	BladeType 
{
	kBTunknown,
	kBTwc,
	kBTocp,
	kBTcli,
    kBTwws          /* Wind Web Server */
};

/*-----------------------------------------------------------------------*/
/*  Structures needed for Environment Variables */

typedef struct CacheHandle                  /* basename:  cac */
{
    struct  list    *p_lstCacheObjects;     /* next object in the list */
    Access          AccessType;             /* cache access (Read/Write) */

} CacheHandle;



typedef struct  environment                     /* (basename:  env) */
{
 
/* vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 *	The QSTART library ENVOYSAM.LIB depends on this block of environment  
 *	variables to remain in this order. If you make a change here, you 
 *	must recompile ENVOYSAM.LIB 
 * vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/

	/* macro method variables */
    Counter             MacroIndex;
    list*               p_lstIndexedValues;     /* list of indexed values */
    sbyte*              pData;
    Length              cBytesAnalyzed;
    RLSTATUS            TxFlowStatus;           /* flow control for the tx eng */
#ifdef __SNMP_API_ENABLED__
    sbyte               MacroTableName[40];     /* used for snmp, need to determine size!!!!!!! */
    void*               pSnmpData;
#endif

    Counter             ScopeDepth;             /* used for nesting macro commands */

    Boolean             isPersistent;           /* used in HTTP 1.1 */
    Boolean             sendChunkedData;        /* used in HTTP 1.1 */
	enum BladeType		blade;					/* identify where request is coming from */

#ifdef ENVOY_STACK_K
	void*				pIntSetSnmpPkt;
#endif

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *	ENVOYSAM.LIB dependant environment block 
 *
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

    sbyte*              variables[kMaxEnvSize]; /* list of pointers to the env variables */
    struct CacheHandle* phCacheHandle;
    Access              UserLevel;

    OS_SPECIFIC_SOCKET_HANDLE sock;
    ubyte4              IpAddr;
    sbyte4              clientIndex;

    Boolean             PostValid;
    sbyte*              PostBuffer;
    Counter             PostBufferLen;

#ifdef __ENABLE_CUSTOM_STRUCT__
    void*               pCustomData;            /* custom session data */
#endif

#ifdef __MFWWS_ENABLED__
    char*               pMFWWS_OutputBuf;
#endif /* __MFWWS_ENABLED__ */

#ifdef __OCP_ENABLED__

    list*               plst_MmOps;
    sbyte*              pContext;           /* context string */
	Boolean				bIsSetOp;			/* true if operation will set MM value */
#endif  /* __OCP_ENABLED__ */

	/* The nested Repeat variables are added at the end so that older libs (like envoysam.lib) 
	   will find the original environments variables in the normal place. rcw_Macro.h handles
	   the old variables to every thing in sync. */
	/* Macro Runtime Stack */
#ifdef	__WEBCONTROL_BLADE_ENABLED__
    sbyte2              Macro_Status;	        /* macro execution status*/
    int	                Macro_NestDepth;        /* used for nesting macro commands */
    Counter             Macro_Index[            kMacroRepeatNestMax];	/* nested index couter */
    list*               p_MacroLstIndexedValues[kMacroRepeatNestMax];	/* list of indexed rapidmarks*/
#ifdef __SNMP_API_ENABLED__
#ifdef __MACRO_REPEAT_NEST_FULL__
    sbyte               Macro_TableName[ kMacroRepeatNestMax][40];		/* used for snmp, need to determine size!!!!!!! */
    void*               p_MacroSnmpData[ kMacroRepeatNestMax];
#endif
#endif
	/* Macro Semantic Stack */
    int	                Macro_Counter;					  /* used for nesting macro commands */
    int                 Macro_Type[      kMacroTableMax]; /* macro repeat,if,... */
    sbyte*              Macro_pStart[    kMacroTableMax]; /* macro data start */   
    Length              Macro_Length[    kMacroTableMax]; /* macro data length */
    sbyte*              Macro_pExit[     kMacroTableMax]; /* data ptr to after macro */  
    sbyte*              Macro_pArgsBegin[kMacroTableMax]; /* Begin arguments Pointer */
    sbyte*              Macro_pArgsEnd[  kMacroTableMax]; /* End   arguments Pointer */   
#endif /* __WEBCONTROL_BLADE_ENABLED__ */
    void *              pConsumer;     /* blade specific info */
#ifdef __ENABLE_SESSION_DIRECTORIES__
#ifdef __ANSI_FILE_MANAGER_ENABLED__
    void               *pDirHandle;
#endif /* __ANSI_FILE_MANAGER_ENABLED__ */
    sbyte               cwd[kDIRECTORY_BUFFER_SIZE];
#endif
} environment;


#define SESSION_CONSUMER_GET_M(pEnv)        pEnv->pConsumer
#define SESSION_CONSUMER_SET_M(pEnv,x)      pEnv->pConsumer = x
#define SESSION_CACHE_GET_M(pEnv)           &(pEnv->phCacheHandle)
#define SESSION_CACHE_SET_M(pEnv,x)         pEnv->phCacheHandle = x
#define SESSION_INDEXED_VALUES_GET_M(pEnv)  (pEnv)->p_lstIndexedValues
#define SESSION_INDEXED_VALUES_SET_M(pEnv,x) (pEnv)->p_lstIndexedValues = x
#define SESSION_VARIABLES_GET_M(pEnv)       (pEnv)->variables
#define SESSION_VARIABLES_SET_M(pEnv,x)     (pEnv)->variables = x
#define SESSION_POSTBUFFER_GET_M(pEnv)      (pEnv)->PostBuffer
#define SESSION_POSTBUFFER_SET_M(pEnv)      (pEnv)->PostBuffer = x
#define SESSION_SOCK_GET_M(pEnv)            (pEnv)->sock
#define SESSION_SOCK_SET_M(pEnv,x)          (pEnv)->sock = x


#ifdef __cplusplus
extern "C" {
#endif


extern RLSTATUS     ENVIRONMENT_Construct(OS_SPECIFIC_SOCKET_HANDLE sock,
                                          environment **pp_envInit);
extern RLSTATUS     ENVIRONMENT_Destruct (environment **pp_envTemp);

/* request data access */
extern  sbyte*      ENVIRONMENT_GetEnvironment(environment *p_envVar, Counter index);
extern RLSTATUS     ENVIRONMENT_PutEnvironment(environment *p_envVar,
                                               Counter index,
                                               sbyte *pValue);
extern RLSTATUS     ENVIRONMENT_ReplaceEnvironment(environment *p_envVar,
                                            Counter index, sbyte *pValue);

extern  Access      ENVIRONMENT_GetAccessLevel(environment *p_envVar);
extern  Boolean     ENVIRONMENT_IsPostValid(environment *p_envVar);
extern  void        ENVIRONMENT_SetPostInvalid(environment *p_envVar);
extern enum BladeType ENVIRONMENT_GetBladeType(environment *p_envVar);

#ifdef __ENABLE_CUSTOM_STRUCT__
extern void ENVIRONMENT_SetUpCustomVectors  ( int  (*p_funcConstructCustom)(environment *p_envVar, void **ppCustomData),
                                              void (*p_funcDestructCustom) (void *pCustomData) );
extern void *ENVIRONMENT_GetPointerToCustomData(environment *p_envVar);
#endif

#ifdef __OCP_ENABLED__
extern  sbyte*      ENVIRONMENT_GetContext(environment *p_envVar);
extern RLSTATUS     ENVIRONMENT_PutContext(environment *p_envVar, sbyte *pValue);
extern Boolean ENVIRONMENT_isSetOp(environment *p_envVar);
#endif

#ifndef __MACRO_REPEAT_NEST_FULL__
#define GET_SNMP_ENV_PTR(pEnv)  pEnv->pSnmpData
#else
#define GET_SNMP_ENV_PTR(pEnv)  pEnv->p_MacroSnmpData[pEnv->Macro_NestDepth]
#endif  /* __MACRO_REPEAT_NEST_FULL__ */

#define MMISC_EnvBladeGet(pEnv)     ((pEnv)->blade)
#define MMISC_EnvBladeSet(pEnv,x)   ((pEnv)->blade = x)

#ifdef __ENABLE_SESSION_DIRECTORIES__
#ifdef __ANSI_FILE_MANAGER_ENABLED__
#define MFILE_GetCwdPtr(pEnv)       (pEnv->cwd)
#define MFILE_GetDirHandle(pEnv)    (pEnv->pDirHandle)
#endif /* __ANSI_FILE_MANAGER_ENABLED__ */
#else
#define MFILE_GetCwdPtr(pEnv)       (NULL)
#define MFILE_GetDirHandle(pEnv)    (NULL)
#endif /* __ENABLE_SESSION_DIRECTORIES__ */


#ifdef __cplusplus
}
#endif


#endif /* __ENVIRONMENT_HEADER__ */
