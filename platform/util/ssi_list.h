
/*************************************************************
 * Filename     : ssi_list.h
 * Description  : SSI(System Service Interface) for list
 * Copyright    : OB Telecom Electronics Co.
 * Email        : hejianguo@obtelecom.com
 *************************************************************/
#ifndef _SSI_LIST_H_
#define _SSI_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL	((void *)0)
#endif

enum {
    SSI_LIST_SUCCESS = 0,
    SSI_LIST_ERROR = -1,
};

/* Node of LIST */
typedef struct list_node_tag { 
	struct list_node_tag *prev;	/* points at the previous node in the list */
	struct list_node_tag *next;	/* points at the next node in the list */
}SSI_LIST_NODE;


/* LIST header */
typedef struct { 
  SSI_LIST_NODE *head;	/* header of list */
  SSI_LIST_NODE *tail;	/* tail of list */
  unsigned int	count;	/* node count of list */
}SSI_LIST_HEAD;


/* Public API Function Prototypes */
int	ssi_list_init(SSI_LIST_HEAD * plist);
void ssi_list_insert(SSI_LIST_HEAD* plist, SSI_LIST_NODE* pprev, SSI_LIST_NODE* pnode);
void ssi_list_add(SSI_LIST_HEAD* plist, SSI_LIST_NODE* pnode);
void ssi_list_remove(SSI_LIST_HEAD* plist, SSI_LIST_NODE* pnode);

SSI_LIST_NODE* ssi_list_first(SSI_LIST_HEAD* plist);                                
SSI_LIST_NODE* ssi_list_last(SSI_LIST_HEAD* plist);
SSI_LIST_NODE* ssi_list_next(SSI_LIST_NODE* pnode);
SSI_LIST_NODE* ssi_list_prev(SSI_LIST_NODE* pnode);
//SSI_LIST_NODE* ssi_list_get(SSI_LIST_HEAD* plist);

unsigned int ssi_list_find(SSI_LIST_HEAD* plist, SSI_LIST_NODE* pnode);
unsigned int ssi_list_count(SSI_LIST_HEAD* plist);
void ssi_list_free(SSI_LIST_HEAD* plist);

#ifdef __cplusplus
}
#endif

#endif /* _SSI_LIST_H_ */

