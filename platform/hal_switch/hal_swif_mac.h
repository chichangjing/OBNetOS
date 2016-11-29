
#ifndef _HAL_SWIF_MAC_H_
#define _HAL_SWIF_MAC_H_

#include "hal_swif_types.h"

#if SWITCH_CHIP_BCM53286
/* ARL Control Registers */
#define	BCM53286_PAGE_ARL_CTRL				0x04
#define BCM53286_FAST_AGING_CTRL			0x0B
#define BCM53286_AGE_OUT_CTRL				0x0C

#define MASK_FAST_AGE_STDN					0x80
#define MASK_EN_AGE_DYNAMIC_MC				0x40
#define MASK_EN_AGE_DYNAMIC_UC				0x20
#define MASK_EN_AGE_STATIC_MC				0x10
#define MASK_EN_AGE_STATIC_UC				0x08

/* Memory Search Registers */
#define	BCM53286_PAGE_MEM_SEARCH			0x07

#define	BCM53286_MEM_SEARCH_INDEX			0x00
#define	BCM53286_MEM_SEARCH_CTRL			0x08
#define	BCM53286_MEM_SEARCH_ADDR			0x10
#define	BCM53286_MEM_SEARCH_DATA0			0x20
#define	BCM53286_MEM_SEARCH_DATA1			0x28
#define	BCM53286_MEM_SEARCH_KEY2			0x90

/* Memory Access Registers */
#define	BCM53286_PAGE_MEM_ACCESS			0x08
#define	BCM53286_MEM_INDEX					0x00
#define	BCM53286_MEM_CTRL					0x08

#define	BCM53286_MEM_ADDR0					0x10
#define	BCM53286_MEM_ADDR1					0x12
#define	BCM53286_MEM_ADDR2					0x14
#define	BCM53286_MEM_ADDR3					0x16

#define	BCM53286_MEM_DATA0					0x20
#define	BCM53286_MEM_DATA1					0x28
#define	BCM53286_MEM_DATA2					0x30
#define	BCM53286_MEM_DATA3					0x38
#define	BCM53286_MEM_DATA4					0x40
#define	BCM53286_MEM_DATA5					0x48
#define	BCM53286_MEM_DATA6					0x50
#define	BCM53286_MEM_DATA7					0x58

#define	BCM53286_MEM_KEY0					0x80
#define	BCM53286_MEM_KEY1					0x88
#define	BCM53286_MEM_KEY2					0x90
#define	BCM53286_MEM_KEY3					0x98
#define	BCM53286_MEM_KEY4					0xA0
#define	BCM53286_MEM_KEY5					0xA8
#define	BCM53286_MEM_KEY6					0xB0
#define	BCM53286_MEM_KEY7					0xB8


typedef union {
    uint32  arl_data;   /* Raw 32-bit value */
    struct {
	    uint32	_arl_mac_47_44:4,
				_arl_pid:5,
				_arl_res1:1,
				_arl_vpid:4,
				_arl_res2:2,
				_arl_vid:12,
				_arl_static:1,
				_arl_age:3;
    } bcm5328x_arl_hi_unicast;

    struct {
	    uint32	_arl_mac_47_44:4,
				_arl_grp_id:12,
				_arl_vid:12,
				_arl_static:1,
				_arl_age:3;
    } bcm5328x_arl_hi_multicast;

    struct {
	    uint32	_arl_mac_43_12;
    } bcm5328x_arl_lo_mac;
	
} br_arl32_t; 

typedef struct _br_arl64
{
	br_arl32_t arl_lo32;
	br_arl32_t arl_hi32;
} br_arl64_t;


#elif SWITCH_CHIP_BCM53101
/* Switch Page define */
#define PAGE_CONTROL					0x00
#define PAGE_ARL_ACCESS					0x05

/* Switch Registers define */
#define REG_ARL_SERCH_CTRL				0x50
#define REG_ARL_SERCH_RESULT			0x24

#define REG_FAST_AGE_CONTROL			0x88
#define REG_FAST_AGE_PORT				0x89

/* Mask define */
#define MASK_ARL_SERCH_START			0x80
#define MASK_ARL_SR_VALID				0x01

#define MASK_EN_FAST_AGE_STATIC			0x01
#define MASK_EN_FAST_AGE_DYNAMIC		0x02
#define MASK_FAST_AGE_STR_DONE			0x80

#define PAGE_ARL_CONTROL				0x04
#define REG_BPDU_MC_ADDR				0x04

/* ARL/VLAN Table Access Registers */
#define	BCM53101M_PAGE_ARL_VLAN_TBL				0x05

#define	BCM53101M_ARL_SEARCH_CTRL				0x50
#define	BCM53101M_ARL_SEARCH_ADDR				0x51
#define	BCM53101M_ARL_SEARCH_MACVID_RESULT0		0x60
#define	BCM53101M_ARL_SEARCH_RESULT0			0x68
#define	BCM53101M_ARL_SEARCH_MACVID_RESULT1		0x70
#define	BCM53101M_ARL_SEARCH_RESULT1			0x78

typedef union {
    uint32  arl_data;   /* Raw 32-bit value */

    struct {
	    uint32	_arl_mac_47_32:16,
				_arl_vid:12,
				_arl_res:4;
    } bcm5396_arl_macvid_hi;
	
    struct {
	    uint32	_arl_mac_31_00;
    } bcm5396_arl_macvid_lo;

    struct {
	    uint32	_arl_portid_n:9,
				_arl_con_n:2,
				_arl_sr_priority_n:3,
				_arl_sr_age_n:1,
				_arl_sr_staric_n:1,
				_arl_sr_valid_n:1,
                _arl_res:15;
    } bcm5396_arl_unicast;
	
} br_arl32_t; 


typedef struct _br_arl64
{
	br_arl32_t arl_lo32;
	br_arl32_t arl_hi32;
} br_arl64_t;

#elif SWITCH_CHIP_BCM5396

/* Control Registers */
#define	BCM5396_PAGE_CTRL						0x00

#define BCM5396_FAST_AGING_CTRL					0x88
#define BCM5396_FAST_AGING_PORT					0x89
#define BCM5396_FAST_AGING_VID					0x8A

/* ARL/VLAN Table Access Registers */
#define	BCM5396_PAGE_ARL_VLAN_TBL				0x05

#define	BCM5396_ARL_SEARCH_CTRL					0x30
#define	BCM5396_ARL_SEARCH_ADDR					0x31
#define	BCM5396_ARL_SEARCH_MACVID_RESULT0		0x33	
#define	BCM5396_ARL_SEARCH_RESULT0				0x3B
#define	BCM5396_ARL_SEARCH_MACVID_RESULT1		0x40
#define	BCM5396_ARL_SEARCH_RESULT1				0x48

typedef union {
    uint32  arl_data;   /* Raw 32-bit value */

    struct {
	    uint32	_arl_mac_47_32:16,
				_arl_vid:12,
				_arl_res:4;
    } bcm5396_arl_macvid_hi;
	
    struct {
	    uint32	_arl_mac_31_00;
    } bcm5396_arl_macvid_lo;

    struct {
	    uint32	_arl_priority:3,
				_arl_valid:1,
				_arl_age:1,
				_arl_fwdportmap_0:1,
				_arl_fwdportmap_5_1:5,
				_arl_fwdportmap_16_6:11,
				_arl_res:10;
    } bcm5396_arl_multicast;

    struct {
	    uint32	_arl_priority:3,
				_arl_valid:1,
				_arl_age:1,
				_arl_static:1,
				_arl_portid:5,
				_arl_res:21;
    } bcm5396_arl_unicast;
	
} br_arl32_t; 

typedef struct _br_arl64
{
	br_arl32_t arl_lo32;
	br_arl32_t arl_hi32;
} br_arl64_t;

#endif

/*****************************************************************************************
 			Port Security Configuration
 			
 	SecurityConfig[]:
      ---------------------------------------------------
    | Bit    |  bit15    |  bit14~8  |     bit7~0       | 
     ---------------------------------------------------   
    | brief  | enable/   |  reserved | MacLimit_maximum |
    |        | disable   |           |     < 256        |
     ---------------------------------------------------    			
 ******************************************************************************************/
#define MAX_PORT_SECURITY_RECORD_COUNT 32
typedef struct {
	u8	PortNum;
	u8	TotalRecordCount;
	u8	SecurityConfig[32 * 2];
} port_security_conf_t;

typedef struct {
	u8	Index;
	u8	MacAddr[6];
	u8	PortVec[4];
	u8	Priority;
	u8	Res;
} port_security_record_conf_t;

typedef struct {
	u32	PortVecCfg;
	u8	MacAddr[6];
} port_security_info;

#define MAX_MAC_LIST_COUNT					32
#define PSC_MASK_SECURITY_ENABLE			0x8000
#define PSC_MASK_SECURITY_MAC_LIMIT_MAXIMUM	0x00FF

typedef struct {
	u8	MacAddr[6];
	u8	PortVec[4];
	u8	Prio;
	u8	Reserved[5];
} mac_list_conf_t;


/*****************************************************************************************
 			Struct for OBNet NMS message			
 ******************************************************************************************/
#if MODULE_OBNMS

typedef struct {
	uint8	GetCode;
	uint8	RetCode;
	uint8	PortNum;
	uint8	OpCode;
	uint8	RecordCount;
	uint8	SecurityConfig[32 * 2];
} obnet_set_port_security, obnet_rsp_get_port_security;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	OpCode;
} obnet_get_port_security;

typedef struct {
	u8	Index;
	u8	MacAddr[6];
	u8	PortVec[4];
	u8	Priority;
	u8	Res;
} obnet_port_security_rec;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	OpCode;
} obnet_get_mac_list;

typedef struct {
	u8	GetCode;
	u8	RetCode;
	u8	PortNum;
	u8	OpCode;
	u8	RecordCount;
} obnet_set_mac_list, obnet_rsp_get_mac_list;

typedef struct {
	u8	Index;
	u8	Priority;
	u8	MacAddr[6];
	u8	PortVec[4];
} obnet_mac_list_rec;

typedef struct {
	u8	PortNum;
	u8	LastMacRecFlag;
	u8	PacketIndex;
	u8	RecSendCount;
} obnet_maclist_stat_t;

#endif


/******************************************************************************************
	      Exported Functions                                                               
 ******************************************************************************************/
 
int hal_swif_mac_flush_all(void);
int hal_swif_mac_add(u8 *Mac, u8 Prio, u32 PortVec);
int hal_swif_mac_unicast_show(void *cliEnv);
int hal_swif_mac_security_conf_initialize(void);
#if ((BOARD_GE22103MA || BOARD_GE_EXT_22002EA || BOARD_GE220044MD) && (BOARD_FEATURE & L2_MAC_FILTER))
void hal_interrupt_proc_entry(void);
#endif
/* Functions for NMS */
#if MODULE_OBNMS
void nms_rsp_get_mac_list(u8 *DMA, u8 *RequestID, obnet_get_mac_list *pGetMacList);
void nms_rsp_set_port_security(u8 *DMA, u8 *RequestID, obnet_set_port_security *pPortSecurity);
void nms_rsp_get_port_security(u8 *DMA, u8 *RequestID, obnet_get_port_security *pGetPortSecurity);
#endif

#endif	/* _HAL_SWIF_MAC_H_ */

