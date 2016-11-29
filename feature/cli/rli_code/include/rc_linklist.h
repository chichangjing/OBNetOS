/*  
 *  rc_linklist.h
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

  $History: rc_linklist.h $
 * 
 * *****************  Version 7  *****************
 * User: Pstuart      Date: 12/11/00   Time: 11:30a
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * added c++ wrappers for prototypes
 * 
 * *****************  Version 6  *****************
 * User: Epeterson    Date: 4/25/00    Time: 2:13p
 * Updated in $/Rapid Logic/Code Line/rli_code/include
 * Include history and enable auto archiving feature from VSS

  */
#ifndef __LINKLIST_HEADER__
#define __LINKLIST_HEADER__

/* Structures & Datatypes */
typedef struct list
{
    void            *pObject;       /* pointer to data object  */
    struct  list    *pNext;         /* next object in the list */

} list;

/* Prototypes */

#ifdef __cplusplus
extern "C" {
#endif

extern list*    List_Construct      (void);
extern void     List_Destruct       (list **lstToDelete, 
                                     void (*funcDelete_pObject)(void *));
extern void     List_Destruct_Param (list **p_lstToDelete, 
                                     void (*funcObject_Destruct)(void *, void *), void *second_param );

extern RLSTATUS   List_AddObject      (list *plstHead, void *pData);
extern RLSTATUS   List_RemoveObject   (list *plstHead, 
                                     Boolean (*funcComparisonHandler)(void *));

extern RLSTATUS List_SelectiveRemoveObject(list *p_lstHead, void *pRemoveParam,
                                           Boolean (*funcComparisonHandler)(void *, void *));

extern RLSTATUS   List_Traverse       (list *plstHead, 
                                     void (*funcProcessObject)(void *));

extern void*    pList_FindObject    (list *Head, void *pVal, 
                                     Boolean (*Compare)(void*,void*));
extern Length   List_Size           (list *plstHead);

extern void *List_GetFirstObject( list *plstHead );
extern void *List_GetNextObject(list **p_lstSpot);

#ifdef __cplusplus
}
#endif

#endif
