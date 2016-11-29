/*
 *  rc_linklist.c
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
#include "rc_linklist.h"
#include "rc_os_spec.h"

/*-----------------------------------------------------------------------*/

static list *List_MoveToEnd(list *pFindEnd)
{
    if (NULL == pFindEnd)
        return NULL;

    while (NULL != pFindEnd->pNext)
        pFindEnd = pFindEnd->pNext;

    return pFindEnd;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS List_AddObject(list *p_lstHead, void *pData)
{
    list *p_lstNewElement, *pFindEnd = p_lstHead;

    if (NULL == p_lstHead)
        return ERROR_GENERAL_NOT_FOUND;

    if ((NULL == p_lstHead->pNext) && (NULL == p_lstHead->pObject))
    {
        p_lstHead->pObject = pData;

        return OK;
    }

    p_lstNewElement = (list *) RC_MALLOC (sizeof(list));

    if (NULL == p_lstNewElement)
        return SYS_ERROR_NO_MEMORY;

    p_lstNewElement->pObject    = pData;
    p_lstNewElement->pNext      = NULL;

    pFindEnd = List_MoveToEnd(pFindEnd);

    pFindEnd->pNext = p_lstNewElement;

    return OK;

} /* List_AddObject */



/*-----------------------------------------------------------------------*/

extern void *List_GetNextObject(list **p_lstSpot)
{
    if (NULL == p_lstSpot)
        return NULL;

    if (NULL != (*p_lstSpot))
    {
        void *pTemp = (*p_lstSpot)->pObject;

        *p_lstSpot  = (*p_lstSpot)->pNext;

        return pTemp;
    }

    return NULL;

}



/*-----------------------------------------------------------------------*/

extern void *List_GetFirstObject(list *plstHead)
{
    if (NULL == plstHead)
        return NULL;

    return plstHead->pObject;
}



/*-----------------------------------------------------------------------*/

extern void *pList_FindObject(list *p_lstHead, void *pValue,
                              Boolean (*fComparisonHandler)(void *, void *) )
{
    void *pObject;

    if ((NULL == p_lstHead) || (NULL == fComparisonHandler))
        return NULL;

    while (NULL != (pObject = List_GetNextObject(&p_lstHead)))
        if (TRUE == fComparisonHandler(pValue, pObject))
            return pObject;

    return NULL;
}



/*-----------------------------------------------------------------------*/

Length List_Size(list *p_lstHead)
{
    Length ListLen = 0;

    if (NULL == p_lstHead)
        return ListLen;

    if (NULL != p_lstHead->pObject)
        ListLen++;

    while (NULL != p_lstHead->pNext)
    {
        p_lstHead = p_lstHead->pNext;

        ListLen++;
    }

    return ListLen;

}



/*-----------------------------------------------------------------------*/

extern RLSTATUS
List_RemoveObject(list *p_lstHead, Boolean (*funcComparisonHandler)(void *))
{
    list *p_lstPrevious, *p_lstCurrent;

    if ((NULL == p_lstHead) || (NULL == p_lstHead->pObject) ||
        (NULL == funcComparisonHandler))
    {
        return ERROR_GENERAL_NOT_FOUND;
    }

    if (TRUE == (funcComparisonHandler(p_lstHead->pObject)))
    {
        list *p_lstTemp;

        RC_FREE(p_lstHead->pObject);

        if (NULL == p_lstHead->pNext)
        {
            p_lstHead->pObject = NULL;

            return OK;
        }

        p_lstHead->pObject  = p_lstHead->pNext->pObject;
        p_lstTemp           = p_lstHead->pNext;
        p_lstHead->pNext    = p_lstHead->pNext->pNext;

        RC_FREE(p_lstTemp);

        return OK;
    }

    p_lstPrevious   = p_lstHead;
    p_lstCurrent    = p_lstPrevious->pNext;

    while (NULL != p_lstCurrent)
    {
        if (TRUE == (funcComparisonHandler(p_lstCurrent->pObject)))
        {
            RC_FREE(p_lstCurrent->pObject);

            p_lstPrevious->pNext = p_lstCurrent->pNext;

            RC_FREE(p_lstCurrent);

            return OK;
        }

        p_lstPrevious   = p_lstCurrent;
        p_lstCurrent    = p_lstCurrent->pNext;
    }

    return ERROR_GENERAL_NOT_FOUND;

} /* List_RemoveObject */



/*-----------------------------------------------------------------------*/

extern RLSTATUS
List_SelectiveRemoveObject(list *p_lstHead, void *pRemoveParam,
                           Boolean (*funcComparisonHandler)(void *, void *))
{
    list *p_lstPrevious, *p_lstCurrent;

    if ((NULL == p_lstHead) || (NULL == p_lstHead->pObject) ||
        (NULL == funcComparisonHandler))
    {
        return ERROR_GENERAL_NOT_FOUND;
    }

    if (TRUE == (funcComparisonHandler(pRemoveParam, p_lstHead->pObject)))
    {
        list *p_lstTemp;

        RC_FREE(p_lstHead->pObject);

        if (NULL == p_lstHead->pNext)
        {
            p_lstHead->pObject = NULL;

            return OK;
        }

        p_lstHead->pObject  = p_lstHead->pNext->pObject;
        p_lstTemp           = p_lstHead->pNext;
        p_lstHead->pNext    = p_lstHead->pNext->pNext;

        RC_FREE(p_lstTemp);

        return OK;
    }

    p_lstPrevious   = p_lstHead;
    p_lstCurrent    = p_lstPrevious->pNext;

    while (NULL != p_lstCurrent)
    {
        if (TRUE == (funcComparisonHandler(pRemoveParam, p_lstCurrent->pObject)))
        {
            RC_FREE(p_lstCurrent->pObject);

            p_lstPrevious->pNext = p_lstCurrent->pNext;

            RC_FREE(p_lstCurrent);

            return OK;
        }

        p_lstPrevious   = p_lstCurrent;
        p_lstCurrent    = p_lstCurrent->pNext;
    }

    return ERROR_GENERAL_NOT_FOUND;

} /* List_SelectiveRemoveObject */



/*-----------------------------------------------------------------------*/

extern RLSTATUS
List_Traverse(list *p_lstHead, void (*funcProcessObject)(void *))
{
    list *p_lstSpot = p_lstHead;
    void *pObject;

    if ((NULL == p_lstHead) || (NULL == funcProcessObject))
        return ERROR_GENERAL_NOT_FOUND;

    while (NULL != (pObject = List_GetNextObject(&p_lstSpot)))
        funcProcessObject(pObject);

    return OK;
}



/*-----------------------------------------------------------------------*/

extern list *List_Construct(void)
{
    list *pTemp = (list *) RC_MALLOC (sizeof(list));

    if (NULL == pTemp)
        return NULL;

    pTemp->pObject  = NULL;
    pTemp->pNext    = NULL;

    return pTemp;
}



/*-----------------------------------------------------------------------*/

extern void
List_Destruct_Param(list **p_lstToDelete,
                    void (*funcObject_Destruct)(void *, void *),
                    void *second_param )
{
    list *p_lstCurrent, *p_lstNext;

    if ((NULL == p_lstToDelete) || (NULL == funcObject_Destruct))
        return;

    p_lstCurrent = *(p_lstToDelete);

    while (NULL != p_lstCurrent)
    {
        if (NULL != p_lstCurrent->pObject)
            funcObject_Destruct(p_lstCurrent->pObject, second_param);

        p_lstNext = p_lstCurrent->pNext;

        RC_FREE(p_lstCurrent);

        p_lstCurrent = p_lstNext;
    }

    *p_lstToDelete = NULL;

}



/*-----------------------------------------------------------------------*/

extern void List_Destruct(list **p_lstToDelete, void (*funcObject_Destruct)(void *))
{
    list *p_lstCurrent, *p_lstNext;

    if ((NULL == p_lstToDelete) || (NULL == *p_lstToDelete) || (NULL == funcObject_Destruct))
        return;

    p_lstCurrent = *(p_lstToDelete);

    while (NULL != p_lstCurrent)
    {
        if (NULL != p_lstCurrent->pObject)
            funcObject_Destruct(p_lstCurrent->pObject);

        p_lstNext = p_lstCurrent->pNext;

        RC_FREE(p_lstCurrent);

        p_lstCurrent = p_lstNext;
    }

    *p_lstToDelete = NULL;

}

