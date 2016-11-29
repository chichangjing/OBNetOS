/*  
 *  rc_cache.c
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
$History: rc_cache.c $
 * 
 * *****************  Version 8  *****************
 * User: Epeterson    Date: 4/26/00    Time: 2:06p
 * Updated in $/Rapid Logic/Code Line/rli_code/ocb
 * Enabled VSS auto-archive feature using keyword expansion
 * $History:  $



*/

#include "rc_options.h"
#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"
#include "rc_database.h"
#include "rc_cache.h"

#ifndef __DISABLE_STRUCTURES__ 



/*-----------------------------------------------------------------------*/

extern RLSTATUS Cache_Construct(CacheHandle **pph_cacHandle, Access AccessType)
{
    
    if (NULL == pph_cacHandle)
        return ERROR_GENERAL_ACCESS_DENIED;

    if (NULL == (*pph_cacHandle = (CacheHandle *) RC_MALLOC (sizeof(CacheHandle))))
        return ERROR_MEMMGR_NO_MEMORY;

    if (NULL == ((*pph_cacHandle)->p_lstCacheObjects = List_Construct()))
    {
        RC_FREE(*pph_cacHandle);
        *pph_cacHandle = NULL;

        return ERROR_MEMMGR_NO_MEMORY;
    }

    (*pph_cacHandle)->AccessType = AccessType;

    return OK;
}



/*-----------------------------------------------------------------------*/

static void CacheObject_Destruct(void *p_cacObject)
{
    if (NULL != p_cacObject)
    {
        if (NULL != ((CacheObject *)(p_cacObject))->pCachedItem)
        {
            RC_FREE(((CacheObject *)(p_cacObject))->pCachedItem);
        }

        RC_FREE(p_cacObject);
    }
}



/*-----------------------------------------------------------------------*/

static void CacheObject_WriteBackDestruct(void *pCacheObject, void *p_envVar)
{
    CacheObject     *p_cacObject = pCacheObject;

    /* remove cache objects and the cached data it points to */
    if (NULL == p_cacObject)
        return;

    if (NULL != p_cacObject->pCachedItem)
    {
        /* make sure the object is dirty, and there is a reverse link */
        if ((TRUE == p_cacObject->Dirty) && (NULL != p_cacObject->p_sdrReverseLink))
        {
            /* lock the indigenous memory down before copying */
            DB_SDR_Private_LockIt(p_cacObject->p_sdrReverseLink);

            if (OK != DB_SDR_Private_WrStruct(p_envVar, p_cacObject->p_sdrReverseLink, p_cacObject->pCachedItem) )
            {
                void    *Dest, *Src;
                Counter NumBytes;

                /* write back the cached information back to the indigenous system memory */
                Dest     = DB_SDR_Private_ObjectLoc(p_envVar, p_cacObject->p_sdrReverseLink);
                Src      = p_cacObject->pCachedItem;
                NumBytes = DB_SDR_Private_SizeOf(p_cacObject->p_sdrReverseLink);

                if ((NULL != Dest) && (NULL != Src))
                    MEMCPY(Dest, Src, NumBytes);
            }

            /* unlock the indigenous memory */
            DB_SDR_Private_UnLock(p_cacObject->p_sdrReverseLink);
        }

        RC_FREE(p_cacObject->pCachedItem);
    }

    RC_FREE(p_cacObject);

} /* CacheObject_WriteBackDestruct */



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
Cache_Destruct(environment *p_envVar, CacheHandle **pph_cacHandle)
{
    if ( (NULL == pph_cacHandle) || (NULL == *pph_cacHandle) )
        return ERROR_GENERAL_NOT_FOUND;

    /*!!!!!!!!!!!!!  check PostValid here!  JB*/

    if (0 != ( ((*pph_cacHandle)->AccessType) & kCacheWrite) )
        List_Destruct_Param(&((*pph_cacHandle)->p_lstCacheObjects), ((void (*) (void *, void *)) CacheObject_WriteBackDestruct), p_envVar );
    else
        List_Destruct(&((*pph_cacHandle)->p_lstCacheObjects), ((void (*) (void *)) CacheObject_Destruct) );

    RC_FREE(*pph_cacHandle);

    *pph_cacHandle = NULL;
    return OK;
}



/*-----------------------------------------------------------------------*/

static Boolean Cache_Compare(void *DatabaseId, void *CacheObjectId)
{
    if ((NULL != DatabaseId) && (NULL != CacheObjectId))
    {
        UniqId uId1 = *((UniqId *)DatabaseId);
        UniqId uId2 = ((CacheObject *)CacheObjectId)->Id;

        if (uId1 == uId2)
            return TRUE;    
    }

    return FALSE;
}



/*-----------------------------------------------------------------------*/

extern void *
Cache_FindObject(environment *p_envVar, CacheHandle *ph_cacHand, UniqId Id, 
                 DataBaseEntry *p_dbeCell, Boolean TempObject)
{
    void *pObject;
	void *pData;

    if (NULL == ph_cacHand)         
        return NULL;

    pObject = pList_FindObject(ph_cacHand->p_lstCacheObjects, &Id, 
        ((Boolean (*) (void *, void *)) Cache_Compare) );

    if (NULL == pObject)        
    {
        CacheObject *p_ccoTemp;

        if (0 == DB_Private_SizeOfStruct(p_dbeCell))
            return NULL;

        if (NULL == (pObject = RC_MALLOC(DB_Private_SizeOfStruct(p_dbeCell))))
            return NULL;

        if (NULL == (p_ccoTemp = RC_MALLOC(sizeof(CacheObject))))
        {
            RC_FREE(pObject);

            return NULL;
        }

        p_ccoTemp->Id           = Id;
        p_ccoTemp->Dirty        = FALSE;        
        p_ccoTemp->TempObject   = TempObject;   
        p_ccoTemp->pCachedItem  = pObject;

        /* attach CacheObject to SDR */
        p_ccoTemp->p_sdrReverseLink = DB_Private_GetSDR(p_dbeCell);

        if (OK != List_AddObject(ph_cacHand->p_lstCacheObjects, p_ccoTemp))
        {
            RC_FREE(pObject);          
            RC_FREE(p_ccoTemp);    

            return NULL;
        }

        DB_SDR_Private_LockIt(p_ccoTemp->p_sdrReverseLink);
        if (NULL != (pData = DB_Private_ObjectIndexedAddr(p_envVar, p_dbeCell)))
            MEMCPY(pObject, pData, DB_Private_SizeOfStruct(p_dbeCell));
        else
            MEMSET(pObject, 0, DB_Private_SizeOfStruct(p_dbeCell));
        DB_SDR_Private_UnLock(p_ccoTemp->p_sdrReverseLink);
    }
    else
        pObject = (void *)(((CacheObject *)pObject)->pCachedItem);

    return pObject;

}  /* Cache_FindObject */



/*-----------------------------------------------------------------------*/

extern void Cache_ObjectDirty(CacheHandle *ph_cacHand, UniqId Id)
{
    CacheObject *p_ccoObject;

    if (NULL == ph_cacHand)         
        return;

    p_ccoObject = (CacheObject *)pList_FindObject(ph_cacHand->p_lstCacheObjects, &Id, 
        ((Boolean (*) (void *, void *)) Cache_Compare) );

    if (NULL == p_ccoObject)
        return;

    p_ccoObject->Dirty = TRUE;      
}



/*-----------------------------------------------------------------------*/

static Boolean Cache_TempObjectCompare(void *p_ccoObject)
{
    Boolean RetVal = ((CacheObject *)p_ccoObject)->TempObject;

    if (TRUE == RetVal)
        RC_FREE(((CacheObject *)p_ccoObject)->pCachedItem);

    return RetVal;
}



/*-----------------------------------------------------------------------*/

extern void Cache_FlushTempObjects(CacheHandle *ph_cacHand)
{
    if (NULL == ph_cacHand)     
        return;

    while (OK == (List_RemoveObject(ph_cacHand->p_lstCacheObjects, 
        ((Boolean (*) (void *)) Cache_TempObjectCompare) )))
        ;
}

#endif /* __DISABLE_STRUCTURES__ */
