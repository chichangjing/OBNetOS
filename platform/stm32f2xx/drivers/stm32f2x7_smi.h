

#ifndef __STM32F2X7_SMI_DRV_H__
#define __STM32F2X7_SMI_DRV_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f2xx.h"

/* Return */
#define SMI_DRV_FAIL					(-1)
#define SMI_DRV_SUCCESS					(0)

#if SWITCH_CHIP_88E6095
/* Address macro */
#define PHYADDR_PHY(p)					(0x00 +(p))
#define PHYADDR_PORT(p)					(0x10 +(p))
#define PHYADDR_GLOBAL					0x1B
#define PHYADDR_GLOBAL2					0x1C


/* QuarterDeck Per Port Registers */
#define SW_REG_PORT_STATUS				0x0
#define SW_REG_PCS_CONTROL				0x1		/* for Sapphire family */
#define SW_REG_SWITCH_ID				0x3
#define SW_REG_PORT_CONTROL				0x4
#define SW_REG_PORT_CONTROL1			0x5
#define SW_REG_PORT_VLAN_MAP			0x6
#define SW_REG_PVID						0x7
#define SW_REG_PORT_CONTROL2			0x8		/* for Sapphire family */
#define SW_REG_INGRESS_RATE_CTRL		0x9		/* for Sapphire family */
#define SW_REG_EGRESS_RATE_CTRL			0xA		/* for Sapphire family */
#define SW_REG_RATE_CTRL				0xA
#define SW_REG_PAV						0xB
#define SW_REG_RX_COUNTER				0x10
#define SW_REG_TX_COUNTER				0x11
#define SW_REG_DROPPED_COUNTER			0x12

#define SW_REG_INDISCARD_LO_COUNTER		0x10
#define SW_REG_INDISCARD_HI_COUNTER		0x11
#define SW_REG_INFILTERED_COUNTER		0x12
#define SW_REG_OUTFILTERED_COUNTER		0x13

#define SW_REG_Q_COUNTER				0x1B
#define SW_REG_RATE_CONTROL				0x0A
#define SW_REG_PORT_ASSOCIATION			0x0B
#define SW_REG_IEEE_PRI_REMAP_3_0		0x18	/* for Sapphire family */
#define SW_REG_IEEE_PRI_REMAP_7_4		0x19	/* for Sapphire family */

#define SW_REG_PROVIDER_TAG				0x1A	/* for Schooner family */

/* QuarterDeck Global Registers */
#define SW_REG_GLOBAL_STATUS			0x0
#define SW_REG_MACADDR_01				0x1
#define SW_REG_MACADDR_23				0x2
#define SW_REG_MACADDR_45				0x3
#define SW_REG_GLOBAL_CONTROL			0x4
#define SW_REG_GLOBAL_CONTROL2			0x1C	/* for Sapphire, Schooner family */
#define SW_REG_CORETAG_TYPE				0x19	/* for Ruby family */
#define SW_REG_MONITOR_CONTROL			0x1A	/* for Ruby family */
#define SW_REG_MANGEMENT_CONTROL		0x1A	/* for Schooner family */
#define SW_REG_TOTAL_FREE_COUNTER		0x1B	/* for Schooner family */

/* QuarterDeck Global 2 Registers */
#define SW_REG_PHYINT_SOURCE			0x0
#define SW_REG_MGMT_ENABLE				0x3		/* page 225 */
#define SW_REG_FLOWCTRL_DELAY			0x4		
#define SW_REG_MANAGEMENT				0x5		/* page 226 */
#define SW_REG_ROUTING_TBL				0x6
#define SW_REG_TRUNK_MASK_TBL			0x7
#define SW_REG_TRUNK_ROUTING			0x8
#define SW_REG_PRIORITY_OVERRIDE		0xF
#define SW_REG_INGRESS_RATE_COMMAND		0x9
#define SW_REG_INGRESS_RATE_DATA		0xA

/* the following VTU entries are added for Fullsail and Clippership */
#define SW_REG_VTU_OPERATION			0x5
#define SW_REG_VTU_VID_REG				0x6
#define SW_REG_VTU_DATA1_REG			0x7
#define SW_REG_VTU_DATA2_REG			0x8
#define SW_REG_VTU_DATA3_REG			0x9
#define SW_REG_STATS_OPERATION			0x1D
#define SW_REG_STATS_COUNTER3_2			0x1E
#define SW_REG_STATS_COUNTER1_0			0x1F
 
#define SW_REG_ATU_CONTROL				0xA
#define SW_REG_ATU_OPERATION			0xB
#define SW_REG_ATU_DATA_REG				0xC
#define SW_REG_ATU_MAC_BASE				0xD
#define SW_REG_IP_PRI_BASE				0x10
#define SW_REG_IEEE_PRI					0x18

/* Definitions for MIB Counter */
#define SW_STATS_NO_OP					0x0
#define SW_STATS_FLUSH_ALL				0x1
#define SW_STATS_FLUSH_PORT				0x2
#define SW_STATS_READ_COUNTER			0x4
#define SW_STATS_CAPTURE_PORT			0x5

#define SW_PHY_CONTROL_REG				0
#define SW_PHY_AUTONEGO_AD_REG			4
#define SW_PHY_NEXTPAGE_TX_REG			7
#define SW_PHY_AUTONEGO_1000AD_REG		9
#define SW_PHY_SPEC_CONTROL_REG			16
#define SW_PHY_SPEC_STATUS_REG			17
#define SW_PHY_INT_ENABLE_REG			18
#define SW_PHY_INT_STATUS_REG			19
#define SW_PHY_INT_PORT_SUMMARY_REG		20

/* Bit Definition for QD_PHY_CONTROL_REG */
#define SW_PHY_RESET					0x8000
#define SW_PHY_LOOPBACK					0x4000
#define SW_PHY_SPEED					0x2000
#define SW_PHY_AUTONEGO					0x1000
#define SW_PHY_POWER					0x800
#define SW_PHY_ISOLATE					0x400
#define SW_PHY_RESTART_AUTONEGO			0x200
#define SW_PHY_DUPLEX					0x100
#define SW_PHY_SPEED_MSB				0x40
#define SW_PHY_POWER_BIT				11
#define SW_PHY_RESTART_AUTONEGO_BIT		9

#endif

/* Exported functions ------------------------------------------------------- */
int smi_getreg(u16 phyAddr, u16 regAddr, u16 *data);
int smi_setreg(u16 phyAddr, u16 regAddr, u16 data);
int smi_getregfield(u16 phyAddr, u16 regAddr, u16 fieldOffset, u16 fieldLength, u16 *data);
int smi_setregfield(u16 phyAddr, u16 regAddr, u16 fieldOffset, u16 fieldLength, u16 data);
int smi_probe(void);

#ifdef __cplusplus
}
#endif

#endif


