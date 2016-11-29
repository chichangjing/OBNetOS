/*
 *  rc_database.h
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


$History: rc_database.h $
 * 
 * *****************  Version 19  *****************
 * User: Pstuart      Date: 3/27/01    Time: 1:42p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Removed DB_HandleGetNextIndexEx proto
 * 
 * *****************  Version 18  *****************
 * User: Pstuart      Date: 12/11/00   Time: 11:30a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * added c++ wrappers for prototypes
 * 
 * *****************  Version 17  *****************
 * User: Leech        Date: 5/30/00    Time: 2:55p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * changed order of data type enums back to original (fixes Import Markups
 * From Device bug in the tool)
 * 
 * *****************  Version 16  *****************
 * User: Dreyna       Date: 5/04/00    Time: 7:30p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * CLI Preamble Dialog box and Access Table generation
 * 
 * *****************  Version 15  *****************
 * User: Pstuart      Date: 5/04/00    Time: 10:52a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Added prototype field for DTNames
 * 
 * *****************  Version 14  *****************
 * User: Epeterson    Date: 4/25/00    Time: 2:13p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Include history and enable auto archiving feature from VSS


$/Rapid Logic/Code Line/rli_code/include/rc_database.h

13   Dreyna      4/17/00   5:56p   Checked in $/Rapid Logic/Code Line/rli_code/include    
12   Pstuart     4/11/00   2:51p   Checked in $/Rapid Logic/Code Line/rli_code/include    
11   Schew       3/22/00   7:32p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Pstuart     3/22/00  10:53a   Labeled 'PreCodeRename'                                
     David       3/07/00   8:06p   Labeled 'RC 3.01 Final Release'                        
     Builder     3/02/00   5:18p   Labeled 'RC 301 Build20000302'                         
     Builder     2/29/00   6:48p   Labeled 'Build301 20000229'                            
     Builder     2/28/00   6:35p   Labeled 'RC301 Build20000228'                          
     Builder     2/24/00   4:01p   Labeled 'RC301 Build20000224'                          
10   Pstuart     2/16/00  11:37a   Checked in $/Rapid Logic/Code Line/rli_code/include    
9    Pstuart     2/11/00  11:47a   Checked in $/Rapid Logic/Code Line/rli_code/include    
     David      12/20/99  10:55a   Labeled 'RC 3.0 Final Release'                         
     Builder    12/15/99   6:36p   Labeled 'RC30 Build19991215'                           
8    Dreyna     12/15/99  11:19a   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Builder    12/14/99   7:08p   Labeled 'RC30 Build19991214'                           
     Builder    12/10/99   5:55p   Labeled 'RC30 Build19991210'                           
     Builder    11/17/99   4:32p   Labeled 'RC30 Build19991117 Release'                   
     Builder    11/16/99   5:37p   Labeled 'RC30 Build19991116'                           
     Builder    11/15/99   5:59p   Labeled 'RC30 Build19991115'                           
     Builder    11/12/99   5:27p   Labeled 'RC30 Build19991112'                           
7    Dreyna     11/12/99   1:33p   Checked in $/Rapid Logic/Code Line/rli_code/include    
6    Dreyna     11/12/99  12:27p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Builder    11/11/99   7:00p   Labeled 'RC30 Build19991111'                           
     Builder    11/10/99   7:12p   Labeled 'RC30 Build19991110'                           
     Builder    11/08/99   6:09p   Labeled 'RC30 Build19991108'                           
     Builder    11/05/99   6:33p   Labeled 'RC30 Build19991105'                           
     Builder    11/04/99   5:04p   Labeled 'RC30 Build19991104'                           
     Builder    11/03/99   6:23p   Labeled 'RC3.0 Build 19991103'                         
     Builder    11/01/99   5:51p   Labeled 'RC 3.0 Build 19991101'                        
     Builder    10/29/99   5:30p   Labeled 'RC30 Build 19991029'                          
5    Kedron     10/23/99   5:39p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     David       9/28/99   5:42p   Labeled 'RC 3.0 beta 2 - 9-28-99'                      
     Builder     8/10/99   2:41p   Labeled 'RC 3.0 alpha 4 limited release'               
     David       8/04/99   1:24p   Labeled 'RC 2.4 release'                               
     Builder     7/29/99   5:56p   Labeled 'RCA/RCW/Mibway 2.4 Final'                     
     David       7/20/99   2:04p   Labeled 'RC 2.4 beta 6 training '                      
4    Kedron      5/27/99   8:54a   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Leech       4/29/99  11:10a   Labeled 'WC/JC 2.31 Release - 4-29-99'                 
3    Jab         4/27/99   2:31p   Checked in $/Rapid Logic/Code Line/rli_code/include    
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
2    Henry      11/06/98   2:38p   Checked in $/Rapid Logic/Code Line/rli_code/include    
     Leech       8/07/98   4:28p   Labeled '2.01'                                         
1    Leech       7/27/98   6:24p   Created database.h                                     


*/



#ifndef __DATABASE_HEADER__
#define __DATABASE_HEADER__

/* constants */
enum    DataTypes           {kDTmacro, kDTstring, kDTipaddress,
                             kDTinteger, kDTchar, kDTshort, kDTlong,
                             kDTuinteger, kDTuchar, kDTushort, kDTulong, kDTvoid, kDTinvalid, 
							 kDTmacaddr, kDTenum, kDTlist, kDTabsolute, kDTnull, kDTaccess };

/* datatype labels */
#define kDTNinvalid    "Invalid"
#define kDTNmacro      "Macro"
#define kDTNstring     "String"
#define kDTNipaddress  "IP Address"
#define kDTNinteger    "Integer"
#define kDTNchar       "Char"
#define kDTNshort      "Short"
#define kDTNlong       "Long"
#define kDTNuinteger   "Unsigned Integer"
#define kDTNuchar      "Unsigned Char"
#define kDTNushort     "Unsigned Short"
#define kDTNulong      "Unsigned Long"
#define kDTNmacaddr    "MAC Address"
#define kDTNenum       "Enumerate"
#define kDTNlist       "List Elements"
#define kDTNabsolute   "Absolute"
#define kDTNnull       "Null"
#define kDTNvoid       "Void"
#define kDTNaccess     "Access"

typedef struct DTNames
{
    DataType  type;
    sbyte    *label;
    sbyte    *prototype;
} DTNames;

/* object access */
#define kDisabledAccess     0
#define kReadAccess         1
#define kWriteAccess        2
#define kEncryptData        4
#define kReadWriteAccess    (kReadAccess | kWriteAccess)

/* cache object access */
#define kCacheNO            0
#define kCacheRead          1
#define kCacheWrite         2
#define kCacheReadWrite     (kCacheRead | kCacheWrite)

/* element(MagicMarkup) types */
#define kMacroCommand       0
#define kIndexedElement     1
#define kNormalElement      2

/* datatypes & structures */
struct environment;

typedef struct  StrcDes                 /* (basename:  sdr) */
{
    void    (*p_funcObjLockIt)(UniqId); /* lock down the data object */
    void    (*p_funcObjUnlock)(UniqId); /* unlock the data object */

    void*   (*p_funcStrucLoc)(struct environment *, UniqId);        /* return pointer to the object */
    void*   (*p_funcIndexNext)(struct environment *, void *, int);  /* only if an indexable element */

    void    (*p_funcWrStruct)(struct environment *, UniqId, void *); /* object write back function */

    Access  CacheAccess;                /* cache permissions */
    Length  SizeOfStruct;               /* size of the structure */
    UniqId  StructureIdentifer;         /* Unique identifer to the structure the  */
                                        /* data object is contained in. */
                                        /* needed for the cache system's book keeping */
} StrcDes;                              /* aka StructureDescriptor */

typedef struct  DataBaseEntry           /* (basename:  dbe) */
{
    sbyte   *pName;                     /* name of the database object */
    Access  Permissions;                /* read access for object */
    Access  UserLevelRead;              /* access level req'd to read obj */
    Access  UserLevelWrite;             /* access level req'd to write obj */
    DataType iDataType;                 /* datatype for the object */
    void    *pDataObject;               /* pointer to the object */

    int     (*p_funcValid) (struct environment *, void *);      /* validation function ptr */
    void    (*p_funcRdPrim)(struct environment *, void *, void *, char *);  /* retrieval function ptr */
    int     (*p_funcWrPrim)(struct environment *, void *, void *, ...);  /* storage function ptr */

    StrcDes *p_sdrObject;               /* object structure descriptor */
    Counter OffsetIntoStruct;           /* the data's offset into the object structure */

} DataBaseEntry;

typedef struct IndexedElement           /* (basename:  idx) */
{
    void*       pDynamicLocation;          
    int         Index;
    RLSTATUS    StateOfIndexedResource; /* has it been updated */
    StrcDes*    p_sdrObject;            /* object structure descriptor */

} IndexedElement;



/* prototypes */

#ifdef __cplusplus
extern "C" {
#endif

/* database initialization routines */
extern RLSTATUS   DB_Construct(void);
extern RLSTATUS   DB_LockDown(void);
extern RLSTATUS   DB_CreateEntry(
                               sbyte    *pMagicMarkup, 
                               void     *pDataObject,
                               DataType iDataType, 
                               Access   Permissions, 
                               Access   ReadLvl, 
                               Access   WriteLvl,
                               int      (*p_funcValid)(struct environment *, void *),
                               void     (*p_funcReadPrim)(struct environment *, void *, void *, char *),
                               int      (*p_funcWritePrim)(struct environment *, void *, void *, ...),
                               StrcDes  *p_sdrStructDesc, 
                               Counter  StructOffset     );

#ifndef __DISABLE_STRUCTURES__
extern StrcDes* DB_CreateStrucDesc (
                               UniqId Id, Length LenStruct, Access CachePermissions,
                               void *(*p_funcObjLocation)(struct environment *, UniqId), 
                               void *(*p_funcIndexNext)(struct environment *, void *, int),
                               void  (*p_funcObjWrite)(struct environment *, UniqId, void *),
                               void  (*p_funcObjLockIt)(UniqId),
                               void  (*p_funcObjUnlock)(UniqId) );
#endif /* __DISABLE_STRUCTURES__ */

/* query database methods */
extern RLSTATUS DB_QueryValue   (struct environment *envVar, sbyte *pMagicMarkup, sbyte *pValue );

#ifndef __DISABLE_STRUCTURES__
extern void*    DB_GetSDR       (sbyte *pMagicMarkup);
#endif /* __DISABLE_STRUCTURES__ */

/* index methods */
extern RLSTATUS DB_ElementType        (sbyte *pMagicMarkup, ubyte2 *p_elementType);

#if defined(__JAVACONTROL_ENABLED__) || defined(__OCP_ENABLED__)
extern RLSTATUS DB_GetDataType(sbyte *pMagicMarkup, DataType *pDataType);

#ifndef __DATABASE_USE_ARRAY__
extern void *DB_GetDataBase();
#endif /* __DATABASE_USE_ARRAY__ */

#endif /* __JAVACONTROL_ENABLED__ */

#ifndef __DISABLE_STRUCTURES__
extern RLSTATUS DB_HandleGetNextIndex   (struct environment *envVar);
#endif /* __DISABLE_STRUCTURES__ */

/* methods for accessing data objects within the indigenous system */
extern RLSTATUS DB_WriteData      (struct environment *envVar, sbyte *pMagicMarkup, sbyte *pData, sbyte *pArgs);
extern RLSTATUS DB_ReadData       (struct environment *envVar, sbyte *pMagicMarkup, sbyte *pBuf, sbyte *pArgs);

#ifndef __DISABLE_STRUCTURES__
extern RLSTATUS DB_ReadIndexedData(struct environment *envVar, sbyte *pMagicMarkup, sbyte *pBuf, sbyte *pArgs);
#endif /* __DISABLE_STRUCTURES__ */

/* cache/ structure related methods */

/* Private Prototypes */

#ifndef __DISABLE_STRUCTURES__
extern Length   DB_Private_SizeOfStruct     (DataBaseEntry *p_dbeCell);
extern void*    DB_Private_ObjectIndexedAddr(  struct environment *envVar, DataBaseEntry *p_dbeCell);
#ifdef __MACRO_REPEAT_NEST__
extern void*    DB_Private_ObjectIndexedAddrEx(struct environment *envVar, DataBaseEntry *p_dbeCell);
#endif
extern void*    DB_Private_GetSDR           (DataBaseEntry *p_dbeCell);

extern void     DB_SDR_Private_LockIt   (void *p_sdrStructDesc);
extern void     DB_SDR_Private_UnLock   (void *p_sdrStructDesc);
extern void*    DB_SDR_Private_ObjectLoc(struct environment *envVar, void *p_sdrStructDesc);
extern RLSTATUS DB_SDR_Private_WrStruct (struct environment *p_envVar, void *p_sdrStructDesc, void *pCachedObject);
extern Length   DB_SDR_Private_SizeOf   (void *p_sdrStructDesc);
#endif /* __DISABLE_STRUCTURES__ */

#ifdef __cplusplus
}
#endif

#endif


