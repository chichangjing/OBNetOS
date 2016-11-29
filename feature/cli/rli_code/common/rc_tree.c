/*  
 *  rc_tree.c
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

#ifdef  __DATABASE_USE_BTREE__
#include "rc_tree.h"

/*-----------------------------------------------------------------------*/

static void 
TREE_InOrderTraversal(node *pNode, void (*funcProcessObject)(void *))
{
    if (NULL != pNode->pLtNode)
        TREE_InOrderTraversal(pNode->pLtNode, funcProcessObject);

    funcProcessObject(pNode->pObject);

    if (NULL != pNode->pRtNode)
        TREE_InOrderTraversal(pNode->pRtNode, funcProcessObject);
}



/*-----------------------------------------------------------------------*/

static void 
TREE_PostOrderDestruct(node *pNode, void (*funcRemoveObject)(void *))
{
    if (NULL != pNode->pLtNode)
        TREE_PostOrderDestruct(pNode->pLtNode, funcRemoveObject);

    if (NULL != pNode->pRtNode)
        TREE_PostOrderDestruct(pNode->pRtNode, funcRemoveObject);

    funcRemoveObject(pNode->pObject);
    RC_FREE(pNode);
}



/*-----------------------------------------------------------------------*/

extern tree *TREE_Construct(void)
{
    tree *NewTree = RC_MALLOC(sizeof(tree));

    if (NULL != NewTree)
    {
        NewTree->RootOfTree = NULL;
    }

    return NewTree;
}



/*-----------------------------------------------------------------------*/

extern void 
TREE_Destruct(tree **ppTreeRoot, void (*funcDelete_pObject)(void *))
{
    if ((NULL != ppTreeRoot         ) && 
        (NULL != *(ppTreeRoot)      ) && 
        (NULL != funcDelete_pObject )   )
    {
        if (NULL != (*ppTreeRoot)->RootOfTree)
            TREE_PostOrderDestruct((*(ppTreeRoot))->RootOfTree, funcDelete_pObject);

        RC_FREE(*(ppTreeRoot));
        *(ppTreeRoot) = NULL;
    }
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
TREE_Traverse(tree *pTreeRoot, void (*funcProcessObject)(void *))
{
    if ((NULL == pTreeRoot)             ||
        (NULL == funcProcessObject)     ||
        (NULL == pTreeRoot->RootOfTree)     )
    {
        return ERROR_GENERAL_NOT_FOUND;
    }

    TREE_InOrderTraversal(pTreeRoot->RootOfTree, funcProcessObject);

    return OK;
}



/*-----------------------------------------------------------------------*/

extern RLSTATUS 
TREE_AddObject(tree *pTreeRoot, void *pObject, 
               TreeDir (*funcCompare)(void *, void *))
{
    node    *pNewNode;

    if ((NULL == pTreeRoot)   || 
        (NULL == funcCompare) ||
        (NULL == pObject)        )
    {
        return ERROR_GENERAL_NOT_FOUND;
    }

    pNewNode = RC_MALLOC(sizeof(node));

    if (NULL == pNewNode)
        return ERROR_MEMMGR_NO_MEMORY;

    pNewNode->pLtNode = NULL;
    pNewNode->pRtNode = NULL;
    pNewNode->pObject = pObject;

    if (NULL == pTreeRoot->RootOfTree)
    {
        pTreeRoot->RootOfTree = pNewNode;
    }
    else
    {
        node*   pTreeNode = pTreeRoot->RootOfTree;
        TreeDir Direction;

        while (1)
        {
            Direction = funcCompare(pTreeNode->pObject, pObject);

            if (GoToLeftNode == Direction)
            {
                if (NULL == pTreeNode->pLtNode)
                {
                    pTreeNode->pLtNode = pNewNode;
                    break;
                }
                pTreeNode = pTreeNode->pLtNode;
            }
            else if (GoToRightNode == Direction)
            {
                if (NULL == pTreeNode->pRtNode)
                {
                    pTreeNode->pRtNode = pNewNode;
                    break;
                }
                pTreeNode = pTreeNode->pRtNode;
            }
            else    /* duplicate node! */
            {
                RC_FREE(pNewNode);

                return ERROR_GENERAL_ACCESS_DENIED;
            }
        }

    }

    return OK;

} /* TREE_AddObject */



/*-----------------------------------------------------------------------*/

extern void *
TREE_FindObject(tree *pTreeRoot, void *pData, 
                TreeDir (*funcCompare)(void*,void*))
{
    node*   pTreeNode;
    TreeDir Direction;

    if ((NULL == pTreeRoot) || (NULL == pData) || (NULL == funcCompare))
        return NULL;

    pTreeNode = pTreeRoot->RootOfTree;

    if (NULL == pTreeNode)
        return NULL;

    while (1)
    {
        Direction = funcCompare(pTreeNode->pObject, pData);

        if (GoToLeftNode == Direction)
        {
            if (NULL == pTreeNode->pLtNode)
                return NULL;

            pTreeNode = pTreeNode->pLtNode;
        }
        else if (GoToRightNode == Direction)
        {
            if (NULL == pTreeNode->pRtNode)
                return NULL;

            pTreeNode = pTreeNode->pRtNode;
        }
        else
        {
            return pTreeNode->pObject;
        }

    }

    return NULL;

} /* TREE_FindObject */



/*-----------------------------------------------------------------------*/

static void TREE_CountNodes(node *pNode, Counter *pCount)
{
    *(pCount) = *(pCount) + 1;

    if (NULL != pNode->pLtNode)
        TREE_CountNodes(pNode->pLtNode, pCount);

    if (NULL != pNode->pRtNode)
        TREE_CountNodes(pNode->pRtNode, pCount);
}



/*-----------------------------------------------------------------------*/

extern Counter TREE_Size(tree *pTreeRoot)
{
    Counter Result = 0;

    if (NULL != pTreeRoot)
        if (NULL != pTreeRoot->RootOfTree)
            TREE_CountNodes(pTreeRoot->RootOfTree, &Result);

    return Result;
}



/*-----------------------------------------------------------------------*/

static node *RotateRightOneNode(node* pNode)
{
    node *pNewRoot, *pPrevLeftNode, *pParentOfNewRoot;
    
    if ((NULL == pNode) || (NULL == pNode->pLtNode))
        return pNode;

    pParentOfNewRoot = pPrevLeftNode = pNewRoot = pNode->pLtNode;

    /* break link */
    pNode->pLtNode   = NULL;

    /* locate new root */
    while (NULL != pNewRoot->pRtNode)
    {
        pParentOfNewRoot = pNewRoot;
        pNewRoot = pNewRoot->pRtNode;
    }

    /* rotate root right one position */
    pNewRoot->pRtNode = pNode;

    if (pNewRoot != pPrevLeftNode)
    {
        node *pNewLeftSegment = pNewRoot;

        /* cut off link to pNewNode from the old tree */
        pParentOfNewRoot->pRtNode = NULL;

        while (NULL != pNewLeftSegment->pLtNode)
            pNewLeftSegment = pNewLeftSegment->pLtNode;

        pNewLeftSegment->pLtNode = pPrevLeftNode;
    }

    return pNewRoot;

} /* RotateRightOneNode */



/*-----------------------------------------------------------------------*/

static node *RotateLeftOneNode(node* pNode)
{
    node *pNewRoot, *pPrevRightNode, *pParentOfNewRoot;
    
    if ((NULL == pNode) || (NULL == pNode->pRtNode))
        return pNode;

    pParentOfNewRoot = pPrevRightNode = pNewRoot = pNode->pRtNode;

    /* break link */
    pNode->pRtNode = NULL;

    /* locate new root */
    while (NULL != pNewRoot->pLtNode)
    {
        pParentOfNewRoot = pNewRoot;
        pNewRoot = pNewRoot->pLtNode;
    }

    /* rotate root left one position */
    pNewRoot->pLtNode = pNode;

    if (pNewRoot != pPrevRightNode)
    {
        node *pNewRightSegment = pNewRoot;

        /* cut off link to pNewNode from the old tree */
        pParentOfNewRoot->pLtNode = NULL;

        while (NULL != pNewRightSegment->pRtNode)
            pNewRightSegment = pNewRightSegment->pRtNode;

        pNewRightSegment->pRtNode = pPrevRightNode;
    }

    return pNewRoot;

} /* RotateLeftOneNode */



/*-----------------------------------------------------------------------*/

static node *TREE_PreOrderBalance(node *pNode, Counter TotalNodes)
{
    Counter     LeftNodes = 0;
    Counter     TruncTotalNodes = ((TotalNodes - 1) / 2) * 2; /* clear bit 0 */

    if (3 > TotalNodes)
        return pNode;

    if (NULL != pNode->pLtNode)
        TREE_CountNodes(pNode->pLtNode, &LeftNodes);

    if ((TruncTotalNodes - LeftNodes) > LeftNodes)
    {
        /* right side of tree is heavier */
        while ((int)(TruncTotalNodes - LeftNodes) > (int)LeftNodes)
        {
            pNode = RotateLeftOneNode(pNode);
            LeftNodes++;
        }
    }
    else
    {
        /* left side of tree is heavier */
        while ((int)(TruncTotalNodes - LeftNodes) < (int)LeftNodes)
        {
            pNode = RotateRightOneNode(pNode);
            LeftNodes--;
        }
    }

    if (NULL != pNode->pLtNode)
        pNode->pLtNode = TREE_PreOrderBalance(pNode->pLtNode, LeftNodes);

    if (NULL != pNode->pRtNode)
        pNode->pRtNode = TREE_PreOrderBalance(pNode->pRtNode, ((TotalNodes - 1) - LeftNodes));

    return pNode;

} /* TREE_PreOrderBalance */



/*-----------------------------------------------------------------------*/

extern RLSTATUS TREE_Balance(tree *pTreeRoot)
{
    Counter NumNodes = TREE_Size(pTreeRoot);

    if (NULL == pTreeRoot)
        return ERROR_GENERAL_NOT_FOUND;

    if (NULL == pTreeRoot->RootOfTree)
        return OK;

    pTreeRoot->RootOfTree = TREE_PreOrderBalance(pTreeRoot->RootOfTree, NumNodes);

    return OK;
}

#endif /*__DATABASE_USE_BTREE__*/
