
#include "ssi_list.h"

int ssi_list_init(SSI_LIST_HEAD* plist)
{
	/* initialize list header*/
	plist->head  = NULL;      
	plist->tail  = NULL;
	plist->count = 0;

	return (SSI_LIST_SUCCESS);
}


void ssi_list_insert(SSI_LIST_HEAD* plist, SSI_LIST_NODE* pprev, SSI_LIST_NODE* pnode)
{
	SSI_LIST_NODE* pnext;

	if (pprev == NULL) {	/* new node is to be first in list */ 
		pnext = plist->head;
		plist->head = pnode;
	} else {				/* make prev node point fwd to new */
		pnext = pprev->next;
		pprev->next = pnode;
	}

	if (pnext == NULL)		/* new node is to be last in list */
		plist->tail = pnode;    
	else					/* make next node point back to new */
		pnext->prev = pnode;  

	/* set pointers in new node */
	pnode->next   = pnext;
	pnode->prev   = pprev;

	plist->count++;
}               


void ssi_list_add(SSI_LIST_HEAD* plist, SSI_LIST_NODE* pnode)
{
	ssi_list_insert(plist, plist->tail, pnode);
}  

void ssi_list_remove(SSI_LIST_HEAD* plist, SSI_LIST_NODE* pnode)
{
	if (pnode->prev == NULL)
		plist->head = pnode->next;
	else
		pnode->prev->next = pnode->next;

	if (pnode->next == NULL)
		plist->tail = pnode->prev;
	else
		pnode->next->prev = pnode->prev;

	plist->count--;
}

SSI_LIST_NODE* ssi_list_first(SSI_LIST_HEAD* plist)
{
	return (plist->head);
}

SSI_LIST_NODE* ssi_list_last(SSI_LIST_HEAD* plist)
{
	return (plist->tail);
}


SSI_LIST_NODE* ssi_list_next(SSI_LIST_NODE* pnode)
{
	return (pnode->next);
}

SSI_LIST_NODE* ssi_list_prev(SSI_LIST_NODE* pnode)
{
	return (pnode->prev);
}

unsigned int ssi_list_count(SSI_LIST_HEAD* plist)
{
	return (plist->count);
}

void ssi_list_free(SSI_LIST_HEAD* plist)
{
	SSI_LIST_NODE *pnode1, *pnode2;

	if (plist->count > 0){
		pnode1 = plist->head;

		while (pnode1 != NULL){
			pnode2 = pnode1->next;
			//ssi_free ((char *)pnode1);
			pnode1 = pnode2;
		}

		plist->count = 0;
		plist->head = plist->tail = NULL;
	}

	return;
}



