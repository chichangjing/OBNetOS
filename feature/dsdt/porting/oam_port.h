
#ifndef __OAM_PORT_H__
#define __OAM_PORT_H__

#include <msApi.h>
#include "ssi_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _eMacPermissionMode {
	MAC_PERMISSION_STATIC,
	MAC_PERMISSION_FORBIDEN
} mac_permission_mode_e;

/* Mac list node */
typedef struct {
	SSI_LIST_NODE	Node;
	GT_U8			MacAddr[6];
	GT_U8			PortVec[4];
	GT_U8			Priority;
} oam_mac_list_t;

GT_STATUS MacListInit(void);
GT_STATUS MacListAdd(oam_mac_list_t *OamMacCfg);
GT_STATUS MacAdd2(GT_U8 mac0,GT_U8 mac1,GT_U8 mac2,GT_U8 mac3,GT_U8 mac4,GT_U8 mac5,GT_U32 portList,GT_U8 priority);

#ifdef __cplusplus
}
#endif
#endif

