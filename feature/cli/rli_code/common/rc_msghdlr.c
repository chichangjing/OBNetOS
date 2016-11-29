/*  
 *  rc_msghdlr.c
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
#include "rc_memmgr.h"
#include "rc_msghdlr.h"
#include "rc_hash.h"
#include "rc_linklist.h"
#include "rc_os_spec.h"

/*-------------------------------------------------------------------------------*/

typedef struct MsgObj
{
    RLSTATUS    msgno;
    sbyte*      pMsg;

} MsgObj;


/*-------------------------------------------------------------------------------*/

typedef struct MsgDictionary
{
    void        *pMsgTable;
    sbyte       *pDefaultMsg;

} MsgDictionary;


/*-------------------------------------------------------------------------------*/

/* for backwards compatibility */
static  sbyte2   mInternalMsgDictionaryID;
static  sbyte2   mCustomerMsgDictionaryID;

/* New set of message dictionaries */
static  MsgDictionary   mMsgDictionaries[ kNumMessageDictionaries ];



/*-------------------------------------------------------------------------------*/

static RLSTATUS
MsgHdlr_Compare ( void *pObject1, void *pObject2 )
{
    MsgObj  *pMsgObj1, *pMsgObj2;
    
    if ( ( NULL == pObject1 ) || ( NULL == pObject2 ) )
        return ERROR_GENERAL;

    pMsgObj1 = (MsgObj*)pObject1;
    pMsgObj2 = (MsgObj*)pObject2;

    return ( pMsgObj1->msgno == pMsgObj2->msgno ) ? OK : ERROR_GENERAL_NOT_EQUAL;
}



/*-------------------------------------------------------------------------------*/

static void
MsgHdlr_ExtractKey ( void *pObject, ubyte4 *pKey )
{
    MsgObj  *pMsgObj;
    
    if ( NULL == pObject )
    {
        *pKey = 0;
        return;
    }
    
    pMsgObj = (MsgObj*) pObject;
    *pKey   = (ubyte4)( pMsgObj->msgno );

    return;
}



/*-------------------------------------------------------------------------------*/

static RLSTATUS
MsgHdlr_RetrieveDictionary( sbyte2 msgDictionaryID, MsgDictionary **ppMsgDictionary )
{
    *ppMsgDictionary = &mMsgDictionaries[ msgDictionaryID ];
    return OK;
}



/*-------------------------------------------------------------------------------*/

static RLSTATUS
MsgHdlr_CheckDictionaryID( sbyte2 msgDictionaryID )
{
    if (   ( msgDictionaryID < 0 ) 
        || ( kNumMessageDictionaries <= msgDictionaryID ) )
        return ERROR_GENERAL_ILLEGAL_VALUE;
    
    return OK;
}



/*-------------------------------------------------------------------------------*/

extern RLSTATUS
MsgHdlr_RetrieveMessage ( RLSTATUS msg, sbyte2 msgDictionaryID, sbyte **ppMsg )
{
    MsgObj          dummyMsgObj, *pRealMsgObj;
    MsgDictionary  *pMsgDictionary;
    RLSTATUS       status;
    
    status = MsgHdlr_CheckDictionaryID( msgDictionaryID );
    if ( OK > status )
        return  status;
    
    /* Line up which set of tables and handlers to use */
    status = MsgHdlr_RetrieveDictionary( msgDictionaryID, &pMsgDictionary );
    if ( OK > status )
        return ERROR_GENERAL_NOT_FOUND;
    
    /* Find and process the message */
    dummyMsgObj.pMsg  = NULL;
    dummyMsgObj.msgno = msg;
    
    pRealMsgObj = HASH_FindObject( pMsgDictionary->pMsgTable, &dummyMsgObj, 
                                   MsgHdlr_ExtractKey, MsgHdlr_Compare );
    if ( NULL == pRealMsgObj )
        *ppMsg = pMsgDictionary->pDefaultMsg;   
    else
        *ppMsg = pRealMsgObj->pMsg;

    return OK;
}



/*-------------------------------------------------------------------------------*/

extern RLSTATUS
MsgHdlr_RetrieveOpenControlMessage ( RLSTATUS msg, sbyte **ppMsg )
{
    if (NULL == ppMsg)
        return ERROR_GENERAL_NOT_FOUND;

    *ppMsg = NULL;

    return MsgHdlr_RetrieveMessage( msg, mInternalMsgDictionaryID, ppMsg );
}



/*-------------------------------------------------------------------------------*/

extern RLSTATUS
MsgHdlr_RetrieveCustomMessage ( RLSTATUS msg, sbyte **ppMsg )
{
    return MsgHdlr_RetrieveMessage( msg, mCustomerMsgDictionaryID, ppMsg );
}



/*-------------------------------------------------------------------------------*/

static Boolean
IsEndOfMessageData ( sbyte *pMessageData )
{
    if (    ( 'x' == pMessageData[0] ) && ( 'x' == pMessageData[1] )
        &&  ( 'x' == pMessageData[2] ) && ( 'x' == pMessageData[3] ) )
        return TRUE;
    else
        return FALSE;
}



/*-------------------------------------------------------------------------------*/

static Boolean
IsDefaultMsg ( sbyte *pStr )
{
    if ( !STRICMP( pStr, "default" ) )
        return TRUE;
    else
        return FALSE;
}



/*-------------------------------------------------------------------------------*/

static RLSTATUS
MsgHdlr_InitDefaultMsg ( sbyte *pStr, MsgDictionary *pMsgDictionary )
{
    sbyte  *pMsg;
    Length msgLen;

    msgLen = STRLEN( pStr );
    pMsg   = (sbyte*)RC_MALLOC( msgLen + 1 );
    if ( NULL == pMsg )
        return ERROR_MEMMGR_NO_MEMORY;

    STRCPY( pMsg, pStr );
    pMsgDictionary->pDefaultMsg = pMsg;

    return OK;
}



/*-------------------------------------------------------------------------------*/

static RLSTATUS
MsgHdlr_AddEntry ( RLSTATUS msgno, sbyte *pMsg, void *pMsgTable )
{
    sbyte  *pCopyMsg;
    Length msgLen;
    MsgObj *pMsgObj;
    RLSTATUS status;

    msgLen      = STRLEN( pMsg );
    pCopyMsg    = (sbyte*)RC_MALLOC( msgLen + 1 );
    if ( NULL == pCopyMsg )
        return ERROR_MEMMGR_NO_MEMORY;

    STRCPY( pCopyMsg, pMsg );

    pMsgObj = (MsgObj*)RC_MALLOC( sizeof(MsgObj) );
    if ( NULL == pMsgObj )
    {
        RC_FREE( pCopyMsg );
        return ERROR_MEMMGR_NO_MEMORY;
    }
    
    pMsgObj->pMsg  = pCopyMsg;
    pMsgObj->msgno = msgno;

    status = HASH_AddObject( pMsgTable, pMsgObj, 
                                MsgHdlr_ExtractKey, MsgHdlr_Compare );
    return status;

}   /* MsgHdlr_AddEntry */



/*-------------------------------------------------------------------------------*/

static RLSTATUS
MsgHdlr_BuildupTable ( sbyte *pMessageData, MsgDictionary *pMsgDictionary )
{
    sbyte     *pStr, *pMessageNumberString, *pMsg, *pDefaultMsg, *pTempStrtok;
    RLSTATUS  msgno, status;
    Boolean   isDefaultInitted;
    void      *pMsgTable;
    
    isDefaultInitted = FALSE;
    pMsgTable = pMsgDictionary->pMsgTable;
    
    pStr = STRTOK_REENTRANT( pMessageData, kMessageDataStartDelimiter, &pTempStrtok);
    while (TRUE)
    {
        /* Extract the first part of the mapping, and perform
         * some checks on the data                               */
        pStr = STRTOK_REENTRANT( NULL, kMessageNumberDelimiter, &pTempStrtok);
        if ( NULL == pStr )
            return ERROR_GENERAL_NO_DATA;
        
        while ( ISWSPACE( *pStr ) )
            pStr++;

        if ( IsEndOfMessageData( pStr ) )
            break;
    
        if ( IsDefaultMsg( pStr ) ) 
        {
            pDefaultMsg = STRTOK_REENTRANT( NULL, kMessageDelimiter, &pTempStrtok);
            if ( NULL == pDefaultMsg )
                return ERROR_GENERAL_NO_DATA;

            status = MsgHdlr_InitDefaultMsg( pDefaultMsg, pMsgDictionary  );
            if ( 0 > status )
                return status;

            isDefaultInitted = TRUE;
        }
        else
        {
            /* Assume the string represents a message number */
            pMessageNumberString = pStr;
            msgno = ATOI( pMessageNumberString );
            if ( 0 == msgno )
                return ERROR_GENERAL_NO_DATA;

            /* Extract the Message */
            pMsg = STRTOK_REENTRANT( NULL, kMessageDelimiter, &pTempStrtok);
            if ( NULL == pMsg )
                return ERROR_GENERAL_NO_DATA;

            status = MsgHdlr_AddEntry( msgno, pMsg, pMsgTable );
            if ( 0 > status )
                return status;
        }
    }

    if ( isDefaultInitted )
        return OK;
    else
        return ERROR_GENERAL_NO_DATA;

}   /* MsgHdlr_BuildupTable */
        


/*-------------------------------------------------------------------------------*/

static void 
MsgHdlr_DestroyMsgObj ( void *pObject )
{
    MsgObj  *pMsgObj;

    pMsgObj = (MsgObj*)pObject;
    RC_FREE( pMsgObj->pMsg );
    RC_FREE( pMsgObj       );
    return;
}


/*-------------------------------------------------------------------------------*/

static RLSTATUS 
MsgHdlr_RetrieveFirstAvailableDictionaryID( sbyte2 *pMsgDictionaryID )
{
    sbyte2 msgDictionaryID;
    
    for (   msgDictionaryID = 0;  
            msgDictionaryID < kNumMessageDictionaries; 
            msgDictionaryID++ )
    {
        if (   ( NULL == mMsgDictionaries[ msgDictionaryID ].pMsgTable   )
            && ( NULL == mMsgDictionaries[ msgDictionaryID ].pDefaultMsg ) )
        {
            *pMsgDictionaryID = msgDictionaryID;        
            return OK;
        }
    }

    return ERROR_GENERAL_NOT_FOUND;
}



/*------------------------------------------------------------------------------*/

extern RLSTATUS
MsgHdlr_InitDictionary (    sbyte   *pMessageData, 
                            sbyte4  strlenSizeOfMsgData,
                            sbyte2  *pMsgDictionaryID       )
{
    MsgDictionary  *pMsgDictionary;
    sbyte2         msgDictionaryID;
    RLSTATUS       status;
    
    if ( pMessageData  == NULL )
        return ERROR_GENERAL;

    *pMsgDictionaryID = kBadMsgDictionaryID;
    msgDictionaryID   = kBadMsgDictionaryID;
    status = MsgHdlr_RetrieveFirstAvailableDictionaryID( &msgDictionaryID );
    if ( 0 > status )
        return status;
    
    status = MsgHdlr_RetrieveDictionary( msgDictionaryID, &pMsgDictionary );
    if ( OK > status )
        return ERROR_GENERAL_NOT_FOUND;

    pMsgDictionary->pMsgTable = HASH_Construct();
    if ( NULL == pMsgDictionary->pMsgTable )
        return ERROR_GENERAL;

    if( 0 > MsgHdlr_BuildupTable( pMessageData, pMsgDictionary ) )
    {
        HASH_Destruct( &(pMsgDictionary->pMsgTable), MsgHdlr_DestroyMsgObj );
        return ERROR_GENERAL;
    }

    *pMsgDictionaryID = msgDictionaryID;
    return OK;  

}   /* MsgHdlr_InitDictionary */


/*------------------------------------------------------------------------------*/

extern RLSTATUS
MsgHdlr_Init (  sbyte   *pMessageData, 
                sbyte4  strlenSizeOfMsgData,
                sbyte   msgType                 )
{
    RLSTATUS status;
    sbyte2   msgDictionaryID;
    
    msgDictionaryID = kBadMsgDictionaryID;
    status = MsgHdlr_InitDictionary( pMessageData, strlenSizeOfMsgData, &msgDictionaryID );
    if ( OK > status )
        return status;
    
    if ( kInternalMessage == msgType )
        mInternalMsgDictionaryID = msgDictionaryID;
    else
        mCustomerMsgDictionaryID = msgDictionaryID;

    return OK;  

}   /* MsgHdlr_Init */



/*------------------------------------------------------------------------------*/

extern RLSTATUS
MsgHdlr_SystemInit (  void  )
{
    sbyte2 i;
    
    for ( i = 0; i < kNumMessageDictionaries; i++ )
    {
        mMsgDictionaries[i].pMsgTable = NULL;
        mMsgDictionaries[i].pDefaultMsg = NULL;
    }
    
    return OK;
    
}   /* MsgHdlr_SystemInit */




