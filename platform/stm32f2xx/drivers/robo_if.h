
#ifndef __ROBO_IF_H__
#define __ROBO_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f2xx.h"

#define MAX_RING_PORT	2

#define MAX_PORT_NUM	5

/****************************************************/
typedef enum {
    LINK_DOWN = 0,
    LINK_UP	= 1
} PORT_LINK_STATE;

typedef enum {
    HALF_DUPLEX = 0,
    FULL_DUPLEX = 1
} PORT_DUPLEX_STATE;

typedef enum {
    SPEED_10M	= 0,
    SPEED_100M	= 1,
    SPEED_1000M	= 2,
    SPEED_200M	= 3
} PORT_SPEED_STATE;

typedef enum {
    DISABLED	= 1,
    BLOCKING	= 2,
    LISTENING	= 3,
    LEARNING	= 4,
    FORWARDING	= 5
} PORT_STP_STATE;

typedef enum {
	S10M_HALF	= 0,
	S10M_FULL	= 1,
	S100M_HALF	= 2,
	S100M_FULL	= 3,
	S1000M_HALF	= 4,
	S1000M_FULL	= 5
} PORT_SPEED_DUPLEX;

/****************************************************/

typedef enum {
    STATE_00 = 0x00,		/* PORTA link down, PORTB link down */
    STATE_01 = 0x01,		/* PORTA link down, PORTB link up   */
    STATE_10 = 0x10,		/* PORTA link up,   PORTB link down */
    STATE_11 = 0x11,		/* PORTA link up,   PORTB link up   */
    STATE_POWER_ON = STATE_00
} NODE_LINK_STATE;

typedef enum {
    MODE_MDIX		= 0,
    MODE_MDI		= 1
} MDI_MDIX_STATE;

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

typedef struct _port_map 
{
	u8 lport;
	u8 port;
} lport_map_t;

u8 swif_Lport_2_Port(u8 lport);
u8 swif_Port_2_Lport(u8 port);
	
int swif_GetPortLinkState(u8 port, PORT_LINK_STATE *state);
int swif_SetPortStpState(u8 port, PORT_STP_STATE state);
int swif_GetPortStpState(u8 port, PORT_STP_STATE *state);
int swif_GetPortDuplex(u8 port, PORT_DUPLEX_STATE *state);
int swif_GetPortSpeed(u8 port, PORT_SPEED_STATE *state);

int swif_SetPortSpeedDuplex(u8 port, PORT_SPEED_DUPLEX state);
int swif_SetPortAutoMDIX(u8 port);
int swif_GetPortMDIMDIX(u8 port, MDI_MDIX_STATE *state);
int swif_ClearMacTbl_ByPort(u8 port);
int swif_ClearMacTbl(void);
	
int swif_ShowPort(void);

#ifdef __cplusplus
}
#endif

#endif

