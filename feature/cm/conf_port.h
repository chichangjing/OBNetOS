
#ifndef __CONF_PORT_H
#define __CONF_PORT_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "mconfig.h"
#include "stm32f2xx.h"
#include "hal_swif_port.h"

/*--------------------------------------------------------------------------------------------------------------
	Mask for port base config
  --------------------------------------------------------------------------------------------------------------*/
#define PC_MASK_ENABLE			0x00004000	/* port config mask for port enable */
#define PC_MASK_AUTO_NEG		0x00000040	/* port config mask for port auto-negotiation */
#define PC_MASK_SPEED			0x00000030	/* port config mask for port speed */
#define PC_MASK_DUPLEX			0x00000008	/* port config mask for port duplex */
#define PC_MASK_SPEED_DUPLEX	0x00000038	/* port config mask for port speed and duplex */
#define PC_MASK_MDIX			0x00000006	/* port config mask for port MDI-MDIX */
#define PC_MASK_FLOW_CTRL		0x00000001	/* port config mask for port flow control */

/* Port type */
typedef enum {
	CABLE_PORT = 0,
	OPTICAL_PORT,
	COMBO_PORT,
}mPortType;

/* Optical mode */
typedef enum {
	SINGLE_MODE = 0,
	MULTI_MODE,
}mOpticalMode;

/* Fiber type */
typedef enum {
	SINGLE_CORE = 0,
	DOUBLE_CORE,
}mFiberType; 

/* Distance */
typedef enum {
	KM20 = 1, 
	KM40,
	KM60,
	KM80,
	KM100,
	KM120,
}mDistance;

/* Optical type */
typedef enum {
	FC = 0,
	SC,
	ST,
	LC,
}mOpticalType; 

/* Electrical Port Type */
typedef enum {
	RJ45 = 0,
	M12,
}mElectricalPortType;


/* Communication mode */
typedef enum {
	COMMAUTO = 0,
	M1000F,
	M1000H,
	M100F,
	M100H,
	M10F,
	M10H
}mCommMode;


/* MDI mode */
typedef enum {
	MDIAUTO = 0,
	MDI,
	MDIX
}mMDIMode;


/*--------------------------------------------------------------------------------------------------------------
	Mask for port rate config
  --------------------------------------------------------------------------------------------------------------*/
#define PRC_MASK_INGRESS_MODE	0xC000		/* Port rate config mask for ingress limit mode */
#define PRC_MASK_PRI3_RATE		0x2000		/* Port rate config mask for priority 3 rate limit */
#define PRC_MASK_PRI2_RATE		0x1000		/* Port rate config mask for priority 2 rate limit */
#define PRC_MASK_PRI1_RATE		0x0800		/* Port rate config mask for priority 1 rate limit */
#define PRC_MASK_PRI0_RATE		0x0700		/* Port rate config mask for priority 0 rate limit */
#define PRC_MASK_EGRESS_RATE	0x0007		/* Port rate config mask for egress rate limit */

typedef enum {
	LIMIT_ALL_FRAMES = 0,
	LIMIT_B_M_U_CAST,
	LIMIT_B_M_CAST,
	LIMIT_B_CAST,
}mLimitMode;

/*--------------------------------------------------------------------------------------------------------------
	Mask for port mirror config
  --------------------------------------------------------------------------------------------------------------*/
#define PMC_MASK_MIRROR_MODE	0xC0000000		/* Bit[31:30]=0 : Egress monitor, Bit[31:30]=1 : Ingress monitor, Bit[31:30]=2/3 : Egress/Ingress monitor */
#define PMC_MASK_MIRROR_DEST	0x3FFFFFFF		/* Egress/Ingress monitor dest */


/*--------------------------------------------------------------------------------------------------------------*/
/* Mask for port qos config */  
#define QOS_SHEDULE_MODE        0x01         /* Set the Scheduling Mode */
#define QOS_STRICT_MODE         0x01
#define QOS_WRR_MODE            0x00

/* This macro calculates the mask for partial read /    */
/* write of register's data.                            */
#define QOS_MASK(fieldOffset,fieldLen,mask)        \
            if((fieldLen + fieldOffset) >= 8)      \
                mask = (GT_U8)(0 - (1 << fieldOffset));    \
            else                                    \
                mask = (GT_U8)(((1 << (fieldLen + fieldOffset))) - (1 << fieldOffset))

#define QOS_PRIO_MAP_RULE       0x40         /* IEEE Tag has higher priority than IP priority fields */
#define QOS_TOS_MAP_EN          0x20         /* Enables the IP priority mapping. */
#define QOS_COS_MAP_EN          0x10         /* Enables the user priority mapping */

/*--------------------------------------------------------------------------------------------------------------
	Mask for port isolation config
  --------------------------------------------------------------------------------------------------------------*/
#define PIC_MASK_LIMIT_SWITCH	0x8000
#define PIC_MASK_PORTS_MEMBER	0x7fff

/*--------------------------------------------------------------------------------------------------------------
	Mask for port vlan (pvid) config
  --------------------------------------------------------------------------------------------------------------*/
#define PVC_MASK_VID_CHECK_MODE	0x6000
#define PVC_MASK_VID_FORCE		0x1000
#define PVC_MASK_VID_NUMBER		0x0FFF

typedef enum {
	INGRESS_FILTER_RULE_NOT_CHECK = 0,
    INGRESS_FILTER_RULE_PRIO_CHECK,
	INGRESS_FILTER_RULE_CHECK,
	INGRESS_FILTER_RULE_SECURE
} INGRESS_FILTER_RULE;

/***********************************************************************/
typedef struct _port_config {
	u8	PortNum;
	u8	PortConfig[MAX_PORT_NUM * 4];
} port_conf_t;

typedef struct _port_rate_config {
	u8	PortNum;
	u8	PortConfig[MAX_PORT_NUM * 2];
} port_rate_conf_t;

typedef struct _port_isolation_config {
	u8	PortNum;
	u8	VlanMap[MAX_PORT_NUM * 2];
} port_isolation_conf_t;

typedef struct _port_vlan_config {
	u8	PortNum;
	u8	VlanConfig[MAX_PORT_NUM * 2];
} port_vlan_conf_t;

typedef struct _port_mirror_config {
	u8	PortNum;
	u8	SourcePort[MAX_PORT_NUM * 4];
} port_mirror_conf_t;

typedef struct _port_qos_config {
	u8	CosMapping[2];
	u8	TosMapping[16];
	u8	QosFlag[2];
	u8	PortNum;
	u8	PortQosConfig[MAX_PORT_NUM];
}port_qos_conf_t;

/* --------------- VLAN ------------------*/
#define MAX_VLAN_RECORD_COUNT 32
typedef struct _vlan_config {
	u8	PortNum;
	u8	TotalRecordCount;
} vlan_conf_t;

typedef struct _vlan_record_conf {
	u8	VLanID[2];
	u8	VLanName[8];
	u8	VLanSetting[3];
} vlan_record_conf_t;

/* --------------- Mcast ------------------*/
#define MAX_MCAST_RECORD_COUNT 32
typedef struct _mcast_config {
	u8	PortNum;
	u8	TotalRecordCount;
} mcast_conf_t;

typedef struct _mcast_record_conf {
	u8	Mac[6];
	u8	Member[2];
} mcast_record_conf_t;

/* --------------- Security ------------------*/
#define MAX_PORT_SECURITY_RECORD_COUNT 32
typedef struct _port_security_config {
	u8	PortNum;
	u8	TotalRecordCount;
	u8	SecuriyConfig[32 * 2];
} port_security_conf_t;

typedef struct _port_security_record_conf {
	u8	Index;
	u8	MacAddr[6];
	u8	PortVec[4];
	u8	Priority;
	u8	Res;
} port_security_record_conf_t;

/* Port security configuration */
/*
     ---------------------------------------------------
    | Bit    |  bit15    |  bit14~8  |     bit7~0       | 
     ---------------------------------------------------   
    | brief  | enable/   |  reserved | MacLimit_maximum |
    |        | disable   |           |     < 256        |
     ---------------------------------------------------
*/
#define MAX_MAC_LIST_COUNT					32
#define PSC_MASK_SECURITY_ENABLE			0x8000
#define PSC_MASK_SECURITY_MAC_LIMIT_MAXIMUM	0x00FF
#if 0
typedef struct _port_security_config {
	u8	PortNum;
	u8	MacListCount;
	u8	PortConfig[MAX_PORT_NUM * 2];
} port_security_conf_t;
#endif
typedef struct _mac_list_cfg {
	u8	MacAddr[6];
	u8	PortVec[4];
	u8	Prio;
	u8	Reserved[5];
} mac_list_conf_t;

/* --------------- Port trunk  ------------------*/
#define MAX_PORT_TRUNK_RECORD_COUNT 8
typedef struct _port_trunk_config {
	u8	PortNum;
	u8	TotalRecordCount;
} port_trunk_conf_t;

typedef struct _port_trunk_record_conf {
	u8	TrunkId;
	u8	MemberVec[4];
} port_trunk_record_conf_t;

#define MAX_TRUNK_ID		15
#define MAX_PORT_IN_TRUNK	8	// for M88E6095
typedef struct _trunk_member {
	u8	trunkId;
	u8	nTrunkPort;
	u8	port[MAX_PORT_IN_TRUNK];
} trunk_member_t;

int conf_port_base_init(void);
int conf_port_rate_init(void);
int conf_port_isolation_init(void);
int conf_port_vlan_init(void);
int conf_port_mirror_init(void);
int conf_port_qos_init(void);
int conf_port_security_init(void);
int conf_8021q_vlan_init(void);
int conf_static_multicast_init(void);

int conf_port_security_enable(unsigned int logic_port);
int conf_port_security_disable(unsigned int logic_port);

void conf_initialize(void);

#ifdef __cplusplus
}
#endif

#endif


