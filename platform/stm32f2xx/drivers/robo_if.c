
/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "semphr.h"

/* BSP include */
#include "stm32f2xx.h"
#include "robo_drv.h"
#include "robo_if.h"

/************************************************************************************************************

	Front panel interface of the product GE-20023 : (IMP port: port8)
    ------------------------------------------------------------------------------------------------------
    | physical port:    |  Port0    |  port1     |  port2      |  port3      | port4      |  RJ45       | 
    | panel interface:  |  100M SFP |  100M SFP  |  100M RJ45  |  100M RJ45  | 100M RJ45  |  RS232/485  |
    ------------------------------------------------------------------------------------------------------
	
*************************************************************************************************************/

static PORT_STP_STATE ring_port_stp_state[MAX_RING_PORT];

static lport_map_t lport_map[MAX_PORT_NUM] = {{1,0}, {2,1}, {3,2}, {4,3}, {5,4}};


/**************************************************************************
  * @brief  convert between logic port and hardware port
  * @param  port or lport
  * @retval port or lport
  *************************************************************************/
u8 swif_Lport_2_Port(u8 lport)
{
	int i;
	
	for(i=0; i<MAX_PORT_NUM; i++) {
		if(lport_map[i].lport == lport) {
			break;
		}
	}

	return lport_map[i].port;
}

u8 swif_Port_2_Lport(u8 port)
{
	int i;
	
	for(i=0; i<MAX_PORT_NUM; i++) {
		if(lport_map[i].port == port) {
			break;
		}
	}

	return lport_map[i].lport;
}

/**************************************************************************
  * @brief  retrives the state
  * @param  port, state
  * @retval 
  *		0: success
  *	   -1: failed
  *************************************************************************/
int swif_GetPortLinkState(u8 port, PORT_LINK_STATE *state)
{
	int ret;
	u16 data;
	
	ret = robo_read(0x01, 0x00, (u8 *)&data, 2);
	*state = (PORT_LINK_STATE)((data & (1 << port)) >> port);
	
	return ret;
}

/**************************************************************************
  * @brief  set the port state
  * @param  port, state
  * @retval 
  *		0: success
  *	   -1: failed
  *************************************************************************/
int swif_SetPortStpState(u8 port, PORT_STP_STATE state)
{
	int ret;
	u8 value, temp;
	
	ret = robo_read(0x00, port, &value, 1);
	value = ((u8)state << 5) | (value & 0x1F);
	ret = robo_write(0x00, port, &value, 1);

	if(port < MAX_RING_PORT)
		ring_port_stp_state[port] = state;
	
	return ret;
}

/**************************************************************************
  * @brief  get the port state
  * @param  port, state
  * @retval 
  *		0: success
  *	   -1: failed
  *************************************************************************/
int swif_GetPortStpState(u8 port, PORT_STP_STATE *state)
{
	int ret;
	u8 value;

	ret = robo_read(0x00, port, &value, 1);
	*state = (PORT_STP_STATE)(value >> 5);
	
	return ret;
}

/**************************************************************************
  * @brief  get the port duplex mode
  * @param  port, state
  * @retval 
  *		0: success
  *	   -1: failed
  *************************************************************************/
int swif_GetPortDuplex(u8 port, PORT_DUPLEX_STATE *state)
{
	int ret;
	u16	duplexState;
	
	ret = robo_read(0x01, 0x08, (u8 *)&duplexState, 2);
	*state = (PORT_DUPLEX_STATE)((duplexState & (1<<port)) >> port);
    
	return ret;
}

/**************************************************************************
  * @brief  get the port speed mode
  * @param  port, state
  * @retval 
  *		0: success
  *	   -1: failed
  *************************************************************************/
int swif_GetPortSpeed(u8 port, PORT_SPEED_STATE *state)
{
	int ret;
	u32 portSpeed;

	ret = robo_read(0x01, 0x04, (u8 *)&portSpeed, 4);
	*state = (PORT_STP_STATE)((portSpeed & (0x3<<port*2)) >> port*2);
	
	return ret;
}

/**************************************************************************
  * @brief  set the port speed and duplex
  * @param  port, state
  * @retval 
  *		0: success
  *	   -1: failed
  *************************************************************************/
int swif_SetPortSpeedDuplex(u8 port, PORT_SPEED_DUPLEX state)
{
	int ret=0;
	u32 portSpeed;
	u8 page_val;
	u16 reg_val;

	page_val = 0x10 + port;
	switch(state) {
		case S10M_HALF:
		reg_val = 0x0000;
		ret = robo_write(page_val, 0x00, (u8 *)&reg_val, 2);
		break;

		case S10M_FULL:
		reg_val = 0x0100;
		ret = robo_write(page_val, 0x00, (u8 *)&reg_val, 2);			
		break;

		case S100M_HALF:
		reg_val = 0x2000;
		ret = robo_write(page_val, 0x00, (u8 *)&reg_val, 2);			
		break;

		case S100M_FULL:
		reg_val = 0x2100;
		ret = robo_write(page_val, 0x00, (u8 *)&reg_val, 2);			
		break;

		default:
		break;
	}
	
	return ret;
}


/**************************************************************************
  * @brief  set the port to auto-mdix
  * @param  port
  * @retval 
  *		0: success
  *	   -1: failed
  *************************************************************************/
int swif_SetPortAutoMDIX(u8 port)
{
	int ret=0;
	u8 page_val;
	u16 reg_val;

	page_val = 0x10 + port;
	reg_val = 0x4000;
	ret = robo_write(page_val, 0x20, (u8 *)&reg_val, 2);
	
	return ret;
}

/**************************************************************************
  * @brief  get the port to mdi-mdix
  * @param  port
  * @retval 
  *		0: success
  *	   -1: failed
  *************************************************************************/
int swif_GetPortMDIMDIX(u8 port, MDI_MDIX_STATE *state)
{
	int ret=0;

	*state = MODE_MDIX;
	
	return ret;
}

/**************************************************************************
  * @brief  show port stats
  * @param  none
  * @retval none
  *************************************************************************/
int swif_ShowPort(void)
{
	u8	port;
	u16	linkState;
	u32	portSpeed;
	u16	duplexState;
	u8	stpState;
	int i;

	robo_read(0x01, 0x04, (u8 *)&portSpeed, 4);
	robo_read(0x01, 0x00, (u8 *)&linkState, 2);
	robo_read(0x01, 0x08, (u8 *)&duplexState, 2);
	
	printf("Port  Link   Duplex   Speed       STP   \r\n");
	printf("-------------------------------------------\r\n");
	for(port=0; port<=8; port++) {
		if((port == 6) || (port == 7))
			continue;

		stpState = 0;
		if(port <= 5){
			robo_read(0x00, port, &stpState, 1);
		}
		printf(" %02d   %4s    %4s    %5s   %10s\r\n",
			port, 
			(((linkState & (1<<port)) >> port) == LINK_UP)? "Up" : "Down",
			(((duplexState & (1<<port)) >> port) == FULL_DUPLEX)? "Full" : "Half",
			(((portSpeed & (0x3<<port*2)) >> port*2) == SPEED_10M)? "10 M" : \
			(((portSpeed & (0x3<<port*2)) >> port*2) == SPEED_100M)? "100 M" : \
			(((portSpeed & (0x3<<port*2)) >> port*2) == SPEED_1000M)? "1000 M" : "200 M",
			(((stpState & 0xe0) >> 5) == DISABLED)? "Disabled" : \
			(((stpState & 0xe0) >> 5) == BLOCKING)? "Blocking" : \
			(((stpState & 0xe0) >> 5) == LISTENING)? "Listening" : \
			(((stpState & 0xe0) >> 5) == LEARNING)? "Learning" : \
			(((stpState & 0xe0) >> 5) == FORWARDING)? "Forwarding" : "Unkown");	
	}

  return 0;
}



int swif_ClearMacTbl_ByPort(u8 port)
{
	u8 reg_val;
	int i;
	
	/* Set fast-aging port control register */
	reg_val = port;
	robo_write(PAGE_CONTROL, REG_FAST_AGE_PORT, &reg_val, 1);	

	/* Start fast aging process */
	robo_read(PAGE_CONTROL, REG_FAST_AGE_CONTROL, &reg_val, 1);	

	reg_val |= (MASK_EN_FAST_AGE_STATIC | MASK_EN_FAST_AGE_DYNAMIC | MASK_FAST_AGE_STR_DONE); 
	robo_write(PAGE_CONTROL, REG_FAST_AGE_CONTROL, &reg_val, 1);

	/* Wait for complete */
	for(i=0; i<100; i++) {
		robo_read(PAGE_CONTROL, REG_FAST_AGE_CONTROL, &reg_val, 1);	
		if((reg_val & 0x80) == 0)
			break;
	}

	/* Restore register value to 0x2, otherwise aging will fail	*/
	reg_val = MASK_EN_FAST_AGE_DYNAMIC;
	robo_write(PAGE_CONTROL, REG_FAST_AGE_CONTROL, &reg_val, 1);
	
	//printf("Fast aging done\r\n");

	return 0;
}


int swif_ClearMacTbl(void)
{
	u8 i;

	for(i=0; i<5; i++) {
		swif_ClearMacTbl_ByPort(i);
	}

	return 0;
}


void swif_showMacTable(void)
{
	u8 macData[8];
	u8 ctrlVal = MASK_ARL_SERCH_START;

	printf("PORT	MAC					STATIC	MODE	AGE	PRIORITY\r\n");
	printf("--------------------------------------------------------\r\n");
	robo_write(PAGE_ARL_ACCESS, REG_ARL_SERCH_CTRL, &ctrlVal, 1);
	do
	{
		robo_read(PAGE_ARL_ACCESS, REG_ARL_SERCH_CTRL, &ctrlVal, 1);
		printf("ctrlVal = 0x%02x\r\n", ctrlVal);
	}while((ctrlVal & MASK_ARL_SERCH_START));
}


#if 0
void swif_showMacTable(void)
{
	u8 macData[8];
	u8 ctrlVal = MASK_ARL_SERCH_START;

	printf("PORT	MAC					STATIC	MODE	AGE	PRIORITY\r\n");
	printf("--------------------------------------------------------\r\n");
	robo_write(PAGE_ARL_ACCESS, REG_ARL_SERCH_CTRL, &ctrlVal, 1);
	do
	{
		robo_read(PAGE_ARL_ACCESS, REG_ARL_SERCH_CTRL, &ctrlVal, 1);
		if(ctrlVal & MASK_ARL_SR_VALID)
		{
			Read(PAGE_ARL_ACCESS, REG_ARL_SERCH_RESULT, 8, macData);
			if (macData[7] & MASK_SR_RESULT_VALID)
			{
				printf("%d	%02x:%02x:%02x:%02x:%02x:%02x			%d	%d	%d	%2x\n", 
					(UShort)(macData[6] & MASK_SR_RESULT_PORTID), 
					macData[5], 
					macData[4], 
					macData[3], 
					macData[2], 
					macData[1], 
					macData[0], 
					(Octet)((macData[7] & MASK_SR_RESULT_STATIC) >> 7), 
					(Octet)((macData[7] & MASK_SR_RESULT_MODE) >> 1), 
					(Octet)((macData[6] & MASK_SR_RESULT_AGE) >> 7), 
					(Octet)((macData[6] & MASK_SR_RESULT_PRI) >> 5));
			}
		}
	}while((ctrlVal & MASK_ARL_SERCH_START));
}

#endif

