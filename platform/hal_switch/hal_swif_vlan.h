
#ifndef _HAL_SWIF_VLAN_H_
#define _HAL_SWIF_VLAN_H_

#include "mconfig.h"

/* Port Link Type */
typedef enum {
	TYPE_ACCESS		= 0,
	TYPE_TRUNK 		= 1,
	TYPE_HYBRID		= 2
} HAL_PORT_LINK_TYPE;

/* Port Link Type */
typedef enum {
	VLAN_MODE_PORT_BASED	= 0,
	VLAN_MODE_8021Q 		= 1
} HAL_VLAN_MODE;

/******************************************************************************************
	      Port-Based VLAN Configuration for NMS
 ******************************************************************************************/
/* Mask for port isolation config */
#define PORT_ISOLATION_MASK_LIMIT_VID_SWITCH	0x8000
#define PORT_ISOLATION_MASK_PORTS_MEMBER		0x7fff

#define MAX_PORT_VLAN_RECORD_COUNT	64
typedef struct {
	u8	PortNum;
	u8	VlanMap[MAX_PORT_NUM * 2];
} hal_port_isolation_conf_t;

/******************************************************************************************
	      802.1Q VLAN Configuration for NMS
 ******************************************************************************************/
/* Mask for pvid config */
#define PVID_MASK_INGRESS_FILTER_RULE			0x6000
#define PVID_MASK_VID_FORCE						0x1000
#define PVID_MASK_VID_NUMBER					0x0FFF

typedef enum {
	VLAN_INGRESS_RULE_NOT_CHECK = 0,
    VLAN_INGRESS_RULE_PRIO_CHECK,
	VLAN_INGRESS_RULE_CHECK,
	VLAN_INGRESS_RULE_SECURE
} INGRESS_FILTER_RULE;

#define MAX_8021Q_VLAN_RECORD_COUNT	32

typedef struct {
	u8	PortNum;
	u8	TotalRecordCount;
} hal_8021q_vlan_conf_t;

/*  
	Analysis for VLanSetting[3]: 
     --------------------------------------------------------------------------------------
    |               Byte2               |     ---      |               Byte0               |
     --------------------------------------------------------------------------------------
    | Port12 | Port11 | Port10 | Port09 |     ---      | Port04 | Port03 | Port02 | Port01 |
    --------------------------------------------------------------------------------------
    | Bit7:6 | Bit5:4 | Bit3:2 | Bit1:0 |     ---      | Bit7:6 | Bit5:4 | Bit3:2 | Bit1:0 |
     --------------------------------------------------------------------------------------
    | 00: UM | 00: UM | 00: UM | 00: UM |              | 00: UM | 00: UM | 00: UM | 00: UM |
    | 01: NA | 01: NA | 01: NA | 01: NA |     ---      | 01: NA | 01: NA | 01: NA | 01: NA |
    | 10: UT | 10: UT | 10: UT | 10: UT |              | 10: UT | 10: UT | 10: UT | 10: UT |
    | 11: TG | 11: TG | 11: TG | 11: TG |              | 11: TG | 11: TG | 11: TG | 11: TG |     
     --------------------------------------------------------------------------------------	
     
	Analysis for VLanSetting[8]: 
     --------------------------------------------------------------------------------------
    |               Byte7               |     ---      |               Byte0               |
     --------------------------------------------------------------------------------------
    | Port32 | Port31 | Port30 | Port29 |     ---      | Port04 | Port03 | Port02 | Port01 |
    --------------------------------------------------------------------------------------
    | Bit7:6 | Bit5:4 | Bit3:2 | Bit1:0 |     ---      | Bit7:6 | Bit5:4 | Bit3:2 | Bit1:0 |
     --------------------------------------------------------------------------------------
    | 00: UM | 00: UM | 00: UM | 00: UM |              | 00: UM | 00: UM | 00: UM | 00: UM |
    | 01: NA | 01: NA | 01: NA | 01: NA |     ---      | 01: NA | 01: NA | 01: NA | 01: NA |
    | 10: UT | 10: UT | 10: UT | 10: UT |              | 10: UT | 10: UT | 10: UT | 10: UT |
    | 11: TG | 11: TG | 11: TG | 11: TG |              | 11: TG | 11: TG | 11: TG | 11: TG |     
     --------------------------------------------------------------------------------------	
*/

typedef struct {
	u8	VLanID[2];
	u8	VLanName[8];
	u8	VLanSetting[8];
} hal_8021q_vlan_record_t;

/*****************************************************************************************
 			Struct for OBNet NMS message			
 ******************************************************************************************/
#if MODULE_OBNMS

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	OutPortSel[2 * MAX_PORT_NUM];
} obnet_set_port_isolation, obnet_rsp_get_port_isolation;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	VLanSetting[2 * MAX_PORT_NUM];
} obnet_set_port_vlan, obnet_rsp_get_port_vlan;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	VLanID[2];
} obnet_set_adm_vlan, obnet_rsp_get_adm_vlan;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	OpCode;
	u8	RecordCount;
} obnet_set_vlan, obnet_rsp_get_vlan;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	OpCode;
} obnet_get_vlan;

typedef struct {
	u8	VLanID[2];
	u8	VLanName[8];
	u8	VLanSetting[8];
} obnet_vlan_rec;

#endif	/* MODULE_OBNMS */

/******************************************************************************************
	      Exported Functions
 ******************************************************************************************/
int hal_swif_port_isolation_conf_initialize(void);
int hal_swif_pvid_conf_initialize(void);
int hal_swif_8021q_vlan_conf_initialize(void);

 /* Functions for NMS */
#if MODULE_OBNMS
void nms_rsp_set_port_isolation(u8 *DMA, u8 *RequestID, obnet_set_port_isolation *pPortIsolation);
void nms_rsp_get_port_isolation(u8 *DMA, u8 *RequestID);
void nms_rsp_set_port_vlan(u8 *DMA, u8 *RequestID, obnet_set_port_vlan *pSetPortVlan);
void nms_rsp_get_port_vlan(u8 *DMA, u8 *RequestID);
void nms_rsp_set_adm_vlan(u8 *DMA, u8 *RequestID, obnet_set_adm_vlan *pSetAdmVlan);
void nms_rsp_get_adm_vlan(u8 *DMA, u8 *RequestID);
void nms_rsp_set_vlan(u8 *DMA, u8 *RequestID, obnet_set_vlan *pSetVlan);
void nms_rsp_get_vlan(u8 *DMA, u8 *RequestID, obnet_get_vlan *pGetVlan);
#endif


/******************************************************************************************/
/******************************************************************************************/

#endif	/* _HAL_SWIF_VLAN_H_ */



