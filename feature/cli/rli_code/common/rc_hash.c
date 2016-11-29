/*  
 *  rc_hash.c
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



#include "rc_options.h"
#include "rc_errors.h"
#include "rc_rlstddef.h"
#include "rc_rlstdlib.h"
#include "rc_hash.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"


/*-----------------------------------------------------------------------*/

#ifdef __NOT_USED__
static void 
HASH_TraverseChain ( HashNode *pChainStart, 
                     void (*funcProcessObject)(void *pObject) )
{
    HashNode *pEntry;

    pEntry = pChainStart;
    while ( pEntry != NULL )
    {
        funcProcessObject( pEntry->pObject );
        pEntry = pEntry->pNextNode;
    }
}
#endif



/*-----------------------------------------------------------------------*/

#ifdef __NOT_USED__
static void 
HASH_CountEntriesInChain ( HashNode *pChainStart, 
                           Counter *pChainEntries )
{
    Counter     cNumEntries;
    HashNode    *pEntry;

    pEntry      = pChainStart;
    cNumEntries = 0;    

    while ( pEntry != NULL )
    {
        pEntry = pEntry->pNextNode;
        cNumEntries++;
    }

    *pChainEntries = cNumEntries;

    return;
}
#endif



/*-----------------------------------------------------------------------*/

static void
HASH_HashValue ( ubyte4 key, Counter *pHashValue )
{
    *pHashValue  = key % kHashTableSize;
}



/*-----------------------------------------------------------------------*/

static void
HASH_DestroyChain ( HashNode *pChainStart, 
                    void (*funcDelete_pObject)(void *pObject) )
{
    HashNode *pEntry, *pLamb;

    pEntry = pChainStart;

    while ( pEntry != NULL )
    {
        funcDelete_pObject( pEntry->pObject );
        pLamb   = pEntry;
        pEntry  = pEntry->pNextNode;
        RC_FREE( pLamb );
    }

    return;

}



/*-----------------------------------------------------------------------*/

extern void *
HASH_Construct( void  )
{
    HashNode **ppHashTable;
    Counter  cHTIndex;
    
    ppHashTable = (HashNode**)RC_MALLOC( sizeof(HashNode*) * kHashTableSize );

    if ( NULL != ppHashTable )
    {
        cHTIndex = 0;
        while ( cHTIndex < kHashTableSize )
        {
            ppHashTable[cHTIndex] = NULL;
            cHTIndex++;
        }
    }

    return ppHashTable;
}



/*-----------------------------------------------------------------------*/

extern void 
HASH_Destruct( void **ppHashTable, void (*funcDelete_pObject)(void *pObject) )
{
    HashNode    **ppTable, *pEntry;
    Counter     cHTIndex;

    if ((NULL == ppHashTable        )   ||
        (NULL == *ppHashTable       )   ||
        (NULL == funcDelete_pObject ))
    {
        return;
    }

    ppTable  = (HashNode**)(*ppHashTable);
    cHTIndex = 0;

    while ( cHTIndex < kHashTableSize )
    {
        pEntry = ppTable[cHTIndex]; 

        if (NULL != pEntry)
            HASH_DestroyChain( pEntry, funcDelete_pObject );

        cHTIndex++;
    }

    RC_FREE( ppTable );
    *ppHashTable = NULL;

    return;

} /* HASH_Destruct */



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
HASH_AddObject ( void   *pHashTable, 
                 void   *pObject, 
                 void   (*funcExtractKey)( void *pData, ubyte4 *pKey ),
                 RLSTATUS (*funcCompare)(void *pObject1, void *pObject2) )
{
    HashNode    **ppTable, *pEntry, *pPrevEntry, *pNewEntry;
    Counter     cHTIndex;
    ubyte4      key;

    if ((NULL == pHashTable     )   ||
        (NULL == pObject        )   ||
        (NULL == funcExtractKey )   ||
        (NULL == funcCompare    ))
    {
        return ERROR_GENERAL_NOT_FOUND;
    }

    /* Create and Initialize a new entry */
    pNewEntry = (HashNode*)RC_MALLOC( sizeof( HashNode ) );

    if (NULL == pNewEntry)
        return ERROR_MEMMGR_NO_MEMORY;
        
    pNewEntry->pObject   = pObject;
    pNewEntry->pNextNode = NULL;    

    /* Find the relevant bucket entry */
    funcExtractKey( pObject, &key );
    HASH_HashValue( key, &cHTIndex );
    ppTable = (HashNode**)pHashTable;
    
    /* Create  a new chain, or add the entry onto the end of an existing one */
    if (NULL == ppTable[cHTIndex])
        ppTable[cHTIndex] = pNewEntry;
    else
    {
        pEntry = ppTable[cHTIndex];
        pPrevEntry = pEntry;
        
        /* Check to make sure there aren't any duplicates */
        while (NULL != pEntry)
        {
            if (OK == funcCompare( pEntry->pObject, pNewEntry->pObject ))
            {
                RC_FREE( pNewEntry );
                return ERROR_GENERAL_ACCESS_DENIED;
            }
            
            pPrevEntry = pEntry;
            pEntry     = pEntry->pNextNode;
        }

        pPrevEntry->pNextNode = pNewEntry;
    }

    return OK;

} /* sHASH_AddObject */



/*-----------------------------------------------------------------------*/

extern void *
HASH_FindObject (   void    *pHashTable, 
                    void    *pData, 
                    void    (*funcExtractKey)( void *pData, ubyte4 *pKey ),
                    RLSTATUS  (*funcCompare)(void *pObject1,void *pObject2) )
{
    HashNode **ppTable, *pEntry;
    Counter  cHTIndex;
    ubyte4   key;

    if ((NULL == pHashTable     ) ||
        (NULL == funcExtractKey ) ||
        (NULL == funcCompare    )   )
    {
        return NULL;
    }

    ppTable = (HashNode**)pHashTable;
    funcExtractKey( pData, &key );
    HASH_HashValue( key, &cHTIndex );

    pEntry = ppTable[cHTIndex];

    while ( NULL != pEntry )
    {
        if ( OK == funcCompare( pEntry->pObject, pData ) )
            return pEntry->pObject;

        pEntry = pEntry->pNextNode;
    }

    return NULL;

} /* HASH_FindObject */

