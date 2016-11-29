/*
 *  rc_database.c
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
$History: rc_database.c $
 * 
 * *****************  Version 34  *****************
 * User: Pstuart      Date: 6/25/01    Time: 3:06p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * compare of type string needed to use strlen(string)
 * 
 * *****************  Version 33  *****************
 * User: Pstuart      Date: 3/27/01    Time: 1:40p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * consolidated DB_HandleGetNextIndexEx into DB_HandleGetNextIndex
 * 
 * *****************  Version 32  *****************
 * User: Leech        Date: 1/18/01    Time: 7:27p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Added rc_occustom.h to include dirs (fixes compile problem if external
 * APIs are enabled)
 * 
 * *****************  Version 31  *****************
 * User: Pstuart      Date: 11/17/00   Time: 7:03p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * use rc.h
 * 
 * *****************  Version 30  *****************
 * User: Dreyna       Date: 6/20/00    Time: 3:39p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Updated History
 * 
 * *****************  Version 28  *****************
 * User: Epeterson    Date: 4/26/00    Time: 2:06p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Enabled VSS auto-archive feature using keyword expansion
 * $History: rc_database.c $
 * 
 * *****************  Version 34  *****************
 * User: Pstuart      Date: 6/25/01    Time: 3:06p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * compare of type string needed to use strlen(string)
 * 
 * *****************  Version 33  *****************
 * User: Pstuart      Date: 3/27/01    Time: 1:40p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * consolidated DB_HandleGetNextIndexEx into DB_HandleGetNextIndex
 * 
 * *****************  Version 32  *****************
 * User: Leech        Date: 1/18/01    Time: 7:27p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Added rc_occustom.h to include dirs (fixes compile problem if external
 * APIs are enabled)
 * 
 * *****************  Version 31  *****************
 * User: Pstuart      Date: 11/17/00   Time: 7:03p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * use rc.h
 * 
 * *****************  Version 30  *****************
 * User: Dreyna       Date: 6/20/00    Time: 3:39p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Updated History
 * 



*/

#include "rc.h"
#include "rc_sizeof.h"

#ifdef __DATABASE_USE_HASH__
#include "rc_hash.h"
#endif

#ifdef  __DATABASE_USE_BTREE__
#include "rc_tree.h"
#endif

#ifdef __WEBCONTROL_BLADE_ENABLED__
/* Enable Nested Repeat Variable Lookups */
#include "rcw_macro.h"
#endif
#include "rc_occustom.h"

/*-----------------------------------------------------------------------*/

#ifdef __DATABASE_USE_ARRAY__
extern DataBaseEntry    gDataBaseEntryTable;
extern Length           gDataBaseTableSize;
#endif

#ifdef __SNMP_API_ENABLED__
extern int OCSNMP_MibObjectType(void *pFullMibName);

extern RLSTATUS OCSNMP_GetMibVar(environment *p_envVar, sbyte *pMagicMibMarkup,
                                 sbyte *pHtmlArgs, sbyte *pHtmlOutputBuf, void *p_func);

extern int  OCSNMP_SetMibVar(environment *p_envVar, sbyte *pMagicMibMarkup,
                             sbyte *pHtmlArgs, sbyte *pHtmlInputBuf, void *p_func);

#endif /* __SNMP_API_ENABLED__ */


/*-----------------------------------------------------------------------*/

#define kDataBaseLocked     1
#define kDataBaseUnlocked   0



/*-----------------------------------------------------------------------*/

#ifndef __DATABASE_USE_ARRAY__
static  void*   pmDataBase;
#endif

static  int     mDataBaseStatus = kDataBaseUnlocked;



/*-----------------------------------------------------------------------*/

extern RLSTATUS DB_Construct(void)
{
#ifndef __DATABASE_USE_ARRAY__
    mDataBaseStatus = kDataBaseUnlocked;
#endif

#ifdef __DATABASE_USE_ARRAY__
    mDataBaseStatus = kDataBaseLocked;
#endif

#ifdef __DATABASE_USE_BTREE__
    if (NULL == (pmDataBase = (void *)TREE_Construct()))
        return ERROR_MEMMGR_NO_MEMORY;
#endif

#ifdef __DATABASE_USE_HASH__
    if (NULL == (pmDataBase = HASH_Construct()))
        return ERROR_MEMMGR_NO_MEMORY;
#endif

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS DB_LockDown(void)
{
#ifdef __DATABASE_USE_BTREE__
    RLSTATUS  errCond =  TREE_Balance((tree *)pmDataBase);

    mDataBaseStatus = kDataBaseLocked;

    return errCond;
#else
    mDataBaseStatus = kDataBaseLocked;

    return OK;
#endif
}



/*-----------------------------------------------------------------------*/

#ifdef __DATABASE_USE_ARRAY__

static CompareType
DB_ArrayObjectComparison(void *p_dbeObject, void *pMagicMarkup1)
{
    sbyte   *pMagicMarkup2;
    sbyte4  Result;

    pMagicMarkup2 = ((DataBaseEntry *)p_dbeObject)->pName;

    Result = STRCMP((sbyte *)pMagicMarkup1, pMagicMarkup2);

    if (0 < Result)
        return LESS_THAN;

    if (0 > Result)
        return GREATER_THAN;

    return EQUAL;
}

#endif



/*-----------------------------------------------------------------------*/

#ifdef __DATABASE_USE_HASH__

static void DB_HashKeyExtractByMagicMarkup( void *pStr, ubyte4 *pKey )
{
    ubyte4 count = 0;
    ubyte* pString = pStr;

    while ( *pString != '\0' )
    {
        count += *pString;
        pString++;
    }

    *pKey = count;
}

static void DB_HashKeyExtractByObject( void *p_dbeTemp, ubyte4 *pKey )
{
    ubyte4 count = 0;
    sbyte* pStr = ((DataBaseEntry *)p_dbeTemp)->pName;

    while ( *pStr != '\0' )
    {
        count += (ubyte)*pStr;
        pStr++;
    }

    *pKey = count;
}

static RLSTATUS p_dbeHashFindMagicMarkup(void *p_dbeTemp, void *pMagicMarkup)
{
    return (STRCMP(((DataBaseEntry *)p_dbeTemp)->pName, pMagicMarkup));
}

static RLSTATUS p_dbeHashFindObject(void *p_dbeExist, void *p_dbeNew)
{
    return (STRCMP(((DataBaseEntry *)p_dbeExist)->pName, ((DataBaseEntry *)p_dbeNew)->pName));
}

#endif  /* __DATABASE_USE_HASH__ */



/*-----------------------------------------------------------------------*/

#ifdef __DATABASE_USE_BTREE__

static TreeDir p_dbeTreeFindMagicMarkup(void *p_dbeTemp, void *pMagicMarkup)
{
    sbyte2 Result = (sbyte2)STRCMP(((DataBaseEntry *)p_dbeTemp)->pName, pMagicMarkup);

    if (0 == Result)
    {
        return (TreeDir)NodesAreEqual;
    }

    if (0 < Result)
    {
        return (TreeDir)GoToLeftNode;
    }

    return (TreeDir)GoToRightNode;
}

static TreeDir p_dbeTreeFindObject(void *p_dbeExist, void *p_dbeNew)
{
    sbyte2 Result = (sbyte2)STRCMP(((DataBaseEntry *)p_dbeExist)->pName, ((DataBaseEntry *)p_dbeNew)->pName);

    if (0 == Result)
    {
        return (TreeDir)NodesAreEqual;
    }

    if (0 < Result)
    {
        return (TreeDir)GoToLeftNode;
    }

    return (TreeDir)GoToRightNode;
}

#endif  /* __DATABASE_USE_BTREE__ */



/*-----------------------------------------------------------------------*/

static DataBaseEntry *p_dbeFindCell(sbyte *pMagicMarkup)
{
    if (NULL == pMagicMarkup)
        return NULL;

#ifdef __DATABASE_USE_BTREE__
    return TREE_FindObject((tree *)pmDataBase, (void *)pMagicMarkup, p_dbeTreeFindMagicMarkup);
#endif

#ifdef __DATABASE_USE_HASH__
    return HASH_FindObject((void *)pmDataBase, (void *)pMagicMarkup, DB_HashKeyExtractByMagicMarkup, p_dbeHashFindMagicMarkup);
#endif

#ifdef __DATABASE_USE_ARRAY__
    return BinSrch_FindObj(&gDataBaseEntryTable, gDataBaseTableSize, sizeof(DataBaseEntry), pMagicMarkup,
                           DB_ArrayObjectComparison);
#endif
}



/*-----------------------------------------------------------------------*/

#ifndef __DATABASE_USE_ARRAY__
extern RLSTATUS
DB_CreateEntry(
               sbyte    *pMagicMarkup,
               void     *pDataObject,
               DataType iDataType,
               Access   Permissions,
               Access   ReadLvl,
               Access   WriteLvl,
               RLSTATUS (*p_funcValid) (struct environment *, void *),
               void     (*p_funcReadPrim)(environment *, void *, void *, char *),
               RLSTATUS (*p_funcWritePrim) (environment *, void *, void *, ...),
               StrcDes  *p_sdrStructDesc,
               Counter  StructOffset     )
{
    DataBaseEntry   *p_dbeCell;
    sbyte           *CopiedMagicMarkup;

    if (kDataBaseUnlocked != mDataBaseStatus)
        return ERROR_GENERAL_ACCESS_DENIED;

    if (NULL == pMagicMarkup)
        return ERROR_GENERAL_NOT_FOUND;

    if (NULL == (p_dbeCell = (DataBaseEntry *)RC_MALLOC(sizeof(DataBaseEntry))))
        return ERROR_MEMMGR_NO_MEMORY;

    if (NULL == (CopiedMagicMarkup = (sbyte *)RC_MALLOC(STRLEN(pMagicMarkup) + 1)))
    {
        RC_FREE(p_dbeCell);

        return ERROR_MEMMGR_NO_MEMORY;
    }

    /* copy information */
    STRCPY(CopiedMagicMarkup, pMagicMarkup);
    p_dbeCell->pName            = CopiedMagicMarkup;
    p_dbeCell->pDataObject      = pDataObject;
    p_dbeCell->iDataType        = iDataType;
    p_dbeCell->Permissions      = Permissions;
    p_dbeCell->UserLevelRead    = ReadLvl;
    p_dbeCell->UserLevelWrite   = WriteLvl;
    p_dbeCell->p_funcValid      = p_funcValid;
    p_dbeCell->p_funcRdPrim     = p_funcReadPrim;
    p_dbeCell->p_funcWrPrim     = p_funcWritePrim;

    /* structural information ( for the cache system ) */
    p_dbeCell->p_sdrObject      = p_sdrStructDesc;
    p_dbeCell->OffsetIntoStruct = StructOffset;

#ifdef __DATABASE_USE_BTREE__
    return TREE_AddObject(pmDataBase, (void *)p_dbeCell, p_dbeTreeFindObject);
#endif

#ifdef __DATABASE_USE_HASH__
    return HASH_AddObject((void *)pmDataBase, (void *)p_dbeCell, DB_HashKeyExtractByObject, p_dbeHashFindObject);
#endif

} /* DB_CreateEntry */
#endif



/*-----------------------------------------------------------------------*/

extern RLSTATUS
DB_QueryValue(environment *p_envVar, sbyte *pMagicMarkup, sbyte *pValue )
{
    DataBaseEntry*      p_dbeCell = p_dbeFindCell(pMagicMarkup);

    if (NULL == p_dbeCell)
    {
#ifdef  __ENABLE_EXTERNAL_API__
        return CUSTOM_EXTERNAL_HANDLER(kDBQueryValue, p_envVar, pMagicMarkup, NULL, pValue, NULL, NULL);
#else
        return OK;
#endif
    }

    if (NULL != p_dbeCell->p_funcValid )
    {
        if ( OK > ( p_dbeCell->p_funcValid( p_envVar, pValue ) ) )
        {
            ENVIRONMENT_SetPostInvalid(p_envVar);
            return ERROR_GENERAL_ILLEGAL_VALUE;
        }
    }

    return OK;

} /* DB_QueryValue */



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

static RLSTATUS DB_StructureRead(environment *p_envVar, sbyte *pObject,
                                 sbyte *pBuf, DataBaseEntry *p_dbeCell, sbyte *pArgs)
{
    DataType DType   = p_dbeCell->iDataType;

    if (NULL != p_dbeCell->p_funcRdPrim)
    {
        p_dbeCell->p_funcRdPrim(p_envVar, pBuf, pObject, pArgs);

        return OK;
    }

    return CONVERT_ToStr(pObject, pBuf, DType);

}   /* DB_StructureRead */

#endif /* __DISABLE_STRUCTURES__ */



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

static RLSTATUS DB_StructureWrite(environment *p_envVar, sbyte *pData, sbyte *pDest,
                                  DataBaseEntry *p_dbeCell, sbyte *pInputArg)
{
    DataType    DType              = p_dbeCell->iDataType;
    UniqId      StructureIdentifer = p_dbeCell->p_sdrObject->StructureIdentifer;
    RLSTATUS    errCode;
    void*       pObjectTemp        = RC_MALLOC(SIZEOF_Type(DType));

    if (NULL == pObjectTemp)
        return ERROR_MEMMGR_NO_MEMORY;

    if (NULL == pDest)
    {
        RC_FREE(pObjectTemp);
        return ERROR_GENERAL_ACCESS_DENIED;
    }

    if (NULL != p_dbeCell->p_funcWrPrim)
    {
        RLSTATUS result;

        if (0 != (result = p_dbeCell->p_funcWrPrim(p_envVar, pDest, pData, pInputArg)))
            Cache_ObjectDirty(p_envVar->phCacheHandle, StructureIdentifer);

        if (kErrorChange == result)
            ENVIRONMENT_SetPostInvalid(p_envVar);

        RC_FREE(pObjectTemp);

        return OK;
    }

    /* move the data into a temporary buffer, and compare
     * with the previous value. if the value
     * has changed then update the storage */

    if (OK != (errCode = CONVERT_StrTo(pData, pObjectTemp, DType)))
    {
        /* conversion failed! */
        ENVIRONMENT_SetPostInvalid(p_envVar);
        RC_FREE(pObjectTemp);

        return errCode;
    }

    if (!(COMPARE_Values(pDest, pObjectTemp, DType)))
    {
        if (kDTstring == DType)
            MEMCPY(pDest, pObjectTemp, STRLEN(pObjectTemp) + 1);
        else
            MEMCPY(pDest, pObjectTemp, SIZEOF_Type(DType));

        Cache_ObjectDirty(p_envVar->phCacheHandle, StructureIdentifer);
    }

    RC_FREE(pObjectTemp);

    return errCode;

}   /* DB_StructureWrite */

#endif /* __DISABLE_STRUCTURES__ */



/*-----------------------------------------------------------------------*/

extern RLSTATUS DB_WriteData(environment *p_envVar, sbyte *pMagicMarkup,
                             sbyte *pData, sbyte *pInputArg)
{
    DataBaseEntry *p_dbeCell = p_dbeFindCell(pMagicMarkup);

    if (NULL == p_dbeCell)
    {
#ifdef  __ENABLE_EXTERNAL_API__
        return CUSTOM_EXTERNAL_HANDLER(kDBWriteData, p_envVar, pMagicMarkup, pInputArg, pData, NULL, NULL);
#else

#ifdef __SNMP_API_ENABLED__
		/* A zero or positive return value indicates success */
		if ( OK <= OCSNMP_SetMibVar(p_envVar, pMagicMarkup, pInputArg, pData, NULL))
			return OK;
#endif
        /* Return OK to allow the post engine to continue processing */
        /* no, fix post engine to deal with the error */
        return ERROR_GENERAL_NOT_FOUND;
#endif
    }

    if (! ((p_dbeCell->Permissions & kWriteAccess) &&
           RC_ACCESS_Allowed(p_dbeCell->UserLevelWrite, p_envVar->UserLevel)))
    {
        /* Return OK to allow the post engine to continue processing */
        /* no, fix post engine to deal with the error */
        return ERROR_GENERAL_ACCESS_DENIED;
    }
#ifndef __DISABLE_STRUCTURES__

    /* Structure Objects have the highest precedence */
    if (NULL != p_dbeCell->p_sdrObject)
    {
        UniqId  structureId = p_dbeCell->p_sdrObject->StructureIdentifer;

        if (p_dbeCell->p_sdrObject->CacheAccess & kCacheWrite)
        {
            sbyte *pObject = (sbyte *)Cache_FindObject( p_envVar,
                                                        p_envVar->phCacheHandle,
                                                        structureId,
                                                        p_dbeCell, FALSE);
            if (NULL == pObject)
                return ERROR_GENERAL_NOT_FOUND;

            pObject += p_dbeCell->OffsetIntoStruct;

            return DB_StructureWrite(p_envVar, pData, pObject, p_dbeCell, pInputArg);
        }

        if (NULL != p_dbeCell->p_sdrObject->p_funcStrucLoc)
        {
            sbyte *pObject = p_dbeCell->p_sdrObject->p_funcStrucLoc(p_envVar, structureId);

            if (NULL == pObject)
                return ERROR_GENERAL_NOT_FOUND;

            pObject += p_dbeCell->OffsetIntoStruct;

            return DB_StructureWrite(p_envVar, pData, pObject, p_dbeCell, pInputArg);
        }

        return ERROR_GENERAL_ACCESS_DENIED;
    }

#endif /* __DISABLE_STRUCTURES__ */

    /* Next, deal with write primitives */
    if (NULL != p_dbeCell->p_funcWrPrim)
    {
        if (kErrorChange == p_dbeCell->p_funcWrPrim(p_envVar, p_dbeCell->pDataObject, pData, pInputArg))
            ENVIRONMENT_SetPostInvalid(p_envVar);

        return OK;
    }

    /* Lastly, do direct writes */
    if (NULL != p_dbeCell->pDataObject)
        return CONVERT_StrTo(pData, p_dbeCell->pDataObject, p_dbeCell->iDataType);

    return OK;

} /* DB_WriteData */



/*-----------------------------------------------------------------------*/

extern RLSTATUS DB_ElementType(sbyte *pMagicMarkup, ubyte2 *p_elementType)
{
    DataBaseEntry *p_dbeCell = p_dbeFindCell(pMagicMarkup);

    if (NULL == p_dbeCell)
    {
#ifdef  __ENABLE_EXTERNAL_API__
        return CUSTOM_EXTERNAL_HANDLER(kDBElementType, NULL, pMagicMarkup, NULL, NULL, NULL, p_elementType);
#else
        *p_elementType = kNormalElement;
        return OK;
#endif
    }

#ifndef __DISABLE_STRUCTURES__
    if (kDTmacro == p_dbeCell->iDataType)
        *p_elementType = kMacroCommand;
    else
#endif /* __DISABLE_STRUCTURES__ */
        if ((NULL == p_dbeCell->p_sdrObject) ||
            (NULL == p_dbeCell->p_sdrObject->p_funcIndexNext))
        {
            *p_elementType = kNormalElement;
        }
        else
            *p_elementType = kIndexedElement;

    return OK;
}



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

static Boolean DB_MoveToNextIndex(void *p_envVar, void *pIndexElement)
{
    IndexedElement *pIdxElmnt = pIndexElement;

    /* lock down struct here */

    /* get next... */
    pIdxElmnt->pDynamicLocation =
      pIdxElmnt->p_sdrObject->p_funcIndexNext( (environment *) p_envVar,
                                               pIdxElmnt->pDynamicLocation,
                                               (++(pIdxElmnt->Index) ) );
    /* if enabled, copy item to cache here */

    /* unlock struct here */

    if ( NULL == pIdxElmnt->pDynamicLocation )
        return TRUE;        /* hit the end of the list! */

    return FALSE;
}

#endif /* __DISABLE_STRUCTURES__ */


/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

extern RLSTATUS DB_HandleGetNextIndex(environment *p_envVar)
{
    list *pList;
    void *pStatus;

#ifdef __MACRO_REPEAT_NEST__
	/* If new nested repeat structure is active, look up values via rcw_macro.c */
	if (p_envVar->Macro_Counter >= 0)
        pList = p_envVar->p_MacroLstIndexedValues[p_envVar->Macro_NestDepth];
    else
#endif
        pList = p_envVar->p_lstIndexedValues;

    pStatus = pList_FindObject(pList, p_envVar, DB_MoveToNextIndex);

    /* non-NULL indicates an index++ failed */
    if (NULL != pStatus)
        return ERROR_GENERAL_NO_DATA;

    return OK;
}

#endif /* __DISABLE_STRUCTURES__ */



/*-----------------------------------------------------------------------*/

extern RLSTATUS
DB_ReadData(environment *p_envVar, sbyte *pMagicMarkup, sbyte *pBuf, sbyte *pArgs)
{
    DataBaseEntry *p_dbeCell = p_dbeFindCell(pMagicMarkup);

    if (NULL == pBuf)
        return ERROR_GENERAL_NOT_FOUND;

    *pBuf = '\0';

    if (NULL == p_dbeCell)
    {
#ifdef  __ENABLE_EXTERNAL_API__
        return CUSTOM_EXTERNAL_HANDLER(kDBReadData, p_envVar, pMagicMarkup, pArgs, NULL, pBuf, NULL);
#else
#ifdef __SNMP_API_ENABLED__
		return OCSNMP_GetMibVar(p_envVar, pMagicMarkup, pArgs, pBuf, NULL);
#else
        return OK;
#endif
#endif
    }

    /* Does the callee have permission to the data? */
    if (! ((p_dbeCell->Permissions & kReadAccess) &&
          RC_ACCESS_Allowed(p_dbeCell->UserLevelRead, p_envVar->UserLevel)))
    {
        return OK;
    }

#ifndef __DISABLE_STRUCTURES__

    /* Structure objects are processed with highest precedence */
    if (NULL != p_dbeCell->p_sdrObject)
    {
        if (p_dbeCell->p_sdrObject->CacheAccess & kCacheRead)
        {
            sbyte *pObject = (sbyte *)Cache_FindObject(p_envVar,
                p_envVar->phCacheHandle,
                p_dbeCell->p_sdrObject->StructureIdentifer, p_dbeCell, FALSE);

            if (NULL == pObject)
                return ERROR_GENERAL_NOT_FOUND;

            pObject += p_dbeCell->OffsetIntoStruct;

            return DB_StructureRead(p_envVar, pObject, pBuf, p_dbeCell, pArgs);
        }

        if (NULL != p_dbeCell->p_sdrObject->p_funcStrucLoc)
        {
            sbyte *pObject = p_dbeCell->p_sdrObject->p_funcStrucLoc(p_envVar, p_dbeCell->p_sdrObject->StructureIdentifer);
            pObject += p_dbeCell->OffsetIntoStruct;

            return DB_StructureRead(p_envVar, pObject, pBuf, p_dbeCell, pArgs);
        }
    }

#endif /* __DISABLE_STRUCTURES__ */

    /* Next, objects with only read primitives are processed */
    if (NULL != p_dbeCell->p_funcRdPrim)
    {
        p_dbeCell->p_funcRdPrim(p_envVar, pBuf, p_dbeCell->pDataObject, pArgs);
        return OK;
    }

    /* Lastly, direct reads are attempted */
    if (NULL != p_dbeCell->pDataObject)
        return CONVERT_ToStr(p_dbeCell->pDataObject, pBuf, p_dbeCell->iDataType);

    return OK;

}   /* DB_ReadData */



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

extern RLSTATUS
DB_ReadIndexedData(environment *p_envVar, sbyte *pMagicMarkup, sbyte *pBuf, sbyte *pArgs)
{
    DataBaseEntry *p_dbeCell = p_dbeFindCell(pMagicMarkup);

    if (NULL == p_dbeCell)
    {
#ifdef  __ENABLE_EXTERNAL_API__
        return CUSTOM_EXTERNAL_HANDLER(kDBReadData, p_envVar, pMagicMarkup, pArgs, NULL, pBuf, NULL);
#else
        return ERROR_GENERAL_NOT_FOUND;
#endif
    }

    if (! ((p_dbeCell->Permissions & kReadAccess) &&
          RC_ACCESS_Allowed(p_dbeCell->UserLevelRead, p_envVar->UserLevel)))
    {
        *pBuf = '\0';

        return OK;
    }

    /* Structure objects are processed with highest precedence */
    if (NULL != p_dbeCell->p_sdrObject)
    {
        if (p_dbeCell->p_sdrObject->CacheAccess & kCacheRead)
        {
            sbyte *pObject = (sbyte *)Cache_FindObject(p_envVar, p_envVar->phCacheHandle,
                p_dbeCell->p_sdrObject->StructureIdentifer, p_dbeCell, TRUE);

            if (NULL == pObject)
                return ERROR_GENERAL_NOT_FOUND;

            pObject += p_dbeCell->OffsetIntoStruct;

            return DB_StructureRead(p_envVar, pObject, pBuf, p_dbeCell, pArgs);
        }

        if (NULL != p_dbeCell->p_sdrObject->p_funcStrucLoc)
        {
            sbyte *pObject = DB_Private_ObjectIndexedAddr(p_envVar, p_dbeCell);

            if (NULL == pObject)
                return ERROR_GENERAL_NOT_FOUND;

            pObject += p_dbeCell->OffsetIntoStruct;

            return DB_StructureRead(p_envVar, pObject, pBuf, p_dbeCell, pArgs);
        }
    }

    return ERROR_GENERAL_NOT_FOUND;

}   /* DB_ReadIndexedData */

#endif /* __DISABLE_STRUCTURES__ */



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

extern void *DB_GetSDR(sbyte *pMagicMarkup)
{
    DataBaseEntry *p_dbeCell = p_dbeFindCell(pMagicMarkup);

    if (NULL == p_dbeCell)
        return NULL;

    return p_dbeCell->p_sdrObject;
}

#endif /* __DISABLE_STRUCTURES__ */



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

extern Length DB_Private_SizeOfStruct(DataBaseEntry *p_dbeCell)
{
    if ((NULL == p_dbeCell) || (NULL == p_dbeCell->p_sdrObject))
        return 0;

    return (p_dbeCell->p_sdrObject->SizeOfStruct);
}

#endif /* __DISABLE_STRUCTURES__ */



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

static Boolean DB_FindIndexedElement(void *p_sdrObject, void *p_idxObject)
{
    if (p_sdrObject == (((IndexedElement *)p_idxObject)->p_sdrObject))
        return TRUE;

    return FALSE;
}

#endif /* __DISABLE_STRUCTURES__ */



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

extern void *
DB_Private_ObjectIndexedAddr(environment *p_envVar, DataBaseEntry *p_dbeCell)
{
    struct IndexedElement   *pObject;
    StrcDes                 *p_sdrObj;

    if ((NULL == p_dbeCell) || (NULL == p_dbeCell->p_sdrObject) ||
        (NULL == p_dbeCell->p_sdrObject->p_funcStrucLoc))
        return NULL;

    p_sdrObj = p_dbeCell->p_sdrObject;

#ifdef __MACRO_REPEAT_NEST__
	/* If new nested repeat structure is active, look up values via rcw_macro.c */
	if (p_envVar->Macro_Counter >= 0)
	{
		return(DB_Private_ObjectIndexedAddrEx(p_envVar,p_dbeCell));
	}
#endif

    if ((NULL == (p_sdrObj->p_funcIndexNext   )) ||
        (NULL == (p_envVar->p_lstIndexedValues))  )
        return (p_sdrObj->p_funcStrucLoc(p_envVar, p_sdrObj->StructureIdentifer));

    if (NULL == (pObject = pList_FindObject(p_envVar->p_lstIndexedValues, p_sdrObj, DB_FindIndexedElement)))
        return (p_sdrObj->p_funcStrucLoc(p_envVar, p_sdrObj->StructureIdentifer));
    return pObject->pDynamicLocation;
}

#ifdef __MACRO_REPEAT_NEST__
extern void *
DB_Private_ObjectIndexedAddrEx(environment *p_envVar, DataBaseEntry *p_dbeCell)
{
    struct IndexedElement   *pObject;
    StrcDes                 *p_sdrObj;

    if ((NULL == p_dbeCell) || (NULL == p_dbeCell->p_sdrObject) ||
        (NULL == p_dbeCell->p_sdrObject->p_funcStrucLoc))
        return NULL;

    p_sdrObj = p_dbeCell->p_sdrObject;

	/* See if Object is does not have a NEXT function */
    if (NULL == (p_sdrObj->p_funcIndexNext))
        return (p_sdrObj->p_funcStrucLoc(p_envVar, p_sdrObj->StructureIdentifer));

	/* See if Object is currently in the REPEAT macro list (note:DB_FindIndexedElement defines the use of the 'p_sdrObj' pointer) */
    if (NULL == (pObject = pMACRO_ListFindObject(p_envVar, (void *) p_sdrObj, DB_FindIndexedElement)))
        return (p_sdrObj->p_funcStrucLoc(p_envVar, p_sdrObj->StructureIdentifer));

    return pObject->pDynamicLocation;
}
#endif	/* __MACRO_REPEAT_NEST__ */

#endif /* __DISABLE_STRUCTURES__ */



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

extern StrcDes *
DB_CreateStrucDesc(UniqId Id, Length LenStruct, Access CachePermissions,
                   void *(*p_funcObjLocation)(environment *, UniqId),
                   void *(*p_funcIndexNext)(environment *, void *, int),
                   void  (*p_funcObjWrite)(environment *, UniqId, void *),
                   void  (*p_funcObjLockIt)(UniqId),
                   void  (*p_funcObjUnlock)(UniqId) )
{
    StrcDes *p_sdrTemp;

    if (NULL != (p_sdrTemp = RC_MALLOC(sizeof(StrcDes))))
    {
        p_sdrTemp->StructureIdentifer   = Id;
        p_sdrTemp->SizeOfStruct         = LenStruct;
        p_sdrTemp->CacheAccess          = CachePermissions;
        p_sdrTemp->p_funcStrucLoc       = p_funcObjLocation;
        p_sdrTemp->p_funcIndexNext      = p_funcIndexNext;
        p_sdrTemp->p_funcWrStruct       = p_funcObjWrite;

        p_sdrTemp->p_funcObjLockIt      = p_funcObjLockIt;
        p_sdrTemp->p_funcObjUnlock      = p_funcObjUnlock;
    }

    return p_sdrTemp;
}

#endif /* __DISABLE_STRUCTURES__ */



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

extern void *DB_Private_GetSDR(DataBaseEntry *p_dbeCell)
{
    return ((void *)(p_dbeCell->p_sdrObject));
}

#endif /* __DISABLE_STRUCTURES__ */



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

extern void DB_SDR_Private_LockIt(void *p_sdrStructDesc)
{
    if ((NULL != p_sdrStructDesc) && (NULL != ((StrcDes *)p_sdrStructDesc)->p_funcObjLockIt))
        ((StrcDes *)p_sdrStructDesc)->p_funcObjLockIt(((StrcDes *)p_sdrStructDesc)->StructureIdentifer);
}

#endif /* __DISABLE_STRUCTURES__ */



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

extern void DB_SDR_Private_UnLock(void *p_sdrStructDesc)
{
    if ((NULL != p_sdrStructDesc) && (NULL != ((StrcDes *)p_sdrStructDesc)->p_funcObjUnlock))
        ((StrcDes *)p_sdrStructDesc)->p_funcObjUnlock(((StrcDes *)p_sdrStructDesc)->StructureIdentifer);
}

#endif /* __DISABLE_STRUCTURES__ */



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

extern void *DB_SDR_Private_ObjectLoc(environment *p_envVar, void *p_sdrStructDesc)
{
    if ((NULL != p_sdrStructDesc) && (NULL != ((StrcDes *)(p_sdrStructDesc))->p_funcStrucLoc))
        return ((StrcDes *)(p_sdrStructDesc))->p_funcStrucLoc(p_envVar, ((StrcDes *)p_sdrStructDesc)->StructureIdentifer);

    return NULL;
}

#endif /* __DISABLE_STRUCTURES__ */



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

extern RLSTATUS DB_SDR_Private_WrStruct(struct environment *p_envVar,
                                        void *p_sdrStructDesc, void *pCachedObject)
{
    if ((NULL != p_sdrStructDesc) && (NULL != ((StrcDes *)(p_sdrStructDesc))->p_funcWrStruct))
    {
        ((StrcDes *)(p_sdrStructDesc))->p_funcWrStruct(p_envVar, ((StrcDes *)p_sdrStructDesc)->StructureIdentifer, pCachedObject);

        return OK;
    }

    return ERROR_GENERAL_NOT_FOUND;
}

#endif /* __DISABLE_STRUCTURES__ */



/*-----------------------------------------------------------------------*/

#ifndef __DISABLE_STRUCTURES__

extern Length DB_SDR_Private_SizeOf(void *p_sdrStructDesc)
{
    if (NULL != p_sdrStructDesc)
        return ((StrcDes *)(p_sdrStructDesc))->SizeOfStruct;

    return 0;
}

#endif /* __DISABLE_STRUCTURES__ */


/*-----------------------------------------------------------------------*/

#if defined(__JAVACONTROL_ENABLED__) || defined(__OCP_ENABLED__)

extern RLSTATUS DB_GetDataType(sbyte *pMarkupName, DataType *pDataType)
{
#ifdef __SNMP_API_ENABLED__
	int	MibObjType; 
#endif
    RLSTATUS status = OK;
    DataBaseEntry *p_dbeCell = p_dbeFindCell(pMarkupName);

    if (NULL != p_dbeCell)
    {
        *pDataType = p_dbeCell->iDataType;
    }
    else
    {
#ifdef __SNMP_API_ENABLED__

	MibObjType = OCSNMP_MibObjectType(pMarkupName);
#ifdef OCP_DEBUG
	printf("OCSNMP_MibObjectType returns %d\n", MibObjType);
#endif
	if (0 > MibObjType)
	{
        status = ERROR_GENERAL_NOT_FOUND;
	}
	else
	{
		/* !!!!! in JC 2.2 tool treats all SNMP vars as strings; change next rev !!!!! */
		/* 1/13/98: restored functionality under ifdef, since broker can handle this, */
		/* but right now the tool can't. So only turn this on if using API, not beans. */
#ifdef	__OCP_SNMP_TYPES_ENABLED__
		*pDataType = MibObjType; 
#else
		*pDataType = kDTstring; 
#endif
	}

#else	
        status = ERROR_GENERAL_NOT_FOUND;
#endif
    }

    return status;
}

#endif /* __JAVACONTROL_ENABLED__ ||__OCP_ENABLED__ */

/*-----------------------------------------------------------------------*/

#if !defined(__DATABASE_USE_ARRAY__) && (defined(__JAVACONTROL_ENABLED__) || defined(__OCP_ENABLED__))
extern void *DB_GetDataBase()
{
    return pmDataBase;
}
#endif /* !defined(__DATABASE_USE_ARRAY__) && defined(__JAVACONTROL_ENABLED__) */

